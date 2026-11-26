# Boot Integration — Tai Saver

## Status

Design contract for the next implementation milestone. This document does **not** claim that a boot plugin or a VPK exists. The C core is host-tested; the Vita startup adapter still needs a VitaSDK build and device validation.

## Objective

When enabled, create one verified snapshot of `ur0:tai` during each normal console startup, without editing the live taiHEN configuration and without holding up the user interface.

The boot adapter is deliberately thin. It schedules work and records lifecycle state; all archive operations remain in `source/tai-saver/tai_saver.c`.

## Startup placement

Do not perform recursive filesystem copying from an early `*KERNEL` plugin callback. At that stage, storage availability and startup ordering may not be suitable for a multi-file transaction.

The selected adapter must run only after:
1. the system has mounted `ur0:`;
2. taiHEN has finished reading its active configuration;
3. the target `ur0:tai` directory can be opened and enumerated.

The first candidate is a user-mode SceShell-associated startup adapter with a worker thread. This is a candidate, not an asserted compatibility fact: confirm the relevant taiHEN load section, plugin lifecycle and storage readiness on the target firmware before enabling automatic backups by default. If that timing cannot be proven, use an explicitly installed startup homebrew/service mechanism rather than a kernel hook.

## Non-blocking execution model

1. Entry point performs minimal initialization and returns control promptly.
2. A worker waits for the required mount/readiness checks with a bounded timeout.
3. Worker reads the Tai Saver settings file. Missing or invalid settings use safe defaults; they never disable verification.
4. If automatic backup is disabled, record `SKIPPED_DISABLED` and exit.
5. Acquire the core transaction lock through `ts_create_snapshot`; do not implement a second lock in the adapter.
6. Create an Automatic snapshot using the configured retention.
7. Verify the resulting archive and persist the result.
8. Emit a completion marker and release resources.

The worker must have bounded retries, a bounded wait, and a bounded amount of work per invocation. It must not spin while waiting for storage.

## Persistent boot journal

Keep the journal outside `ur0:tai`, under:

```text
ur0:data/RtSettingsExtended/TaiSaver/
├── boot/
│   ├── boot-state.json
│   └── events/
└── automatic/
```

The journal is diagnostic metadata, not a substitute for the archive manifest. Writes use temp-file + flush/sync + rename. Never overwrite the only valid journal copy before the replacement is durable.

Suggested state fields:

| Field | Meaning |
|---|---|
| `schema_version` | Journal schema |
| `boot_id` | Unique identifier for this startup attempt |
| `state` | Current lifecycle state |
| `started_at` | UTC timestamp |
| `snapshot_id` | Completed archive ID, if any |
| `last_error` | Stage and error code, if any |
| `shell_ready` | Whether the SceShell-ready point was observed |

States:
- `STARTED`
- `WAITING_FOR_STORAGE`
- `BACKUP_RUNNING`
- `BACKUP_VERIFIED`
- `SHELL_READY`
- `SKIPPED_DISABLED`
- `FAILED_STORAGE_TIMEOUT`
- `FAILED_BACKUP`
- `INTERRUPTED`

At the beginning of a new run, if the previous journal remains in a non-terminal state, append an `INTERRUPTED` event for that prior run before starting the new one. Do not infer that a plugin or taiHEN configuration was defective: an interrupted run only proves that the previous adapter did not record its terminal state.

A verified snapshot and a shell-ready event are separate facts. Do not label an archive Known-Good based on either event.

## Staging recovery

A hard power loss can leave a directory under `staging/`. The boot adapter must not promote it or offer it for restore.

Recovery policy:
- enumerate only the Tai Saver-owned staging directory;
- reject symlinks and unexpected entry types;
- inspect each staging directory without following links;
- quarantine it under a timestamped `failed/interrupted-<id>` diagnostic directory, or leave it untouched if the move cannot be completed safely;
- never delete an unknown or unverified directory automatically;
- write a diagnostic report containing the reason and original path;
- completed archives are unaffected.

This recovery pass must be implemented in the core/storage layer and covered by host tests before the boot adapter calls it.

## Failure policy

| Failure | Required behavior |
|---|---|
| `ur0:` not ready before timeout | Record storage timeout; do not create an empty archive |
| `ur0:tai` missing | Record source missing; do not create a success marker |
| Insufficient space | Record required/available bytes if known; preserve existing archives |
| Lock unavailable | Exit as busy; do not retry tightly |
| Copy/hash/manifest failure | Core rejects the snapshot; do not apply retention |
| Journal write failure | Report degraded diagnostics; never claim the journal is current |
| Shell exits during work | Next startup records the previous run as interrupted |

No automatic repair, plugin toggling, config rewriting or taiHEN reload is permitted in the boot path.

## Storage strategy

The initial target is `ur0:data/RtSettingsExtended/TaiSaver/` because it is independent of the user's SD2Vita mount. Do not silently fall back to `ux0:` or another volume. A future mirrored emergency copy may be added only with its own capacity checks, manifest and explicit retention rules.

## Acceptance criteria

- [ ] VitaSDK adapter builds with the selected toolchain.
- [ ] Startup placement is proven on Vita 1000 / 3.65 Ensō.
- [ ] Backup starts only after storage readiness checks pass.
- [ ] Settings UI remains responsive while backup runs.
- [ ] Exactly one Automatic snapshot is produced per enabled successful startup.
- [ ] Storage timeout, missing source, insufficient space and lock contention are recorded distinctly.
- [ ] A hard-interrupted staging directory is quarantined or safely left untouched.
- [ ] Automatic retention runs only after a newly verified snapshot.
- [ ] No boot path writes to `ur0:tai`.
- [ ] Device logs and a repeatable test procedure are attached before marking the milestone complete.
