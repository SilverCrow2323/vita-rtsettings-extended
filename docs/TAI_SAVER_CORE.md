# Tai Saver Core — implementation notes

## Scope of this milestone

This milestone turns the Tai Saver design into a callable C subsystem. It is deliberately independent of the Settings page and the boot entry point: both will call the same service API rather than duplicate backup logic.

Implemented in this milestone:

- deterministic recursive enumeration of regular files;
- complete file copies into a staging directory;
- SHA-256 for every copied file;
- JSON manifest creation and strict manifest checks;
- verification of file count, paths, sizes and hashes;
- archive comparison by relative path and hash;
- classification of Automatic snapshots as Known-Good or Failed;
- Automatic-only retention;
- pre-restore safety snapshot and staged directory replacement with rollback on rename failure;
- operation locking, free-space preflight and Vita-specific I/O hooks;
- path-overlap and traversal rejection for source/storage roots;
- manifest identity checks against the snapshot directory name;
- linear-time comparison of the two sorted manifests.

Not included yet: the taiHEN boot plugin entry point, Settings category registration, the native Settings page, progress callbacks, automatic stale-staging recovery, VPK packaging and on-device validation.

## Storage layout

Default root: \`ur0:data/RtSettingsExtended/TaiSaver/\`

\`\`\`text
TaiSaver/
├── .tai-saver.lock
├── automatic/
│   └── <snapshot-id>/
│       ├── manifest.json
│       └── payload/
├── known-good/
├── failed/
├── pre-restore/
└── staging/
\`\`\`

A snapshot is first assembled under \`staging/<snapshot-id>\`. It is promoted with a directory rename only after the payload and manifest have passed verification. A normal failure removes the current staging directory. A hard interruption can leave an orphan under \`staging\`; boot-time orphan recovery and diagnostic quarantine are a follow-up item and must run before the Settings UI offers those entries as archives.

Snapshot IDs use a UTC timestamp, process identifier and process-local sequence. The manifest, not the directory name, is the source of truth.

## Public API

Header: \`include/tai_saver.h\`

| Function | Responsibility |
|---|---|
| \`ts_create_snapshot\` | Copy and verify a source directory into an archive class. Automatic snapshots invoke Automatic-only retention. |
| \`ts_verify_snapshot\` | Validate the manifest and re-hash every payload file. Rejects extra, missing or changed regular files. |
| \`ts_classify_snapshot\` | Move an Automatic snapshot to Known-Good or Failed and update its manifest. |
| \`ts_compare_snapshots\` | Verify both inputs and count added, removed, changed and unchanged files. |
| \`ts_restore_snapshot\` | Verify the selected archive, create a Pre-Restore safety snapshot, stage the replacement, then swap directories with rollback on failure. |
| \`ts_apply_retention\` | Retain the newest configured number of verified Automatic snapshots. |

\`ts_create_snapshot\` expects \`automatic_retention >= 2\` for the Automatic class. The supported settings range is 2–1000. Other archive classes are not subject to that setting.

## Safety rules enforced by the core

1. Backup is read-only against the source directory. The live \`ur0:tai\` tree is not rewritten during snapshot creation.
2. Only regular files are copied. Symbolic links, sockets, devices and other special entries are rejected rather than followed.
3. Source/storage roots cannot overlap; \`.\` and \`..\` path components are rejected, and relative file paths cannot escape the payload root.
4. A source file that changes while it is copied causes the operation to fail; no partial snapshot is promoted.
5. A snapshot is promoted only after its manifest has been written and every copied file has been re-hashed.
6. Restore is refused unless the selected archive passes verification and the live configuration has first been saved as a Pre-Restore snapshot.
7. Restore writes into a separate staging directory. The old live directory is renamed aside before the new directory is installed; a failed second rename triggers rollback.
8. The Automatic retention routine only considers directories with a valid, complete manifest whose class is \`automatic\`. Unknown, incomplete or unverifiable directories are left untouched.
9. Known-Good, Failed and Pre-Restore directories are never candidates for Automatic rotation.
10. A successful file copy is not itself a Known-Good classification. Classification is always an explicit operation.

## Limits and failure behavior

- Maximum tracked regular files per snapshot: 4096.
- Maximum relative path / path buffer: 1023 bytes.
- Maximum manifest input: 8 MiB.
- Maximum directory recursion depth: 24.
- Free space is checked before copying. The estimate includes a 5% margin, per-file metadata overhead and a 64 KiB reserve.
- A failure to acquire the transaction lock returns \`TS_ERR_TRANSACTION\`; callers must not retry in a tight loop.
- A copy-stage failure returns an error and removes the current staging directory. The live source remains unchanged.
- A retention failure does not invalidate a newly promoted snapshot. \`ts_snapshot_result.warning\` reports the cleanup problem.
- A failed cleanup after a successful restore is reported as a warning; the Pre-Restore snapshot remains available.

## Build and tests

Host build (Linux):

\`\`\`sh
make test
\`\`\`

The host test suite exercises:

- SHA-256 against the standard \`abc\` test vector;
- nested directory copying;
- snapshot creation and verification;
- path/hash comparison;
- Known-Good and Failed classification;
- real retention pruning with preservation checks for permanent archives;
- Pre-Restore creation and restoration of a selected Known-Good snapshot;
- rejection of symbolic links;
- detection of a modified archive payload.

Run the regression suite with \`make test\`. Build the host static library with \`make\`; run AddressSanitizer and UndefinedBehaviorSanitizer with \`make sanitize\` on a compiler/runtime that supports them.

For Vita builds, \`Makefile.vita\` targets \`arm-vita-eabi-gcc\` and defines \`RTSE_PLATFORM_VITA\`. That branch uses the Vita I/O non-shared write-lock flag (\`SCE_O_WRLOCK\`) with non-blocking acquisition (\`SCE_O_NOWAIT\`), \`sceIoSyncByFd\` for file synchronization, and \`sceIoDevctl\` command \`0x3001\` to query free bytes from the storage device. The host implementation uses an advisory \`fcntl\` write lock on the same persistent lock file. The Vita SDK documents these user I/O APIs and the \`SceIoDevInfo.free_size\` field:

- https://docs.vitasdk.org/group__SceFcntlUser.html
- https://docs.vitasdk.org/group__SceDevCtlUser.html
- https://docs.vitasdk.org/group__SceIofilemgrKernel.html

The Vita toolchain is not available in the current development environment. The Vita target recipe and conditional API code are present, but a successful cross-build and hardware run must still be recorded before this module is described as device-ready.

## Integration contract for the next milestone

The boot service should call \`ts_create_snapshot("ur0:tai", "ur0:data/RtSettingsExtended/TaiSaver", TS_ARCHIVE_AUTOMATIC, retention, "Boot snapshot", &result)\` after its startup conditions are satisfied. It must not edit \`config.txt\`, enable/disable plugins or attempt automatic repair.

The Settings page should call the same APIs for manual backup, verification, comparison, classification and restore. Long-running operations will need progress reporting in a later API revision; the UI must not block the Settings thread while hashing or copying large plugin files.
