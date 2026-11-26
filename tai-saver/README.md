# Tai Saver

Tai Saver is the first feature module for Rt:Settings Extended. It preserves, inspects and recovers the user's ur0:tai configuration.

## Settings page

Planned navigation: **Settings → Rt:Settings Extended → Tai Saver**

The page is organized into four sections.

### Protection
- Enable or disable automatic backup at boot.
- Show the last backup result and timestamp.
- Run a backup now.
- Show source path and available storage information.

### Automatic backup policy
- Retention choices: 2, 3, 4, 5 or Custom.
- Custom retention accepts a validated positive integer.
- Display the number of current Automatic snapshots.
- Explain that retention affects Automatic snapshots only.

### Archive
- Automatic snapshots.
- Known-Good configurations.
- Failed / KO configurations.
- Snapshot details: date, file count, total size, verification status and notes.
- Mark an Automatic snapshot as Known-Good or Failed.
- Compare two snapshots by file path and hash.
- Explicitly delete an archive only after confirmation.

### Recovery
- Restore a selected verified snapshot.
- Always create a verified pre-restore safety snapshot first.
- Show the exact source archive and target path before confirmation.
- Report success or failure with actionable details.

## Archive retention policy

| Archive class | Created by | Automatic rotation |
|---|---|---|
| Automatic | Boot service or manual backup | Yes |
| Known-Good | Explicit user action | Never |
| Failed / KO | Explicit user action or diagnostic workflow | Never |

Moving an archive from Automatic to Known-Good or Failed removes it from the Automatic retention pool. The move preserves its manifest.

## Storage layout

Proposed root: ur0:data/RtSettingsExtended/TaiSaver/

- automatic/ — rolling boot snapshots.
- known-good/ — user-preserved working configurations.
- failed/ — user-preserved problematic configurations.
- staging/ — temporary incomplete backup and restore transactions.
- logs/ — concise operation logs.
- settings.json — Tai Saver preferences.

Snapshot directory names use a sortable timestamp plus a unique suffix. Integrity never relies on the directory name alone.

## Manifest

Each completed snapshot contains manifest.json with:
- schema_version
- snapshot_id
- created_at
- source_path
- archive_class
- completion_state
- file_count
- total_bytes
- files: relative_path, size_bytes and sha256 for each file

The manifest is validated before a snapshot is offered for restore. Hashes are calculated from file contents, not filenames or timestamps.

## Boot behavior

The boot task performs a snapshot only. It does not repair, rewrite, enable or disable taiHEN plugins and does not change config.txt.

The task must:
- avoid modifying ur0:tai;
- use a lock to prevent overlapping runs;
- detect insufficient storage before copying;
- leave a clear failure record if interrupted;
- avoid blocking boot for an unbounded period;
- apply retention only after a new snapshot has been fully verified.

## Classification rule

A Known-Good archive is not inferred from a successful copy. It means the user explicitly identified that configuration as working. Failed / KO is a diagnostic classification, not a claim that Tai Saver itself failed.
