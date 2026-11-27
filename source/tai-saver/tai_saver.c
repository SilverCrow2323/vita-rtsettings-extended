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

static ts_result apply_retention_locked(const char *storage_root, unsigned keep_count, uint32_t *removed_count) {
    typedef struct { char id[TS_ID_MAX]; } snapshot_name;
    char automatic_dir[TS_PATH_MAX], path[TS_PATH_MAX];
    rtfs_dir dir;
    rtfs_dirent entry;
    snapshot_name *items = NULL;
    size_t count = 0, capacity = 0, i;
    uint32_t removed = 0;
    ts_result result = TS_OK;
    int read_result, close_result, dir_open = 0;
    if (removed_count) *removed_count = 0;
    if (!valid_directory_path(storage_root) || keep_count < 2 || keep_count > 1000) return TS_ERR_ARGUMENT;
    if (path_join(automatic_dir, sizeof(automatic_dir), storage_root, "automatic") != 0 || mkdir_p(automatic_dir) != 0)
        return TS_ERR_IO;
    if (rtfs_dir_open(&dir, automatic_dir) != 0) return TS_ERR_IO;
    dir_open = 1;
    while ((read_result = rtfs_dir_read(&dir, &entry)) > 0) {
        ts_snapshot_info info;
        if (!strcmp(entry.name, ".") || !strcmp(entry.name, "..") || !valid_id(entry.name)) continue;
        if (path_join(path, sizeof(path), automatic_dir, entry.name) != 0) continue;
        if (ts_verify_snapshot(path, &info) != TS_OK || info.archive_class != TS_ARCHIVE_AUTOMATIC) continue;
        if (count == capacity) {
            size_t next_capacity = capacity ? capacity * 2 : 8;
            snapshot_name *next = (snapshot_name *)realloc(items, next_capacity * sizeof(snapshot_name));
            if (!next) { result = TS_ERR_MEMORY; goto done; }
            items = next; capacity = next_capacity;
        }
        memcpy(items[count].id, entry.name, strlen(entry.name) + 1U);
        ++count;
    }
    close_result = rtfs_dir_close(&dir);
    dir_open = 0;
    if (read_result < 0 || close_result != 0) { result = TS_ERR_IO; goto done; }
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
    if (dir_open) (void)rtfs_dir_close(&dir);
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
    if (rtfs_stat_path(target, &st) == 0) { result = TS_ERR_EXISTS; goto done; }
    if (rtfs_rename(source, target) != 0) { result = TS_ERR_IO; goto done; }
    if (manifest_set_class(target, &manifest, target_class) != 0) {
        (void)rtfs_rename(target, source);
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
    rtfs_stat st;
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
    if (rtfs_stat_path(stage, &st) == 0 || rtfs_stat_path(old_source, &st) == 0) { result = TS_ERR_EXISTS; goto done; }
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
    if (rtfs_rename(source_dir, old_source) != 0) {
        (void)remove_tree(stage);
        result = TS_ERR_TRANSACTION;
        goto done;
    }
    if (rtfs_rename(stage, source_dir) != 0) {
        if (rtfs_rename(old_source, source_dir) != 0) {
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
