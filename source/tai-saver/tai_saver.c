#define _POSIX_C_SOURCE 200809L
#include "tai_saver.h"
#include "sha256.h"
#include "rt_fs.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#if !defined(RTSE_PLATFORM_VITA)
#include <sys/statvfs.h>
#endif
#include <sys/types.h>
#if defined(RTSE_PLATFORM_VITA)
#include <psp2/io/devctl.h>
#include <psp2/io/fcntl.h>
#endif
#include <time.h>
#include <unistd.h>

#define TS_SCHEMA_VERSION 1U
#define TS_MAX_FILES 4096U
#define TS_MAX_MANIFEST_BYTES (8U * 1024U * 1024U)
#define TS_JSON_TOKEN_LIMIT (TS_MAX_FILES * 10U + 512U)
#define TS_COPY_BUFFER 32768U

typedef struct {
    char path[TS_PATH_MAX];
    uint64_t size;
    char sha256[65];
} ts_file_entry;

typedef struct {
    uint32_t schema_version;
    char snapshot_id[TS_ID_MAX];
    char created_at[32];
    char source_path[TS_PATH_MAX];
    ts_archive_class archive_class;
    char completion_state[32];
    char notes[TS_MESSAGE_MAX];
    uint32_t file_count;
    uint64_t total_bytes;
    ts_file_entry *files;
} ts_manifest;

typedef enum { JT_OBJECT, JT_ARRAY, JT_STRING, JT_PRIMITIVE } json_type;
typedef struct { int start, end, parent; json_type type; } json_token;

static unsigned g_id_counter = 0;

static ts_result apply_retention_locked(const char *storage_root, unsigned keep_count, uint32_t *removed_count);

static void set_error(char *buffer, size_t size, const char *format, ...) {
    va_list ap;
    if (!buffer || size == 0) return;
    va_start(ap, format);
    (void)vsnprintf(buffer, size, format, ap);
    va_end(ap);
}

static int path_join(char *out, size_t size, const char *left, const char *right) {
    size_t n;
    if (!out || !left || !right) return -1;
    n = strlen(left);
    if (n + strlen(right) + 2 > size) return -1;
    {
        size_t r = strlen(right);
        size_t separator = (n && left[n-1] == '/') ? 0U : 1U;
        if (n + separator + r + 1U > size) return -1;
        memcpy(out, left, n);
        if (separator) out[n++] = '/';
        memcpy(out + n, right, r);
        out[n + r] = '\0';
    }
    return 0;
}

