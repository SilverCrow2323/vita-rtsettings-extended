#define _POSIX_C_SOURCE 200809L
#include "boot_journal.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#if defined(RTSE_PLATFORM_VITA)
#include <psp2/io/fcntl.h>
#endif

#define TBJ_SCHEMA_VERSION "1"
#define TBJ_LINE_MAX (TS_PATH_MAX + 64)

static int safe_token(const char *s, size_t max, int allow_empty) {
    size_t i, n;
    if (!s) return 0;
    n = strlen(s);
    if ((!allow_empty && n == 0) || n >= max) return 0;
    for (i = 0; i < n; ++i) {
        unsigned char c = (unsigned char)s[i];
        if (c < 0x21 || c > 0x7e || c == '=' || c == '|') return 0;
    }
    return 1;
}

static int valid_boot_id(const char *id) {
    size_t i;
    if (!safe_token(id, TBJ_BOOT_ID_MAX, 0) || strstr(id, "..")) return 0;
    for (i = 0; id[i]; ++i) {
        char c = id[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '-' || c == '_')) return 0;
    }
    return 1;
}

const char *ts_boot_state_name(tbj_state state) {
    switch (state) {
        case TBJ_STATE_STARTED: return "STARTED";
        case TBJ_STATE_WAITING_FOR_STORAGE: return "WAITING_FOR_STORAGE";
        case TBJ_STATE_BACKUP_RUNNING: return "BACKUP_RUNNING";
        case TBJ_STATE_BACKUP_VERIFIED: return "BACKUP_VERIFIED";
        case TBJ_STATE_SKIPPED_DISABLED: return "SKIPPED_DISABLED";
        case TBJ_STATE_FAILED_STORAGE_TIMEOUT: return "FAILED_STORAGE_TIMEOUT";
        case TBJ_STATE_FAILED_BACKUP: return "FAILED_BACKUP";
        case TBJ_STATE_INTERRUPTED: return "INTERRUPTED";
        default: return NULL;
    }
}

static int parse_state(const char *s, tbj_state *out) {
    int i;
    for (i = (int)TBJ_STATE_STARTED; i <= (int)TBJ_STATE_INTERRUPTED; ++i) {
        if (strcmp(s, ts_boot_state_name((tbj_state)i)) == 0) {
            *out = (tbj_state)i;
            return 0;
        }
    }
    return -1;
}

static int state_is_pending(tbj_state state) {
    return state == TBJ_STATE_STARTED || state == TBJ_STATE_WAITING_FOR_STORAGE ||
           state == TBJ_STATE_BACKUP_RUNNING;
}

static int transition_allowed(tbj_state from, tbj_state to) {
    if (from == TBJ_STATE_STARTED)
        return to == TBJ_STATE_WAITING_FOR_STORAGE || to == TBJ_STATE_BACKUP_RUNNING ||
               to == TBJ_STATE_SKIPPED_DISABLED ||
               to == TBJ_STATE_FAILED_STORAGE_TIMEOUT || to == TBJ_STATE_FAILED_BACKUP;
    if (from == TBJ_STATE_WAITING_FOR_STORAGE)
        return to == TBJ_STATE_BACKUP_RUNNING || to == TBJ_STATE_FAILED_STORAGE_TIMEOUT ||
               to == TBJ_STATE_FAILED_BACKUP;
    if (from == TBJ_STATE_BACKUP_RUNNING)
        return to == TBJ_STATE_BACKUP_VERIFIED || to == TBJ_STATE_FAILED_BACKUP;
    return 0;
}

static int join_path(char *out, size_t size, const char *a, const char *b) {
    size_t n = strlen(a), m = strlen(b);
    int slash = n && a[n - 1] != '/';
    if (n + (size_t)slash + m + 1 > size) return -1;
    memcpy(out, a, n);
    if (slash) out[n++] = '/';
    memcpy(out + n, b, m + 1);
    return 0;
}

