# Tai Saver

Tai Saver is the first feature module for Rt:Settings Extended. It preserves, inspects and recovers the user's `ur0:tai` configuration.

## Implemented core

The C service is exposed by `include/tai_saver.h` and implemented in `source/tai-saver/`.

| Operation | API | Current behavior |
|---|---|---|
| Create snapshot | `ts_create_snapshot` | Recursively copies regular files into staging, generates SHA-256 entries, writes `manifest.json`, verifies the payload and promotes the completed archive. |
| Verify | `ts_verify_snapshot` | Checks the manifest, exact file set, file sizes and SHA-256 hashes. |
| Classify | `ts_classify_snapshot` | Moves an Automatic snapshot to Known-Good or Failed and updates its manifest. |
| Compare | `ts_compare_snapshots` | Reports files only in the left/right snapshot, changed files and unchanged files. |
| Restore | `ts_restore_snapshot` | Verifies the selected archive, snapshots the live source to Pre-Restore, stages the replacement and uses a rollback rename if installation fails. |
| Rotate | `ts_apply_retention` | Removes only old, verified Automatic snapshots above the configured count. |

## Settings page

Planned navigation: **Settings → Rt:Settings Extended → Tai Saver**

The native Settings page will expose four groups.

### Protection
- Enable or disable automatic backup at boot.
- Show the last backup result and timestamp.
- Run a backup now.
- Show source path and available storage information.

### Automatic backup policy
- Retention choices: 2, 3, 4, 5 or Custom.
- Custom retention accepts a validated integer from 2 to 1000.
- Display the number of current Automatic snapshots.
- Explain that retention affects Automatic snapshots only.

### Archive
- Automatic snapshots.
- Known-Good configurations.
- Failed / KO configurations.
- Pre-Restore safety snapshots.
- Snapshot details: date, file count, total size, verification status and notes.
- Compare two snapshots by file path and hash.
- Mark an Automatic snapshot as Known-Good or Failed.
- Explicitly delete an archive only after confirmation.

### Recovery
- Restore a selected verified snapshot.
- Always create a verified Pre-Restore safety snapshot first.
- Show the exact source archive and target path before confirmation.
- Report success or failure with actionable details.

## Archive retention policy

| Archive class | Created by | Automatic rotation |
|---|---|---|
| Automatic | Boot service or manual backup | Yes |
| Known-Good | Explicit user action | Never |
| Failed / KO | Explicit user action or diagnostic workflow | Never |
| Pre-Restore | Automatically before restore | Never |

Moving an archive from Automatic to Known-Good or Failed removes it from the Automatic retention pool. The move preserves its manifest.

## Storage layout

Default root: `ur0:data/RtSettingsExtended/TaiSaver/`

- `automatic/` — rolling boot/manual snapshots.
- `known-good/` — user-preserved working configurations.
- `failed/` — user-preserved problematic configurations.
- `pre-restore/` — safety snapshots made before restoring another archive.
- `staging/` — temporary snapshots before verification and promotion.
- `.tai-saver.lock` — transaction lock handle target.

## Manifest

Each completed snapshot contains `manifest.json` with:

- `schema_version`, `snapshot_id`, `created_at`, `source_path`;
- `archive_class` and `completion_state`;
- `file_count` and `total_bytes`;
- `files[]`: `relative_path`, `size_bytes` and `sha256`;
- optional notes.

Hash values are calculated from file contents, not filenames or timestamps. A manifest is accepted for restore only when every file is present, the size matches and the SHA-256 digest matches.

## Boot behavior

The boot task will perform a snapshot only. It must not repair, rewrite, enable or disable taiHEN plugins and must not change `config.txt`.

The core is ready to be called by a boot service, but the boot entry point and stale-staging recovery are not implemented yet. The future service must:

- avoid modifying `ur0:tai`;
- invoke the core under its transaction lock;
- check storage before copying;
- record failure without presenting a partial snapshot as valid;
- avoid blocking boot for an unbounded period;
- apply retention only after a new snapshot has been fully verified.

## Classification rule

A Known-Good archive is not inferred from a successful copy. It means the user explicitly identified that configuration as working. Failed / KO is a diagnostic classification, not a claim that Tai Saver itself failed.

## Current limitations

- Maximum 4096 regular files per snapshot; maximum path buffer 1023 bytes; maximum recursion depth 24.
- Symlinks and special files are rejected rather than followed; source and archive roots cannot overlap.
- Manifests must match the directory snapshot ID, contain unique sorted relative paths and pass complete hash verification.
- Host regression tests pass. A VitaSDK cross-build and physical-device validation are still pending.
- Progress reporting, native Settings registration, VPK packaging and interrupted-staging quarantine are next implementation work.

See [`docs/TAI_SAVER_CORE.md`](../docs/TAI_SAVER_CORE.md) for implementation details and tests.
