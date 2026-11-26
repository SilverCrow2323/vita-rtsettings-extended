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

## Settings Hub registration

The first feature navigation tree is now registered through `rtp_metadata_register_tai_saver()` and the shared Preferences UI model:

- **Protection:** automatic backup control, manual backup action and read-only last result.
- **Automatic Backup Policy:** retention settings.
- **Snapshot Archives:** Automatic, Known-Good, Failed / KO and Pre-Restore views.
- **Compare & Recovery:** compare, verify and restore actions.

Stable IDs, descriptions, icon keys, action kinds and backend-required/read-only flags are defined in `settings-ui/metadata/settings_metadata.c`. See [Settings Hub — Tai Saver](../docs/SETTINGS_HUB_TAI_SAVER.md).

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

The core is ready to be called by a boot service, but the boot entry point and stale-staging recovery are not implemented yet. The future service must avoid modifying `ur0:tai`, check storage before copying, avoid blocking boot for an unbounded period and apply retention only after a new snapshot has been fully verified.

## Current limitations

- Maximum 4096 regular files per snapshot; maximum path buffer 1023 bytes; maximum recursion depth 24.
- Symlinks and special files are rejected rather than followed; source and archive roots cannot overlap.
- The Settings Hub metadata/navigation tree is host-testable, but it is not yet a native Sony Settings extension.
- The boot service, live Vita filesystem integration, native Settings renderer/registration, installer transaction engine and VPK packaging remain pending.
- Host regression tests cover the portable core. A VitaSDK cross-build and physical-device validation are separate gates.

See [Settings Hub — Tai Saver](../docs/SETTINGS_HUB_TAI_SAVER.md) and [Tai Saver Core](../docs/TAI_SAVER_CORE.md).