static int mkdir_tree(const char *path) {
    char tmp[TS_PATH_MAX];
    size_t i, n;
    struct stat st;
    if (!path || !*path || strlen(path) >= sizeof(tmp)) return -1;
    strcpy(tmp, path);
    n = strlen(tmp);
    for (i = 1; i < n; ++i) {
        if (tmp[i] == '/') {
            char saved = tmp[i];
            tmp[i] = '\0';
            if (*tmp && mkdir(tmp, 0700) != 0 && errno != EEXIST) return -1;
            if (*tmp && (lstat(tmp, &st) != 0 || !S_ISDIR(st.st_mode))) return -1;
            tmp[i] = saved;
        }
    }
    if (mkdir(tmp, 0700) != 0 && errno != EEXIST) return -1;
    return lstat(tmp, &st) == 0 && S_ISDIR(st.st_mode) ? 0 : -1;
}

static int journal_paths(const char *root, char *boot_dir, char *state_path, char *events_dir) {
    char name[TS_PATH_MAX];
    if (!root || !*root || strlen(root) >= TS_PATH_MAX) return -1;
    if (join_path(boot_dir, TS_PATH_MAX, root, "boot") != 0 ||
        join_path(state_path, TS_PATH_MAX, boot_dir, "boot-state.ini") != 0 ||
        join_path(events_dir, TS_PATH_MAX, boot_dir, "events") != 0) return -1;
    (void)name;
    return 0;
}

static int sync_file(FILE *file) {
    if (fflush(file) != 0) return -1;
#if defined(RTSE_PLATFORM_VITA)
    return sceIoSyncByFd(fileno(file), 0);
#else
    return fsync(fileno(file));
#endif
}

static ts_result write_record(const char *path, const tbj_record *record) {
    char temp[TS_PATH_MAX];
    FILE *file;
    int ok = 1;
    if (snprintf(temp, sizeof(temp), "%s.tmp", path) >= (int)sizeof(temp)) return TS_ERR_ARGUMENT;
    file = fopen(temp, "wb");
    if (!file) return TS_ERR_IO;
    if (fprintf(file,
        "schema=%s\nboot_id=%s\nstate=%s\nstarted_at=%s\nupdated_at=%s\nshell_ready=%d\nsnapshot_id=%s\nerror=%s\n",
        TBJ_SCHEMA_VERSION, record->boot_id, ts_boot_state_name(record->state),
        record->started_at, record->updated_at, record->shell_ready ? 1 : 0,
        record->snapshot_id, record->error) < 0) ok = 0;
    if (ok && sync_file(file) != 0) ok = 0;
    if (fclose(file) != 0) ok = 0;
    if (!ok) {
        (void)unlink(temp);
        return TS_ERR_IO;
    }
    if (rename(temp, path) != 0) {
        (void)unlink(temp);
        return TS_ERR_IO;
    }
    return TS_OK;
}

