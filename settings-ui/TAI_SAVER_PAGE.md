# Tai Saver — Settings page specification

## Navigation

Settings
└── Rt:Settings Extended
    └── Tai Saver

The category must appear as a normal Settings entry and open a dedicated Tai Saver page. It must not launch an external standalone dashboard.

## Page layout

### Protection
| Control | Type | Behavior |
|---|---|---|
| Automatic backup at boot | Toggle | Enables/disables future boot snapshots. Does not delete existing archives. |
| Back up now | Action | Starts a manual snapshot and displays live progress/result. |
| Last backup | Read-only status | Shows timestamp, result and verification state. |
| Storage status | Read-only status | Shows free space and Tai Saver storage usage. |

### Automatic backup
| Control | Type | Behavior |
|---|---|---|
| Keep automatic backups | Choice | 2, 3, 4, 5 or Custom. |
| Custom count | Numeric input | Appears only for Custom; minimum 2. |
| Automatic archive count | Read-only status | Shows current count and configured limit. |

A short explanation must remain visible: “This limit applies only to Automatic backups. Known-Good and Failed archives are kept separately.”

### Archive manager
Three distinct archive lists:
1. Automatic
2. Known-Good
3. Failed / KO

Each item displays date/time, file count, size and verification status. Selecting an item opens its detail page with:
- file list and hashes;
- manifest/integrity result;
- notes and archive classification;
- Compare;
- Mark Known-Good or Mark Failed (where applicable);
- Restore;
- Delete, with a separate confirmation step.

Known-Good and Failed / KO lists must never be affected by automatic retention.

### Recovery
Restore is available only for a completed, verified snapshot. Before the final confirmation, show:
- selected archive and timestamp;
- archive class;
- verification result;
- destination (ur0:tai);
- statement that the current configuration will first be saved as a separate safety snapshot.

If the pre-restore safety snapshot cannot be completed and verified, abort restore.

## Status language

Use explicit states:
- Ready
- Backup in progress
- Backup completed — verified
- Backup completed — verification failed
- Backup failed
- Restore in progress
- Restore completed
- Restore failed
- Insufficient storage
- Archive incomplete — not restorable

Never show a green success state for a partial copy.

## Interaction rules

- Long operations expose progress and a cancel option where cancellation is safe.
- A cancelled or interrupted operation is marked incomplete.
- Destructive actions require a confirmation screen naming the exact archive.
- Errors include the operation stage and a useful reason; do not expose only a generic “Failed”.
