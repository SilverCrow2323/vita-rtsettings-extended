#define _POSIX_C_SOURCE 200809L
#include "boot_journal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(void) {
    char temp[] = "/tmp/rtse-boot-journal-XXXXXX";
    char root[TS_PATH_MAX], event_path[TS_PATH_MAX];
    tbj_record record, previous;
    char *base = mkdtemp(temp);
    assert(base != NULL);
    assert(snprintf(root, sizeof(root), "%s/TaiSaver", base) < (int)sizeof(root));

    assert(ts_boot_journal_begin(root, "boot-001", "2026-11-26T10:00:00Z", NULL) == TS_OK);
    assert(ts_boot_journal_update(root, "boot-001", TBJ_STATE_WAITING_FOR_STORAGE,
                                  "2026-11-26T10:00:01Z", NULL, NULL) == TS_OK);
    assert(ts_boot_journal_mark_shell_ready(root, "boot-001", "2026-11-26T10:00:02Z") == TS_OK);
    assert(ts_boot_journal_update(root, "boot-001", TBJ_STATE_BACKUP_RUNNING,
                                  "2026-11-26T10:00:03Z", NULL, NULL) == TS_OK);
    assert(ts_boot_journal_update(root, "boot-001", TBJ_STATE_BACKUP_VERIFIED,
                                  "2026-11-26T10:00:04Z", "snapshot-001", NULL) == TS_OK);
    assert(ts_boot_journal_read(root, &record) == TS_OK);
    assert(record.state == TBJ_STATE_BACKUP_VERIFIED);
    assert(record.shell_ready == 1);
    assert(strcmp(record.snapshot_id, "snapshot-001") == 0);

    /* A completed previous run is archived unchanged when the next run begins. */
    assert(ts_boot_journal_begin(root, "boot-002", "2026-11-26T11:00:00Z", &previous) == TS_OK);
    assert(previous.state == TBJ_STATE_BACKUP_VERIFIED);
    assert(ts_boot_journal_read_event(root, "boot-001", &record) == TS_OK);
    assert(record.state == TBJ_STATE_BACKUP_VERIFIED);

    /* A power loss / forced process exit leaves a pending run; next boot marks it interrupted. */
    assert(ts_boot_journal_update(root, "boot-002", TBJ_STATE_WAITING_FOR_STORAGE,
                                  "2026-11-26T11:00:01Z", NULL, NULL) == TS_OK);
    assert(ts_boot_journal_begin(root, "boot-003", "2026-11-26T12:00:00Z", &previous) == TS_OK);
    assert(previous.state == TBJ_STATE_INTERRUPTED);
    assert(strcmp(previous.error, "previous-run-ended-before-terminal-state") == 0);
    assert(ts_boot_journal_read_event(root, "boot-002", &record) == TS_OK);
    assert(record.state == TBJ_STATE_INTERRUPTED);
    assert(ts_boot_journal_read(root, &record) == TS_OK);
    assert(strcmp(record.boot_id, "boot-003") == 0 && record.state == TBJ_STATE_STARTED);

    /* Invalid transitions and unsafe identifiers are rejected. */
    assert(ts_boot_journal_update(root, "boot-003", TBJ_STATE_BACKUP_VERIFIED,
                                  "2026-11-26T12:00:01Z", "snapshot-003", NULL) == TS_ERR_ARGUMENT);
    assert(ts_boot_journal_begin(root, "../escape", "2026-11-26T12:00:02Z", NULL) == TS_ERR_ARGUMENT);
    assert(ts_boot_journal_update(root, "boot-003", TBJ_STATE_FAILED_BACKUP,
                                  "2026-11-26T12:00:03Z", NULL, "copy-failed") == TS_OK);

    assert(snprintf(event_path, sizeof(event_path), "%s/boot/events/boot-002.ini", root) < (int)sizeof(event_path));
    assert(access(event_path, F_OK) == 0);
    printf("Boot Journal tests passed.\n");
    printf("Lifecycle transitions, shell-ready marker and interrupted-run recovery verified.\n");
    return 0;
}