static ts_result read_record_path(const char *path, tbj_record *out) {
    FILE *file;
    char line[TBJ_LINE_MAX];
    unsigned seen = 0;
    tbj_record record;
    memset(&record, 0, sizeof(record));
    file = fopen(path, "rb");
    if (!file) return errno == ENOENT ? TS_ERR_NOT_FOUND : TS_ERR_IO;
    while (fgets(line, sizeof(line), file)) {
        char *eq = strchr(line, '='), *value, *end;
        if (!eq) { fclose(file); return TS_ERR_MANIFEST; }
        *eq = '\0';
        value = eq + 1;
        end = strpbrk(value, "\r\n");
        if (end) *end = '\0';
        if (strcmp(line, "schema") == 0) {
            if ((seen & 1U) || strcmp(value, TBJ_SCHEMA_VERSION)) { fclose(file); return TS_ERR_MANIFEST; }
            seen |= 1U;
        } else if (strcmp(line, "boot_id") == 0) {
            if ((seen & 2U) || !valid_boot_id(value)) { fclose(file); return TS_ERR_MANIFEST; }
            strcpy(record.boot_id, value); seen |= 2U;
        } else if (strcmp(line, "state") == 0) {
            if ((seen & 4U) || parse_state(value, &record.state) != 0) { fclose(file); return TS_ERR_MANIFEST; }
            seen |= 4U;
        } else if (strcmp(line, "started_at") == 0) {
            if ((seen & 8U) || !safe_token(value, sizeof(record.started_at), 0)) { fclose(file); return TS_ERR_MANIFEST; }
            strcpy(record.started_at, value); seen |= 8U;
        } else if (strcmp(line, "updated_at") == 0) {
            if ((seen & 16U) || !safe_token(value, sizeof(record.updated_at), 0)) { fclose(file); return TS_ERR_MANIFEST; }
            strcpy(record.updated_at, value); seen |= 16U;
        } else if (strcmp(line, "shell_ready") == 0) {
            if ((seen & 32U) || (strcmp(value, "0") && strcmp(value, "1"))) { fclose(file); return TS_ERR_MANIFEST; }
            record.shell_ready = value[0] == '1'; seen |= 32U;
        } else if (strcmp(line, "snapshot_id") == 0) {
            if ((seen & 64U) || !safe_token(value, sizeof(record.snapshot_id), 1)) { fclose(file); return TS_ERR_MANIFEST; }
            strcpy(record.snapshot_id, value); seen |= 64U;
        } else if (strcmp(line, "error") == 0) {
            if ((seen & 128U) || !safe_token(value, sizeof(record.error), 1)) { fclose(file); return TS_ERR_MANIFEST; }
            strcpy(record.error, value); seen |= 128U;
        } else {
            fclose(file); return TS_ERR_MANIFEST;
        }
    }
    {
        int had_error = ferror(file);
        int close_error = fclose(file);
        if (had_error || close_error != 0) return TS_ERR_IO;
    }
    if (seen != 255U) return TS_ERR_MANIFEST;
    if (record.state == TBJ_STATE_BACKUP_VERIFIED && !record.snapshot_id[0]) return TS_ERR_MANIFEST;
    *out = record;
    return TS_OK;
}

static ts_result prepare_dirs(const char *root, char *boot_dir, char *state_path, char *events_dir) {
    if (journal_paths(root, boot_dir, state_path, events_dir) != 0) return TS_ERR_ARGUMENT;
    if (mkdir_tree(root) != 0 || mkdir_tree(boot_dir) != 0 || mkdir_tree(events_dir) != 0) return TS_ERR_IO;
    return TS_OK;
}

ts_result ts_boot_journal_read(const char *storage_root, tbj_record *out) {
    char boot_dir[TS_PATH_MAX], state_path[TS_PATH_MAX], events_dir[TS_PATH_MAX];
    ts_result result;
    if (!out) return TS_ERR_ARGUMENT;
    result = prepare_dirs(storage_root, boot_dir, state_path, events_dir);
    if (result != TS_OK) return result;
    return read_record_path(state_path, out);
}

ts_result ts_boot_journal_read_event(const char *storage_root, const char *boot_id, tbj_record *out) {
    char boot_dir[TS_PATH_MAX], state_path[TS_PATH_MAX], events_dir[TS_PATH_MAX], filename[TS_PATH_MAX], path[TS_PATH_MAX];
    ts_result result;
    if (!out || !valid_boot_id(boot_id)) return TS_ERR_ARGUMENT;
    result = prepare_dirs(storage_root, boot_dir, state_path, events_dir);
    if (result != TS_OK) return result;
    if (snprintf(filename, sizeof(filename), "%s.ini", boot_id) >= (int)sizeof(filename) ||
        join_path(path, sizeof(path), events_dir, filename) != 0) return TS_ERR_ARGUMENT;
    return read_record_path(path, out);
}

