#ifndef RTSE_BOOT_JOURNAL_H
#define RTSE_BOOT_JOURNAL_H

#include "tai_saver.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TBJ_BOOT_ID_MAX TS_ID_MAX
#define TBJ_TIMESTAMP_MAX 32

typedef enum {
    TBJ_STATE_STARTED = 0,
    TBJ_STATE_WAITING_FOR_STORAGE,
    TBJ_STATE_BACKUP_RUNNING,
    TBJ_STATE_BACKUP_VERIFIED,
    TBJ_STATE_SKIPPED_DISABLED,
    TBJ_STATE_FAILED_STORAGE_TIMEOUT,
    TBJ_STATE_FAILED_BACKUP,
    TBJ_STATE_INTERRUPTED
} tbj_state;

typedef struct {
    char boot_id[TBJ_BOOT_ID_MAX];
    tbj_state state;
    char started_at[TBJ_TIMESTAMP_MAX];
    char updated_at[TBJ_TIMESTAMP_MAX];
    int shell_ready;
    char snapshot_id[TBJ_BOOT_ID_MAX];
    char error[TS_MESSAGE_MAX];
} tbj_record;

/* Starts a new run. A prior non-terminal run is archived as INTERRUPTED. */
ts_result ts_boot_journal_begin(const char *storage_root,
                                const char *boot_id,
                                const char *timestamp,
                                tbj_record *previous_out);

/* Reads the current run or one archived event by boot ID. */
ts_result ts_boot_journal_read(const char *storage_root, tbj_record *out);
ts_result ts_boot_journal_read_event(const char *storage_root,
                                     const char *boot_id,
                                     tbj_record *out);

/* Advances the backup state using the allowed transition graph. */
ts_result ts_boot_journal_update(const char *storage_root,
                                 const char *boot_id,
                                 tbj_state next_state,
                                 const char *timestamp,
                                 const char *snapshot_id,
                                 const char *error);

/* Records SceShell readiness independently from the backup result. */
ts_result ts_boot_journal_mark_shell_ready(const char *storage_root,
                                           const char *boot_id,
                                           const char *timestamp);

const char *ts_boot_state_name(tbj_state state);

#ifdef __cplusplus
}
#endif
#endif
