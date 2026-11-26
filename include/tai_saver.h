#ifndef RTSE_TAI_SAVER_H
#define RTSE_TAI_SAVER_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TS_ID_MAX 96
#define TS_PATH_MAX 1024
#define TS_MESSAGE_MAX 256

typedef enum {
    TS_ARCHIVE_AUTOMATIC = 0,
    TS_ARCHIVE_KNOWN_GOOD,
    TS_ARCHIVE_FAILED,
    TS_ARCHIVE_PRE_RESTORE
} ts_archive_class;

typedef enum {
    TS_OK = 0,
    TS_ERR_ARGUMENT = -1,
    TS_ERR_IO = -2,
    TS_ERR_MEMORY = -3,
    TS_ERR_UNSUPPORTED_FILE = -4,
    TS_ERR_SOURCE_CHANGED = -5,
    TS_ERR_NO_SPACE = -6,
    TS_ERR_MANIFEST = -7,
    TS_ERR_INTEGRITY = -8,
    TS_ERR_NOT_FOUND = -9,
    TS_ERR_EXISTS = -10,
    TS_ERR_RETENTION = -11,
    TS_ERR_TRANSACTION = -12,
    TS_ERR_LIMIT = -13
} ts_result;

typedef struct {
    char snapshot_id[TS_ID_MAX];
    char snapshot_path[TS_PATH_MAX];
    uint32_t file_count;
    uint64_t total_bytes;
    uint32_t retention_removed;
    char warning[TS_MESSAGE_MAX];
} ts_snapshot_result;

typedef struct {
    ts_archive_class archive_class;
    uint32_t file_count;
    uint64_t total_bytes;
    char snapshot_id[TS_ID_MAX];
    char created_at[32];
} ts_snapshot_info;

typedef struct {
    uint32_t only_in_left;
    uint32_t only_in_right;
    uint32_t changed;
    uint32_t unchanged;
} ts_compare_result;

/* Creates a fully copied and verified snapshot. Only Automatic snapshots are rotated. */
ts_result ts_create_snapshot(const char *source_dir,
                             const char *storage_root,
                             ts_archive_class archive_class,
                             unsigned automatic_retention,
                             const char *notes,
                             ts_snapshot_result *out);

/* Verifies manifest structure, every listed file, and rejects unlisted regular files. */
ts_result ts_verify_snapshot(const char *snapshot_dir, ts_snapshot_info *out);

/* Moves an Automatic snapshot into a permanent Known-Good or Failed archive. */
ts_result ts_classify_snapshot(const char *storage_root,
                               const char *snapshot_id,
                               ts_archive_class target_class);

/* Verifies both snapshots, then compares their path/hash manifests. */
ts_result ts_compare_snapshots(const char *left_snapshot_dir,
                               const char *right_snapshot_dir,
                               ts_compare_result *out);

/* Restores a verified snapshot and first stores the live source as a Pre-Restore snapshot. */
ts_result ts_restore_snapshot(const char *source_dir,
                              const char *storage_root,
                              ts_archive_class source_class,
                              const char *snapshot_id,
                              ts_snapshot_result *safety_snapshot_out);

/* Prunes only verified Automatic snapshots. Known-Good/Failed/Pre-Restore are untouched. */
ts_result ts_apply_retention(const char *storage_root,
                             unsigned keep_count,
                             uint32_t *removed_count);

const char *ts_archive_class_name(ts_archive_class archive_class);
const char *ts_result_name(ts_result result);

#ifdef __cplusplus
}
#endif
#endif