ts_result ts_boot_journal_begin(const char *storage_root, const char *boot_id,
                                const char *timestamp, tbj_record *previous_out) {
    char boot_dir[TS_PATH_MAX], state_path[TS_PATH_MAX], events_dir[TS_PATH_MAX], event_path[TS_PATH_MAX], filename[TS_PATH_MAX];
    tbj_record previous, current;
    ts_result result;
    if (!valid_boot_id(boot_id) || !safe_token(timestamp, TBJ_TIMESTAMP_MAX, 0)) return TS_ERR_ARGUMENT;
    result = prepare_dirs(storage_root, boot_dir, state_path, events_dir);
    if (result != TS_OK) return result;
    result = read_record_path(state_path, &previous);
    if (result == TS_OK) {
        if (strcmp(previous.boot_id, boot_id) == 0) return TS_ERR_EXISTS;
        if (state_is_pending(previous.state)) {
            previous.state = TBJ_STATE_INTERRUPTED;
            strcpy(previous.error, "previous-run-ended-before-terminal-state");
            strcpy(previous.updated_at, timestamp);
        }
        if (snprintf(filename, sizeof(filename), "%s.ini", previous.boot_id) >= (int)sizeof(filename) ||
            join_path(event_path, sizeof(event_path), events_dir, filename) != 0) return TS_ERR_ARGUMENT;
        if (access(event_path, F_OK) == 0) {
            tbj_record archived;
            result = read_record_path(event_path, &archived);
            if (result != TS_OK || strcmp(archived.boot_id, previous.boot_id) != 0 ||
                archived.state != previous.state) return TS_ERR_EXISTS;
        } else {
            result = write_record(event_path, &previous);
            if (result != TS_OK) return result;
        }
        if (previous_out) *previous_out = previous;
    } else if (result != TS_ERR_NOT_FOUND) {
        return result;
    } else if (previous_out) {
        memset(previous_out, 0, sizeof(*previous_out));
    }
    memset(&current, 0, sizeof(current));
    strcpy(current.boot_id, boot_id);
    current.state = TBJ_STATE_STARTED;
    strcpy(current.started_at, timestamp);
    strcpy(current.updated_at, timestamp);
    result = write_record(state_path, &current);
    return result;
}

ts_result ts_boot_journal_update(const char *storage_root, const char *boot_id,
                                 tbj_state next_state, const char *timestamp,
                                 const char *snapshot_id, const char *error) {
    char boot_dir[TS_PATH_MAX], state_path[TS_PATH_MAX], events_dir[TS_PATH_MAX];
    tbj_record record;
    ts_result result;
    if (!valid_boot_id(boot_id) || !safe_token(timestamp, TBJ_TIMESTAMP_MAX, 0) ||
        !ts_boot_state_name(next_state) || !safe_token(snapshot_id ? snapshot_id : "", sizeof(record.snapshot_id), 1) ||
        (snapshot_id && *snapshot_id && !valid_boot_id(snapshot_id)) ||
        !safe_token(error ? error : "", sizeof(record.error), 1)) return TS_ERR_ARGUMENT;
    if (next_state == TBJ_STATE_BACKUP_VERIFIED && (!snapshot_id || !*snapshot_id)) return TS_ERR_ARGUMENT;
    if ((next_state == TBJ_STATE_FAILED_BACKUP || next_state == TBJ_STATE_FAILED_STORAGE_TIMEOUT) &&
        (!error || !*error)) return TS_ERR_ARGUMENT;
    result = prepare_dirs(storage_root, boot_dir, state_path, events_dir);
    if (result != TS_OK) return result;
    result = read_record_path(state_path, &record);
    if (result != TS_OK) return result;
    if (strcmp(record.boot_id, boot_id) != 0) return TS_ERR_NOT_FOUND;
    if (!transition_allowed(record.state, next_state)) return TS_ERR_ARGUMENT;
    record.state = next_state;
    strcpy(record.updated_at, timestamp);
    if (snapshot_id && *snapshot_id) strcpy(record.snapshot_id, snapshot_id);
    if (error && *error) strcpy(record.error, error);
    return write_record(state_path, &record);
}

ts_result ts_boot_journal_mark_shell_ready(const char *storage_root, const char *boot_id,
                                           const char *timestamp) {
    char boot_dir[TS_PATH_MAX], state_path[TS_PATH_MAX], events_dir[TS_PATH_MAX];
    tbj_record record;
    ts_result result;
    if (!valid_boot_id(boot_id) || !safe_token(timestamp, TBJ_TIMESTAMP_MAX, 0)) return TS_ERR_ARGUMENT;
    result = prepare_dirs(storage_root, boot_dir, state_path, events_dir);
    if (result != TS_OK) return result;
    result = read_record_path(state_path, &record);
    if (result != TS_OK) return result;
    if (strcmp(record.boot_id, boot_id) != 0) return TS_ERR_NOT_FOUND;
    record.shell_ready = 1;
    strcpy(record.updated_at, timestamp);
    return write_record(state_path, &record);
}