static int valid_id(const char *id) {
    size_t i;
    if (!id || !*id || strlen(id) >= TS_ID_MAX || strstr(id, "..")) return 0;
    for (i = 0; id[i]; ++i) {
        char c = id[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return 1;
}

static int valid_relative_path(const char *path) {
    const char *p, *start;
    if (!path || !*path || path[0] == '/' || strlen(path) >= TS_PATH_MAX) return 0;
    p = path;
    while (*p) {
        start = p;
        while (*p && *p != '/') ++p;
        if ((p - start) == 0 || ((p - start) == 1 && start[0] == '.') ||
            ((p - start) == 2 && start[0] == '.' && start[1] == '.')) return 0;
        if (*p == '/') ++p;
    }
    return 1;
}

static int valid_directory_path(const char *path) {
    const char *p, *start;
    size_t length;
    if (!path || !*path || (length = strlen(path)) >= TS_PATH_MAX) return 0;
    if (length > 1 && path[0] == '/' && path[1] == '/') return 0;
    p = path;
    if (*p == '/') ++p;
    while (*p) {
        start = p;
        while (*p && *p != '/') ++p;
        if ((p - start) == 1 && start[0] == '.') return 0;
        if ((p - start) == 2 && start[0] == '.' && start[1] == '.') return 0;
        if (*p == '/') {
            ++p;
            if (!*p) break;
            if (*p == '/') return 0;
        }
    }
    return 1;
}

static size_t trim_trailing_slashes(const char *path) {
    size_t length = strlen(path);
    while (length > 1 && path[length - 1] == '/') --length;
    return length;
}

static int normalize_directory_path(char *out, size_t out_size, const char *path) {
    size_t length;
    if (!out || !valid_directory_path(path)) return -1;
    length = trim_trailing_slashes(path);
    if (length + 1U > out_size) return -1;
    memcpy(out, path, length);
    out[length] = '\0';
    return 0;
}

static int path_is_same_or_child(const char *parent, const char *candidate) {
    size_t parent_length = trim_trailing_slashes(parent);
    size_t candidate_length = trim_trailing_slashes(candidate);
    if (parent_length > candidate_length) return 0;
    if (parent_length == 1 && parent[0] == '/') return candidate[0] == '/';
    if (memcmp(parent, candidate, parent_length) != 0) return 0;
    return parent_length == candidate_length || candidate[parent_length] == '/';
}

static int paths_overlap(const char *left, const char *right) {
    return path_is_same_or_child(left, right) || path_is_same_or_child(right, left);
}

static int snapshot_path_matches_id(const char *snapshot_dir, const char *snapshot_id) {
    size_t length = strlen(snapshot_dir), start;
    while (length > 1 && snapshot_dir[length - 1] == '/') --length;
    start = length;
    while (start > 0 && snapshot_dir[start - 1] != '/') --start;
    return length > start && strlen(snapshot_id) == length - start &&
           memcmp(snapshot_dir + start, snapshot_id, length - start) == 0;
}

static int mkdir_one(const char *path) {
    rtfs_stat st;
    if (rtfs_mkdir(path, 0700) == 0) return 0;
    if (rtfs_stat_path(path, &st) != 0 || st.type != RTFS_TYPE_DIRECTORY) return -1;
    return 0;
}

static int mkdir_p(const char *path) {
    char temp[TS_PATH_MAX];
    size_t i, n;
    if (!path || !*path || strlen(path) >= sizeof(temp)) return -1;
    (void)snprintf(temp, sizeof(temp), "%s", path);
    n = strlen(temp);
    for (i = 1; i < n; ++i) {
        if (temp[i] == '/') {
            char saved = temp[i];
            temp[i] = '\0';
            if (*temp && mkdir_one(temp) != 0) return -1;
            temp[i] = saved;
        }
    }
    return mkdir_one(temp);
}

static int remove_tree(const char *path) {
    rtfs_stat st;
    rtfs_dir dir;
    rtfs_dirent entry;
    char child[TS_PATH_MAX];
    int result, close_result;
    if (rtfs_stat_path(path, &st) != 0) return -1;
    if (st.type != RTFS_TYPE_DIRECTORY) return rtfs_remove(path);
    if (rtfs_dir_open(&dir, path) != 0) return -1;
    while ((result = rtfs_dir_read(&dir, &entry)) > 0) {
        if (!strcmp(entry.name, ".") || !strcmp(entry.name, "..")) continue;
        if (path_join(child, sizeof(child), path, entry.name) != 0 || remove_tree(child) != 0) {
            (void)rtfs_dir_close(&dir);
            return -1;
        }
    }
    close_result = rtfs_dir_close(&dir);
    if (result < 0 || close_result != 0) return -1;
    return rtfs_rmdir(path);
}

static int acquire_lock(const char *root, int *lock_fd) {
    char path[TS_PATH_MAX];
    int fd;
    if (!root || !lock_fd || mkdir_p(root) != 0 ||
        path_join(path, sizeof(path), root, ".tai-saver.lock") != 0) return -1;
    fd = rtfs_open(path, RTFS_OPEN_WRITE | RTFS_OPEN_CREATE |
                   RTFS_OPEN_WRITE_LOCK | RTFS_OPEN_NOWAIT, 0600);
    if (fd < 0) return -1;
    *lock_fd = fd;
    return 0;
}
static void release_lock(int fd) {
    if (fd >= 0) (void)rtfs_close(fd);
}

static int sync_output_fd(int fd) {
    return rtfs_sync(fd);
}

static int class_directory(ts_archive_class archive_class, char *out, size_t size) {
    const char *name = ts_archive_class_name(archive_class);
    if (!name || !out || size <= strlen(name)) return -1;
    (void)snprintf(out, size, "%s", name);
    return 0;
}

const char *ts_archive_class_name(ts_archive_class archive_class) {
    switch (archive_class) {
        case TS_ARCHIVE_AUTOMATIC: return "automatic";
        case TS_ARCHIVE_KNOWN_GOOD: return "known-good";
        case TS_ARCHIVE_FAILED: return "failed";
        case TS_ARCHIVE_PRE_RESTORE: return "pre-restore";
        default: return NULL;
    }
}

const char *ts_result_name(ts_result result) {
    switch (result) {
        case TS_OK: return "ok";
        case TS_ERR_ARGUMENT: return "invalid-argument";
        case TS_ERR_IO: return "io-error";
        case TS_ERR_MEMORY: return "out-of-memory";
        case TS_ERR_UNSUPPORTED_FILE: return "unsupported-file-type";
        case TS_ERR_SOURCE_CHANGED: return "source-changed-during-copy";
        case TS_ERR_NO_SPACE: return "insufficient-storage";
        case TS_ERR_MANIFEST: return "invalid-manifest";
        case TS_ERR_INTEGRITY: return "integrity-check-failed";
        case TS_ERR_NOT_FOUND: return "not-found";
        case TS_ERR_EXISTS: return "already-exists";
        case TS_ERR_RETENTION: return "retention-error";
        case TS_ERR_TRANSACTION: return "transaction-error";
        case TS_ERR_LIMIT: return "configured-limit-exceeded";
        default: return "unknown-error";
    }
}

static void manifest_free(ts_manifest *manifest) {
    if (!manifest) return;
    free(manifest->files);
    memset(manifest, 0, sizeof(*manifest));
}

static int manifest_allocate(ts_manifest *manifest, uint32_t count) {
    if (count > TS_MAX_FILES) return -1;
    manifest->files = (ts_file_entry *)calloc(count ? count : 1, sizeof(ts_file_entry));
    if (!manifest->files) return -1;
    manifest->file_count = count;
    return 0;
}

static int append_file(ts_manifest *manifest, const char *relative, uint64_t size) {
    ts_file_entry *next;
    uint32_t new_count;
    if (manifest->file_count >= TS_MAX_FILES || !valid_relative_path(relative)) return -1;
    new_count = manifest->file_count + 1;
    next = (ts_file_entry *)realloc(manifest->files, (size_t)new_count * sizeof(ts_file_entry));
    if (!next) return -1;
    manifest->files = next;
    memset(&manifest->files[new_count - 1], 0, sizeof(ts_file_entry));
    (void)snprintf(manifest->files[new_count - 1].path, TS_PATH_MAX, "%s", relative);
    manifest->files[new_count - 1].size = size;
    manifest->file_count = new_count;
    if (UINT64_MAX - manifest->total_bytes < size) return -1;
    manifest->total_bytes += size;
    return 0;
}

static int compare_entry_path(const void *a, const void *b) {
    const ts_file_entry *left = (const ts_file_entry *)a;
    const ts_file_entry *right = (const ts_file_entry *)b;
    return strcmp(left->path, right->path);
}

static int scan_directory(const char *base, const char *relative, ts_manifest *manifest, unsigned depth) {
    char directory[TS_PATH_MAX], child_path[TS_PATH_MAX], child_relative[TS_PATH_MAX];
    rtfs_dir dir;
    rtfs_dirent entry;
    int result, close_result;
    if (depth > 24) return -1;
    if (*relative) {
        if (path_join(directory, sizeof(directory), base, relative) != 0) return -1;
    } else {
        if (strlen(base) >= sizeof(directory)) return -1;
        (void)snprintf(directory, sizeof(directory), "%s", base);
    }
    if (rtfs_dir_open(&dir, directory) != 0) return -1;
    while ((result = rtfs_dir_read(&dir, &entry)) > 0) {
        size_t rel_len;
        if (!strcmp(entry.name, ".") || !strcmp(entry.name, "..")) continue;
        if (*relative) {
            if (path_join(child_relative, sizeof(child_relative), relative, entry.name) != 0) goto fail;
        } else {
            if (strlen(entry.name) >= sizeof(child_relative)) goto fail;
            (void)snprintf(child_relative, sizeof(child_relative), "%s", entry.name);
        }
        if (!valid_relative_path(child_relative) ||
            path_join(child_path, sizeof(child_path), base, child_relative) != 0) goto fail;
        if (entry.type == RTFS_TYPE_DIRECTORY) {
            if (scan_directory(base, child_relative, manifest, depth + 1) != 0) goto fail;
        } else if (entry.type == RTFS_TYPE_FILE) {
            if (append_file(manifest, child_relative, entry.size) != 0) goto fail;
        } else {
            (void)rtfs_dir_close(&dir);
            return -2;
        }
        rel_len = strlen(child_relative);
        if (rel_len >= TS_PATH_MAX) goto fail;
    }
    close_result = rtfs_dir_close(&dir);
    return result < 0 || close_result != 0 ? -1 : 0;
fail:
    (void)rtfs_dir_close(&dir);
    return -1;
}

static int hash_file(const char *path, uint64_t *size_out, char hash_out[65]) {
    int fd;
    uint8_t buffer[TS_COPY_BUFFER], digest[32];
    int64_t got;
    uint64_t total = 0;
    ts_sha256_ctx ctx;
    fd = rtfs_open(path, RTFS_OPEN_READ, 0);
    if (fd < 0) return -1;
    ts_sha256_init(&ctx);
    while ((got = rtfs_read(fd, buffer, sizeof(buffer))) > 0) {
        if (UINT64_MAX - total < (uint64_t)got) { (void)rtfs_close(fd); return -1; }
        total += (uint64_t)got;
        ts_sha256_update(&ctx, buffer, (size_t)got);
    }
    {
        int close_result = rtfs_close(fd);
        if (got < 0 || close_result != 0) return -1;
    }
    ts_sha256_final(&ctx, digest);
    ts_sha256_hex(digest, hash_out);
    if (size_out) *size_out = total;
    return 0;
}

static int copy_file_hash(const char *source, const char *destination, uint64_t expected_size, char hash_out[65]) {
    int in_fd = -1, out_fd = -1;
    rtfs_stat before, opened, after;
    uint8_t buffer[TS_COPY_BUFFER], digest[32];
    int64_t got;
    uint64_t total = 0;
    ts_sha256_ctx ctx;
    if (rtfs_stat_path(source, &before) != 0 || before.type != RTFS_TYPE_FILE) return -1;
    in_fd = rtfs_open(source, RTFS_OPEN_READ, 0);
    if (in_fd < 0 || rtfs_stat_fd(in_fd, &opened) != 0 ||
        opened.type != RTFS_TYPE_FILE || opened.size != before.size) goto fail;
    out_fd = rtfs_open(destination, RTFS_OPEN_WRITE | RTFS_OPEN_CREATE | RTFS_OPEN_EXCLUSIVE, 0600);
    if (out_fd < 0) goto fail;
    ts_sha256_init(&ctx);
    while ((got = rtfs_read(in_fd, buffer, sizeof(buffer))) > 0) {
        size_t offset = 0;
        if (UINT64_MAX - total < (uint64_t)got) goto fail;
        ts_sha256_update(&ctx, buffer, (size_t)got);
        total += (uint64_t)got;
        while (offset < (size_t)got) {
            int64_t written = rtfs_write(out_fd, buffer + offset, (size_t)got - offset);
            if (written <= 0) goto fail;
            offset += (size_t)written;
        }
    }
    if (got < 0 || rtfs_stat_fd(in_fd, &after) != 0 ||
        after.type != before.type || after.size != before.size ||
        (before.mtime && after.mtime != before.mtime) ||
        total != expected_size || rtfs_sync(out_fd) != 0) goto fail;
    ts_sha256_final(&ctx, digest);
    ts_sha256_hex(digest, hash_out);
    if (rtfs_close(out_fd) != 0) { out_fd = -1; goto fail; }
    out_fd = -1;
    if (rtfs_close(in_fd) != 0) { in_fd = -1; return -1; }
    return 0;
fail:
    if (out_fd >= 0) (void)rtfs_close(out_fd);
    if (in_fd >= 0) (void)rtfs_close(in_fd);
    (void)rtfs_remove(destination);
    return -1;
}

static int ensure_parent(const char *path) {
    char temp[TS_PATH_MAX];
    char *slash;
    if (!path || strlen(path) >= sizeof(temp)) return -1;
    (void)snprintf(temp, sizeof(temp), "%s", path);
    slash = strrchr(temp, '/');
    if (!slash) return 0;
    if (slash == temp) return 0;
    *slash = '\0';
    return mkdir_p(temp);
}

typedef struct {
    int fd;
    unsigned char buffer[4096];
    size_t used;
} ts_writer;

static int writer_flush(ts_writer *writer) {
    size_t offset = 0;
    while (offset < writer->used) {
        int64_t written = rtfs_write(writer->fd, writer->buffer + offset, writer->used - offset);
        if (written <= 0) return -1;
        offset += (size_t)written;
    }
    writer->used = 0;
    return 0;
}

static int writer_bytes(ts_writer *writer, const void *data, size_t length) {
    const unsigned char *bytes = (const unsigned char *)data;
    while (length) {
        size_t available = sizeof(writer->buffer) - writer->used;
        size_t amount = length < available ? length : available;
        memcpy(writer->buffer + writer->used, bytes, amount);
        writer->used += amount;
        bytes += amount;
        length -= amount;
        if (writer->used == sizeof(writer->buffer) && writer_flush(writer) != 0) return -1;
    }
    return 0;
}

static int writer_printf(ts_writer *writer, const char *format, ...) {
    char text[2048];
    int length;
    va_list ap;
    va_start(ap, format);
    length = vsnprintf(text, sizeof(text), format, ap);
    va_end(ap);
    if (length < 0 || (size_t)length >= sizeof(text)) return -1;
    return writer_bytes(writer, text, (size_t)length);
}

static int json_write_string(ts_writer *writer, const char *text) {
    const unsigned char *p = (const unsigned char *)(text ? text : "");
    if (writer_bytes(writer, "\"", 1) != 0) return -1;
    for (; *p; ++p) {
        const char *escaped = NULL;
        char encoded[6];
        size_t length = 0;
        switch (*p) {
            case '"': escaped = "\\\""; break;
            case '\\': escaped = "\\\\"; break;
            case '\b': escaped = "\\b"; break;
            case '\f': escaped = "\\f"; break;
            case '\n': escaped = "\\n"; break;
            case '\r': escaped = "\\r"; break;
            case '\t': escaped = "\\t"; break;
            default:
                if (*p < 0x20) {
                    int n = snprintf(encoded, sizeof(encoded), "\\u%04x", (unsigned)*p);
                    if (n != 6) return -1;
                    length = 6;
                } else {
                    encoded[0] = (char)*p;
                    length = 1;
                }
        }
        if (escaped) {
            length = strlen(escaped);
            if (writer_bytes(writer, escaped, length) != 0) return -1;
        } else if (writer_bytes(writer, encoded, length) != 0) return -1;
    }
    return writer_bytes(writer, "\"", 1);
}

static int manifest_write(const char *snapshot_dir, const ts_manifest *manifest) {
    char path[TS_PATH_MAX], temp[TS_PATH_MAX];
    ts_writer writer;
    uint32_t i;
    int fd, ok = 0, close_result;
    if (path_join(path, sizeof(path), snapshot_dir, "manifest.json") != 0 ||
        path_join(temp, sizeof(temp), snapshot_dir, "manifest.json.tmp") != 0) return -1;
    (void)rtfs_remove(temp);
    fd = rtfs_open(temp, RTFS_OPEN_WRITE | RTFS_OPEN_CREATE | RTFS_OPEN_EXCLUSIVE, 0600);
    if (fd < 0) return -1;
    memset(&writer, 0, sizeof(writer));
    writer.fd = fd;
    if (writer_printf(&writer, "{\\n  \\"schema_version\\": %u,\\n  \\"snapshot_id\\": ", manifest->schema_version) != 0 ||
        json_write_string(&writer, manifest->snapshot_id) != 0 ||
        writer_bytes(&writer, ",\\n  \\"created_at\\": ", 18) != 0 ||
        json_write_string(&writer, manifest->created_at) != 0 ||
        writer_bytes(&writer, ",\\n  \\"source_path\\": ", 19) != 0 ||
        json_write_string(&writer, manifest->source_path) != 0 ||
        writer_bytes(&writer, ",\\n  \\"archive_class\\": ", 21) != 0 ||
        json_write_string(&writer, ts_archive_class_name(manifest->archive_class)) != 0 ||
        writer_bytes(&writer, ",\\n  \\"completion_state\\": ", 25) != 0 ||
        json_write_string(&writer, manifest->completion_state) != 0 ||
        writer_printf(&writer, ",\\n  \\"file_count\\": %u,\\n  \\"total_bytes\\": %" PRIu64 ",\\n  \\"files\\": [\\n",
                      manifest->file_count, manifest->total_bytes) != 0) goto done;
    for (i = 0; i < manifest->file_count; ++i) {
        if (writer_bytes(&writer, "    {\\"relative_path\\": ", 22) != 0 ||
            json_write_string(&writer, manifest->files[i].path) != 0 ||
            writer_printf(&writer, ", \\"size_bytes\\": %" PRIu64 ", \\"sha256\\": ", manifest->files[i].size) != 0 ||
            json_write_string(&writer, manifest->files[i].sha256) != 0 ||
            writer_printf(&writer, "}%s\\n", (i + 1 == manifest->file_count) ? "" : ",") != 0) goto done;
    }
    if (writer_bytes(&writer, "  ],\\n  \\"notes\\": ", 17) != 0 ||
        json_write_string(&writer, manifest->notes) != 0 ||
        writer_bytes(&writer, "\\n}\\n", 4) != 0 ||
        writer_flush(&writer) != 0 || rtfs_sync(fd) != 0) goto done;
    ok = 1;
done:
    close_result = rtfs_close(fd);
    if (close_result != 0) ok = 0;
    if (!ok) { (void)rtfs_remove(temp); return -1; }
    if (rtfs_rename(temp, path) != 0) { (void)rtfs_remove(temp); return -1; }
    return 0;
}

static int json_parse(const char *json, json_token *tokens, int capacity, int *count_out) {
    int i = 0, count = 0, parent = -1, stack[32], depth = 0;
    int length = (int)strlen(json);
    while (i < length) {
        unsigned char c = (unsigned char)json[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == ':' || c == ',') { ++i; continue; }
        if (c == '{' || c == '[') {
            int idx;
            if (count >= capacity || depth >= 32) return -1;
            idx = count++;
            tokens[idx].start = i;
            tokens[idx].end = -1;
            tokens[idx].parent = parent;
            tokens[idx].type = c == '{' ? JT_OBJECT : JT_ARRAY;
            stack[depth++] = idx;
            parent = idx;
            ++i;
            continue;
        }
        if (c == '}' || c == ']') {
            if (depth <= 0) return -1;
            if ((c == '}' && tokens[stack[depth - 1]].type != JT_OBJECT) ||
                (c == ']' && tokens[stack[depth - 1]].type != JT_ARRAY)) return -1;
            tokens[stack[--depth]].end = i + 1;
            parent = depth ? stack[depth - 1] : -1;
            ++i;
            continue;
        }
        if (c == '"') {
            int start = ++i, escaped = 0;
            while (i < length) {
                if (!escaped && json[i] == '"') break;
                if (!escaped && json[i] == '\\') escaped = 1;
                else escaped = 0;
                ++i;
            }
            if (i >= length || count >= capacity) return -1;
            tokens[count].start = start;
            tokens[count].end = i;
            tokens[count].parent = parent;
            tokens[count].type = JT_STRING;
            ++count;
            ++i;
            continue;
        }
        {
            int start = i;
            while (i < length && json[i] != ',' && json[i] != '}' && json[i] != ']' &&
                   json[i] != ' ' && json[i] != '\t' && json[i] != '\r' && json[i] != '\n') ++i;
            if (i == start || count >= capacity) return -1;
            tokens[count].start = start;
            tokens[count].end = i;
            tokens[count].parent = parent;
            tokens[count].type = JT_PRIMITIVE;
            ++count;
        }
    }
    if (depth != 0 || count == 0) return -1;
    *count_out = count;
    return 0;
}

static int token_after(const json_token *tokens, int count, int index) {
    int i = index + 1;
    while (i < count && tokens[i].start < tokens[index].end) ++i;
    return i;
}

static int token_string(const char *json, const json_token *token, char *out, size_t out_size) {
    size_t used = 0;
    int i;
    if (!token || token->type != JT_STRING || !out || out_size == 0) return -1;
    for (i = token->start; i < token->end; ++i) {
        unsigned char c = (unsigned char)json[i];
        if (c == '\\') {
            unsigned char e;
            if (++i >= token->end) return -1;
            e = (unsigned char)json[i];
            switch (e) {
                case '"': c='"'; break; case '\\': c='\\'; break; case '/': c='/'; break;
                case 'b': c='\b'; break; case 'f': c='\f'; break; case 'n': c='\n'; break;
                case 'r': c='\r'; break; case 't': c='\t'; break;
                case 'u': {
                    unsigned value = 0, j;
                    if (i + 4 >= token->end) return -1;
                    for (j = 1; j <= 4; ++j) {
                        unsigned char h = (unsigned char)json[i+j];
                        value <<= 4;
                        if (h >= '0' && h <= '9') value |= h - '0';
                        else if (h >= 'a' && h <= 'f') value |= h - 'a' + 10;
                        else if (h >= 'A' && h <= 'F') value |= h - 'A' + 10;
                        else return -1;
                    }
                    if (value > 0x7f) return -1;
                    c = (unsigned char)value;
                    i += 4;
                    break;
                }
                default: return -1;
            }
        }
        if (used + 1 >= out_size) return -1;
        out[used++] = (char)c;
    }
    out[used] = '\0';
    return 0;
}
static int object_get(const char *json, const json_token *tokens, int count, int object, const char *key) {
    int i;
    if (object < 0 || object >= count || tokens[object].type != JT_OBJECT) return -1;
    i = object + 1;
    while (i < count && tokens[i].start < tokens[object].end) {
        int value;
        if (tokens[i].parent != object) { ++i; continue; }
        if (tokens[i].type != JT_STRING) return -1;
        value = token_after(tokens, count, i);
        if (value >= count || tokens[value].parent != object) return -1;
        {
            char candidate[64];
            if (token_string(json, &tokens[i], candidate, sizeof(candidate)) == 0 && !strcmp(candidate, key)) return value;
        }
        i = token_after(tokens, count, value);
    }
    return -1;
}

static int token_u64(const char *json, const json_token *token, uint64_t *out) {
    char temp[32], *end;
    unsigned long long value;
    size_t n;
    if (!token || token->type != JT_PRIMITIVE || !out) return -1;
    n = (size_t)(token->end - token->start);
    if (n == 0 || n >= sizeof(temp)) return -1;
    memcpy(temp, json + token->start, n); temp[n] = '\0';
    if (temp[0] == '-') return -1;
    errno = 0;
    value = strtoull(temp, &end, 10);
    if (errno || *end) return -1;
    *out = (uint64_t)value;
    return 0;
}

static int parse_class(const char *value, ts_archive_class *out) {
    if (!strcmp(value, "automatic")) *out = TS_ARCHIVE_AUTOMATIC;
    else if (!strcmp(value, "known-good")) *out = TS_ARCHIVE_KNOWN_GOOD;
    else if (!strcmp(value, "failed")) *out = TS_ARCHIVE_FAILED;
    else if (!strcmp(value, "pre-restore")) *out = TS_ARCHIVE_PRE_RESTORE;
    else return -1;
    return 0;
}

static int manifest_read(const char *snapshot_dir, ts_manifest *manifest) {
    char path[TS_PATH_MAX], *json = NULL, value[TS_PATH_MAX];
    rtfs_stat file_stat;
    size_t length, total = 0;
    int fd = -1;
    int64_t got;
    json_token *tokens = NULL;
    int token_count = 0, root = 0, t, files_idx, i;
    uint64_t number;
    uint32_t parsed_count = 0, expected_count = 0;
    memset(manifest, 0, sizeof(*manifest));
    if (path_join(path, sizeof(path), snapshot_dir, "manifest.json") != 0) return -1;
    if (rtfs_stat_path(path, &file_stat) != 0 || file_stat.type != RTFS_TYPE_FILE ||
        file_stat.size > TS_MAX_MANIFEST_BYTES || file_stat.size > SIZE_MAX - 1U) return -1;
    length = (size_t)file_stat.size;
    fd = rtfs_open(path, RTFS_OPEN_READ, 0);
    if (fd < 0) return -1;
    json = (char *)malloc(length + 1U);
    if (!json) goto fail;
    while (total < length) {
        got = rtfs_read(fd, json + total, length - total);
        if (got <= 0) goto fail;
        total += (size_t)got;
    }
    {
        unsigned char extra;
        got = rtfs_read(fd, &extra, 1);
        if (got != 0) goto fail;
    }
    if (rtfs_close(fd) != 0) { fd = -1; goto fail; }
    fd = -1;
    json[length] = '\0';
    tokens = (json_token *)calloc(TS_JSON_TOKEN_LIMIT, sizeof(json_token));
    if (!tokens || json_parse(json, tokens, (int)TS_JSON_TOKEN_LIMIT, &token_count) != 0 || tokens[root].type != JT_OBJECT) goto fail;
#define GET_STRING(field, target, target_size) do { t = object_get(json, tokens, token_count, root, field); if (t < 0 || token_string(json, &tokens[t], target, target_size) != 0) goto fail; } while (0)
    t = object_get(json, tokens, token_count, root, "schema_version");
    if (t < 0 || token_u64(json, &tokens[t], &number) != 0 || number != TS_SCHEMA_VERSION) goto fail;
    manifest->schema_version = (uint32_t)number;
    GET_STRING("snapshot_id", manifest->snapshot_id, sizeof(manifest->snapshot_id));
    GET_STRING("created_at", manifest->created_at, sizeof(manifest->created_at));
    GET_STRING("source_path", manifest->source_path, sizeof(manifest->source_path));
    GET_STRING("completion_state", value, sizeof(value));
    if (strcmp(value, "complete") != 0) goto fail;
    memcpy(manifest->completion_state, "complete", sizeof("complete"));
    GET_STRING("archive_class", value, sizeof(value));
    if (parse_class(value, &manifest->archive_class) != 0) goto fail;
    GET_STRING("notes", manifest->notes, sizeof(manifest->notes));
    t = object_get(json, tokens, token_count, root, "file_count");
    if (t < 0 || token_u64(json, &tokens[t], &number) != 0 || number > TS_MAX_FILES) goto fail;
    expected_count = (uint32_t)number;
    t = object_get(json, tokens, token_count, root, "total_bytes");
    if (t < 0 || token_u64(json, &tokens[t], &manifest->total_bytes) != 0) goto fail;
    files_idx = object_get(json, tokens, token_count, root, "files");
    if (files_idx < 0 || tokens[files_idx].type != JT_ARRAY || manifest_allocate(manifest, expected_count) != 0) goto fail;
    i = files_idx + 1;
    {
        uint64_t calculated_total = 0;
    while (i < token_count && tokens[i].start < tokens[files_idx].end) {
        int p, size_idx, hash_idx;
        uint32_t slot;
        if (tokens[i].parent != files_idx) { ++i; continue; }
        if (tokens[i].type != JT_OBJECT || parsed_count >= expected_count) goto fail;
        slot = parsed_count;
        p = object_get(json, tokens, token_count, i, "relative_path");
        size_idx = object_get(json, tokens, token_count, i, "size_bytes");
        hash_idx = object_get(json, tokens, token_count, i, "sha256");
        if (p < 0 || size_idx < 0 || hash_idx < 0 || tokens[i].type != JT_OBJECT ||
            token_string(json, &tokens[p], manifest->files[slot].path, TS_PATH_MAX) != 0 ||
            !valid_relative_path(manifest->files[slot].path) ||
            token_u64(json, &tokens[size_idx], &manifest->files[slot].size) != 0 ||
            token_string(json, &tokens[hash_idx], manifest->files[slot].sha256, sizeof(manifest->files[slot].sha256)) != 0 ||
            strlen(manifest->files[slot].sha256) != 64) goto fail;
        {
            size_t h;
            for (h = 0; h < 64; ++h) if (!((manifest->files[slot].sha256[h] >= '0' && manifest->files[slot].sha256[h] <= '9') ||
                (manifest->files[slot].sha256[h] >= 'a' && manifest->files[slot].sha256[h] <= 'f') ||
                (manifest->files[slot].sha256[h] >= 'A' && manifest->files[slot].sha256[h] <= 'F'))) goto fail;
        }
        if (UINT64_MAX - calculated_total < manifest->files[slot].size) goto fail;
        calculated_total += manifest->files[slot].size;
        if (parsed_count > 0 && strcmp(manifest->files[slot - 1].path, manifest->files[slot].path) >= 0) goto fail;
        i = token_after(tokens, token_count, i);
        ++parsed_count;
        if (parsed_count > TS_MAX_FILES) goto fail;
    }
        if (calculated_total != manifest->total_bytes) goto fail;
    }
    if (parsed_count != expected_count || !valid_id(manifest->snapshot_id) || !manifest->source_path[0]) goto fail;
#undef GET_STRING
    free(tokens); free(json);
    return 0;
fail:
    if (fd >= 0) (void)rtfs_close(fd);
    free(tokens); free(json); manifest_free(manifest);
    return -1;
}

static int verify_manifest_payload(const char *snapshot_dir, const ts_manifest *manifest, int hash_files) {
    char payload[TS_PATH_MAX], path[TS_PATH_MAX], hash[65];
    ts_manifest actual;
    uint32_t i;
    int result = -1;
    memset(&actual, 0, sizeof(actual));
    if (path_join(payload, sizeof(payload), snapshot_dir, "payload") != 0) return -1;
    if (scan_directory(payload, "", &actual, 0) != 0) goto done;
    if (actual.file_count != manifest->file_count) goto done;
    qsort(actual.files, actual.file_count, sizeof(ts_file_entry), compare_entry_path);
    for (i = 0; i < manifest->file_count; ++i) {
        uint64_t size = 0;
        if (strcmp(actual.files[i].path, manifest->files[i].path) != 0 ||
            actual.files[i].size != manifest->files[i].size ||
            path_join(path, sizeof(path), payload, manifest->files[i].path) != 0) goto done;
        if (hash_files) {
            if (hash_file(path, &size, hash) != 0 || size != manifest->files[i].size || strcmp(hash, manifest->files[i].sha256) != 0)
                goto done;
        }
    }
    result = 0;
done:
    manifest_free(&actual);
    return result;
}

static int storage_has_space(const char *root, uint64_t bytes, uint32_t files) {
    uint64_t available, needed;
    const char *colon;
    char device[16];
    size_t length;
    needed = bytes + bytes / 20U + (uint64_t)files * 512U + 65536U;
    if (!root || !(colon = strchr(root, ':'))) return -1;
    length = (size_t)(colon - root) + 1U;
    if (!length || length >= sizeof(device)) return -1;
    memcpy(device, root, length);
    device[length] = '\0';
    if (rtfs_free_bytes(device, &available) != 0) return -1;
    return available >= needed ? 0 : -1;
}

static int make_snapshot_id(char out[TS_ID_MAX]) {
    time_t now = time(NULL);
    struct tm tm_utc;
    char stamp[24];
    if (!gmtime_r(&now, &tm_utc) || strftime(stamp, sizeof(stamp), "%Y%m%dT%H%M%SZ", &tm_utc) == 0) return -1;
    ++g_id_counter;
    if (snprintf(out, TS_ID_MAX, "%s-%ld-%u", stamp, (long)getpid(), g_id_counter) >= TS_ID_MAX) return -1;
    return 0;
}

static int copy_manifest_payload(const char *source_dir, const char *payload_dir, ts_manifest *manifest) {
    char source[TS_PATH_MAX], destination[TS_PATH_MAX];
    uint32_t i;
    if (mkdir_p(payload_dir) != 0) return -1;
    for (i = 0; i < manifest->file_count; ++i) {
        if (path_join(source, sizeof(source), source_dir, manifest->files[i].path) != 0 ||
            path_join(destination, sizeof(destination), payload_dir, manifest->files[i].path) != 0 ||
            ensure_parent(destination) != 0 ||
            copy_file_hash(source, destination, manifest->files[i].size, manifest->files[i].sha256) != 0) return -1;
    }
    return 0;
}

static ts_result create_snapshot_locked(const char *source_dir, const char *storage_root,
                                        ts_archive_class archive_class, unsigned retention,
                                        const char *notes, ts_snapshot_result *out) {
    ts_manifest manifest;
    char class_name[32], class_dir[TS_PATH_MAX], staging_dir[TS_PATH_MAX];
    char stage_path[TS_PATH_MAX], final_path[TS_PATH_MAX], payload[TS_PATH_MAX];
    struct stat st;
    struct tm tm_utc;
    time_t now;
    uint32_t removed = 0;
    ts_result result = TS_ERR_IO;
    memset(&manifest, 0, sizeof(manifest));
    memset(out, 0, sizeof(*out));
    if (!source_dir || !storage_root || !ts_archive_class_name(archive_class) ||
        (archive_class == TS_ARCHIVE_AUTOMATIC && retention < 2)) return TS_ERR_ARGUMENT;
    if (lstat(source_dir, &st) != 0 || !S_ISDIR(st.st_mode)) return TS_ERR_NOT_FOUND;
    if (mkdir_p(storage_root) != 0 || class_directory(archive_class, class_name, sizeof(class_name)) != 0 ||
        path_join(class_dir, sizeof(class_dir), storage_root, class_name) != 0 || mkdir_p(class_dir) != 0 ||
        path_join(staging_dir, sizeof(staging_dir), storage_root, "staging") != 0 || mkdir_p(staging_dir) != 0)
        return TS_ERR_IO;
    if (scan_directory(source_dir, "", &manifest, 0) != 0) {
        result = TS_ERR_UNSUPPORTED_FILE;
        goto done;
    }
    qsort(manifest.files, manifest.file_count, sizeof(ts_file_entry), compare_entry_path);
    if (storage_has_space(storage_root, manifest.total_bytes, manifest.file_count) != 0) {
        result = TS_ERR_NO_SPACE;
        goto done;
    }
    if (make_snapshot_id(manifest.snapshot_id) != 0) { result = TS_ERR_IO; goto done; }
    (void)snprintf(manifest.source_path, sizeof(manifest.source_path), "%s", source_dir);
    manifest.schema_version = TS_SCHEMA_VERSION;
    manifest.archive_class = archive_class;
    (void)snprintf(manifest.completion_state, sizeof(manifest.completion_state), "complete");
    if (notes) (void)snprintf(manifest.notes, sizeof(manifest.notes), "%s", notes);
    now = time(NULL);
    if (!gmtime_r(&now, &tm_utc) || strftime(manifest.created_at, sizeof(manifest.created_at), "%Y-%m-%dT%H:%M:%SZ", &tm_utc) == 0) {
        result = TS_ERR_IO; goto done;
    }
    if (path_join(stage_path, sizeof(stage_path), staging_dir, manifest.snapshot_id) != 0 ||
        path_join(final_path, sizeof(final_path), class_dir, manifest.snapshot_id) != 0 ||
        path_join(payload, sizeof(payload), stage_path, "payload") != 0) { result = TS_ERR_ARGUMENT; goto done; }
    if (lstat(stage_path, &st) == 0 || lstat(final_path, &st) == 0) { result = TS_ERR_EXISTS; goto done; }
    if (mkdir(stage_path, 0700) != 0) { result = TS_ERR_IO; goto done; }
    if (copy_manifest_payload(source_dir, payload, &manifest) != 0) {
        (void)remove_tree(stage_path);
        result = TS_ERR_SOURCE_CHANGED;
        goto done;
    }
    if (manifest_write(stage_path, &manifest) != 0) {
        (void)remove_tree(stage_path);
        result = TS_ERR_IO;
        goto done;
    }
    if (verify_manifest_payload(stage_path, &manifest, 1) != 0) {
        (void)remove_tree(stage_path);
        result = TS_ERR_INTEGRITY;
        goto done;
    }
    if (rename(stage_path, final_path) != 0) {
        (void)remove_tree(stage_path);
        result = TS_ERR_IO;
        goto done;
    }
    (void)snprintf(out->snapshot_id, sizeof(out->snapshot_id), "%s", manifest.snapshot_id);
    (void)snprintf(out->snapshot_path, sizeof(out->snapshot_path), "%s", final_path);
    out->file_count = manifest.file_count;
    out->total_bytes = manifest.total_bytes;
    result = TS_OK;
    if (archive_class == TS_ARCHIVE_AUTOMATIC) {
        ts_result retention_result = apply_retention_locked(storage_root, retention, &removed);
        out->retention_removed = removed;
        if (retention_result != TS_OK) set_error(out->warning, sizeof(out->warning), "Snapshot is safe, but automatic retention failed (%s).", ts_result_name(retention_result));
    }
done:
    manifest_free(&manifest);
    return result;
}

static ts_result verify_snapshot_internal(const char *snapshot_dir, ts_snapshot_info *out, ts_manifest *manifest_out) {
    ts_manifest manifest;
    if (!snapshot_dir) return TS_ERR_ARGUMENT;
    memset(&manifest, 0, sizeof(manifest));
    if (manifest_read(snapshot_dir, &manifest) != 0) return TS_ERR_MANIFEST;
    if (!snapshot_path_matches_id(snapshot_dir, manifest.snapshot_id)) {
        manifest_free(&manifest);
        return TS_ERR_MANIFEST;
    }
    if (verify_manifest_payload(snapshot_dir, &manifest, 1) != 0) {
        manifest_free(&manifest);
        return TS_ERR_INTEGRITY;
    }
    if (out) {
        memset(out, 0, sizeof(*out));
        out->archive_class = manifest.archive_class;
        out->file_count = manifest.file_count;
        out->total_bytes = manifest.total_bytes;
        (void)snprintf(out->snapshot_id, sizeof(out->snapshot_id), "%s", manifest.snapshot_id);
        (void)snprintf(out->created_at, sizeof(out->created_at), "%s", manifest.created_at);
    }
    if (manifest_out) *manifest_out = manifest;
    else manifest_free(&manifest);
    return TS_OK;
}

ts_result ts_verify_snapshot(const char *snapshot_dir, ts_snapshot_info *out) {
    return verify_snapshot_internal(snapshot_dir, out, NULL);
}

static ts_result apply_retention_locked(const char *storage_root, unsigned keep_count, uint32_t *removed_count) {
    typedef struct { char id[TS_ID_MAX]; } snapshot_name;
    char automatic_dir[TS_PATH_MAX], path[TS_PATH_MAX];
    DIR *dir;
    struct dirent *entry;
    snapshot_name *items = NULL;
    size_t count = 0, capacity = 0, i;
    uint32_t removed = 0;
    ts_result result = TS_OK;
    if (removed_count) *removed_count = 0;
    if (!valid_directory_path(storage_root) || keep_count < 2 || keep_count > 1000) return TS_ERR_ARGUMENT;
    if (path_join(automatic_dir, sizeof(automatic_dir), storage_root, "automatic") != 0 || mkdir_p(automatic_dir) != 0)
        return TS_ERR_IO;
    dir = opendir(automatic_dir);
    if (!dir) return TS_ERR_IO;
    while ((entry = readdir(dir)) != NULL) {
        ts_snapshot_info info;
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..") || !valid_id(entry->d_name)) continue;
        if (path_join(path, sizeof(path), automatic_dir, entry->d_name) != 0) continue;
        if (ts_verify_snapshot(path, &info) != TS_OK || info.archive_class != TS_ARCHIVE_AUTOMATIC) continue;
        if (count == capacity) {
            size_t next_capacity = capacity ? capacity * 2 : 8;
            snapshot_name *next = (snapshot_name *)realloc(items, next_capacity * sizeof(snapshot_name));
            if (!next) { result = TS_ERR_MEMORY; goto done; }
            items = next; capacity = next_capacity;
        }
        memcpy(items[count].id, entry->d_name, strlen(entry->d_name) + 1U);
        ++count;
    }
    (void)closedir(dir);
    dir = NULL;
    /* Snapshot IDs begin with UTC timestamps, so lexical order is chronological. */
    for (i = 0; i < count; ++i) {
        size_t j;
        for (j = i + 1; j < count; ++j) {
            if (strcmp(items[i].id, items[j].id) > 0) {
                snapshot_name swap = items[i]; items[i] = items[j]; items[j] = swap;
            }
        }
    }
    while (count > keep_count) {
        if (path_join(path, sizeof(path), automatic_dir, items[0].id) != 0 || remove_tree(path) != 0) {
            result = TS_ERR_RETENTION;
            goto done;
        }
        memmove(items, items + 1, (count - 1) * sizeof(snapshot_name));
        --count;
        ++removed;
    }
done:
    if (dir) (void)closedir(dir);
    free(items);
    if (removed_count) *removed_count = removed;
    return result;
}

ts_result ts_apply_retention(const char *storage_root, unsigned keep_count, uint32_t *removed_count) {
    int lock_fd = -1;
    ts_result result;
    if (!storage_root || keep_count < 2 || keep_count > 1000) return TS_ERR_ARGUMENT;
    if (acquire_lock(storage_root, &lock_fd) != 0) return TS_ERR_TRANSACTION;
    result = apply_retention_locked(storage_root, keep_count, removed_count);
    release_lock(lock_fd);
    return result;
}

ts_result ts_create_snapshot(const char *source_dir, const char *storage_root,
                             ts_archive_class archive_class, unsigned automatic_retention,
                             const char *notes, ts_snapshot_result *out) {
    char normalized_source[TS_PATH_MAX], normalized_root[TS_PATH_MAX];
    int lock_fd = -1;
    ts_result result;
    if (!out || normalize_directory_path(normalized_source, sizeof(normalized_source), source_dir) != 0 ||
        normalize_directory_path(normalized_root, sizeof(normalized_root), storage_root) != 0 ||
        paths_overlap(normalized_source, normalized_root)) return TS_ERR_ARGUMENT;
    memset(out, 0, sizeof(*out));
    if (acquire_lock(normalized_root, &lock_fd) != 0) return TS_ERR_TRANSACTION;
    result = create_snapshot_locked(normalized_source, normalized_root, archive_class,
                                    automatic_retention, notes, out);
    release_lock(lock_fd);
    return result;
}

static int manifest_set_class(const char *snapshot_dir, ts_manifest *manifest, ts_archive_class archive_class) {
    manifest->archive_class = archive_class;
    return manifest_write(snapshot_dir, manifest);
}

ts_result ts_classify_snapshot(const char *storage_root, const char *snapshot_id, ts_archive_class target_class) {
    char automatic_dir[TS_PATH_MAX], target_dir[TS_PATH_MAX], source[TS_PATH_MAX], target[TS_PATH_MAX];
    ts_manifest manifest;
    ts_snapshot_info info;
    int lock_fd = -1;
    ts_result result = TS_ERR_IO;
    memset(&manifest, 0, sizeof(manifest));
    if (!storage_root || !valid_id(snapshot_id) ||
        !valid_directory_path(storage_root) ||
        (target_class != TS_ARCHIVE_KNOWN_GOOD && target_class != TS_ARCHIVE_FAILED)) return TS_ERR_ARGUMENT;
    if (acquire_lock(storage_root, &lock_fd) != 0) return TS_ERR_TRANSACTION;
    if (path_join(automatic_dir, sizeof(automatic_dir), storage_root, "automatic") != 0 ||
        path_join(target_dir, sizeof(target_dir), storage_root, ts_archive_class_name(target_class)) != 0 ||
        path_join(source, sizeof(source), automatic_dir, snapshot_id) != 0 ||
        path_join(target, sizeof(target), target_dir, snapshot_id) != 0) goto done;
    result = verify_snapshot_internal(source, &info, &manifest);
    if (result != TS_OK) goto done;
    if (info.archive_class != TS_ARCHIVE_AUTOMATIC) { result = TS_ERR_ARGUMENT; goto done; }
    if (mkdir_p(target_dir) != 0) { result = TS_ERR_IO; goto done; }
    if (lstat(target, &(struct stat){0}) == 0) { result = TS_ERR_EXISTS; goto done; }
    if (rename(source, target) != 0) { result = TS_ERR_IO; goto done; }
    if (manifest_set_class(target, &manifest, target_class) != 0) {
        (void)rename(target, source);
        result = TS_ERR_IO;
        goto done;
    }
    result = TS_OK;
done:
    manifest_free(&manifest);
    release_lock(lock_fd);
    return result;
}

ts_result ts_compare_snapshots(const char *left_snapshot_dir, const char *right_snapshot_dir, ts_compare_result *out) {
    ts_manifest left, right;
    uint32_t i, j;
    ts_result result;
    if (!left_snapshot_dir || !right_snapshot_dir || !out) return TS_ERR_ARGUMENT;
    memset(&left, 0, sizeof(left)); memset(&right, 0, sizeof(right)); memset(out, 0, sizeof(*out));
    result = verify_snapshot_internal(left_snapshot_dir, NULL, &left);
    if (result != TS_OK) return result;
    result = verify_snapshot_internal(right_snapshot_dir, NULL, &right);
    if (result != TS_OK) { manifest_free(&left); return result; }
    /* Both manifests are sorted and unique, so comparison is linear in file count. */
    i = 0;
    j = 0;
    while (i < left.file_count && j < right.file_count) {
        int order = strcmp(left.files[i].path, right.files[j].path);
        if (order < 0) {
            ++out->only_in_left;
            ++i;
        } else if (order > 0) {
            ++out->only_in_right;
            ++j;
        } else {
            if (left.files[i].size == right.files[j].size &&
                !strcmp(left.files[i].sha256, right.files[j].sha256)) ++out->unchanged;
            else ++out->changed;
            ++i;
            ++j;
        }
    }
    out->only_in_left += left.file_count - i;
    out->only_in_right += right.file_count - j;
    manifest_free(&left); manifest_free(&right);
    return TS_OK;
}

static ts_result restore_locked(const char *source_dir, const char *storage_root,
                               ts_archive_class source_class, const char *snapshot_id,
                               ts_snapshot_result *safety_out) {
    char class_dir[32], snapshot_dir[TS_PATH_MAX], payload[TS_PATH_MAX];
    char stage[TS_PATH_MAX], old_source[TS_PATH_MAX], path[TS_PATH_MAX], destination[TS_PATH_MAX];
    ts_manifest manifest;
    ts_snapshot_info info;
    ts_snapshot_result safety;
    struct stat st;
    uint32_t i;
    ts_result result;
    memset(&manifest, 0, sizeof(manifest));
    if (!source_dir || !storage_root || !valid_id(snapshot_id) || !ts_archive_class_name(source_class) || !safety_out)
        return TS_ERR_ARGUMENT;
    if (class_directory(source_class, class_dir, sizeof(class_dir)) != 0 ||
        path_join(path, sizeof(path), storage_root, class_dir) != 0 ||
        path_join(snapshot_dir, sizeof(snapshot_dir), path, snapshot_id) != 0 ||
        path_join(payload, sizeof(payload), snapshot_dir, "payload") != 0) return TS_ERR_ARGUMENT;
    result = verify_snapshot_internal(snapshot_dir, &info, &manifest);
    if (result != TS_OK) return result;
    if (info.archive_class != source_class) { result = TS_ERR_MANIFEST; goto done; }
    result = create_snapshot_locked(source_dir, storage_root, TS_ARCHIVE_PRE_RESTORE, 0,
                                    "Safety snapshot created automatically before restore.", &safety);
    if (result != TS_OK) goto done;
    *safety_out = safety;
    if (snprintf(stage, sizeof(stage), "%s.rtse-stage-%s", source_dir, safety.snapshot_id) >= (int)sizeof(stage) ||
        snprintf(old_source, sizeof(old_source), "%s.rtse-rollback-%s", source_dir, safety.snapshot_id) >= (int)sizeof(old_source)) {
        result = TS_ERR_ARGUMENT; goto done;
    }
    if (lstat(stage, &st) == 0 || lstat(old_source, &st) == 0) { result = TS_ERR_EXISTS; goto done; }
    if (mkdir_p(stage) != 0) { result = TS_ERR_IO; goto done; }
    for (i = 0; i < manifest.file_count; ++i) {
        uint64_t copied_size = 0;
        char copied_hash[65];
        if (path_join(path, sizeof(path), payload, manifest.files[i].path) != 0 ||
            path_join(destination, sizeof(destination), stage, manifest.files[i].path) != 0 ||
            ensure_parent(destination) != 0 ||
            copy_file_hash(path, destination, manifest.files[i].size, copied_hash) != 0 ||
            hash_file(destination, &copied_size, copied_hash) != 0 || copied_size != manifest.files[i].size ||
            strcmp(copied_hash, manifest.files[i].sha256) != 0) {
            (void)remove_tree(stage);
            result = TS_ERR_INTEGRITY;
            goto done;
        }
    }
    if (rename(source_dir, old_source) != 0) {
        (void)remove_tree(stage);
        result = TS_ERR_TRANSACTION;
        goto done;
    }
    if (rename(stage, source_dir) != 0) {
        if (rename(old_source, source_dir) != 0) {
            (void)snprintf(safety_out->warning, sizeof(safety_out->warning),
                           "Restore rollback failed; original source remains at %.180s", old_source);
        }
        (void)remove_tree(stage);
        result = TS_ERR_TRANSACTION;
        goto done;
    }
    {
        int cleanup_failed = remove_tree(old_source) != 0;
        if (cleanup_failed) {
            (void)snprintf(safety_out->warning, sizeof(safety_out->warning),
                           "Restore succeeded; old source directory remains at %.160s", old_source);
        }
    }
    result = TS_OK;
done:
    manifest_free(&manifest);
    return result;
}

ts_result ts_restore_snapshot(const char *source_dir, const char *storage_root,
                              ts_archive_class source_class, const char *snapshot_id,
                              ts_snapshot_result *safety_snapshot_out) {
    char normalized_source[TS_PATH_MAX], normalized_root[TS_PATH_MAX];
    int lock_fd = -1;
    ts_result result;
    if (!safety_snapshot_out || normalize_directory_path(normalized_source, sizeof(normalized_source), source_dir) != 0 ||
        normalize_directory_path(normalized_root, sizeof(normalized_root), storage_root) != 0 ||
        paths_overlap(normalized_source, normalized_root)) return TS_ERR_ARGUMENT;
    memset(safety_snapshot_out, 0, sizeof(*safety_snapshot_out));
    if (acquire_lock(normalized_root, &lock_fd) != 0) return TS_ERR_TRANSACTION;
    result = restore_locked(normalized_source, normalized_root, source_class, snapshot_id, safety_snapshot_out);
    release_lock(lock_fd);
    return result;
}
