# Custom Assets Manager Core

## Purpose

A shared, bounded registry and transaction boundary for boot splash images, boot animations, startup sounds, custom warning assets and supported interface themes. Feature adapters remain responsible for their own file formats and plugin APIs; the core does not pretend all boot engines share one format.

## Implemented portable core

- Registry snapshots with stable IDs, human-readable titles, owner, source, path, format, size, dimensions/duration, audio sample rate/channels, SHA-256, compatibility and ownership metadata.
- Duplicate ID/path detection and summary counts for active, backup, orphaned, incompatible and unverified assets.
- Vita mount-path validation and traversal rejection.
- Conservative size ceilings: boot sound 1 MiB, boot animation 108 MiB, splash/theme 32 MiB, warning 16 KiB and other assets 128 MiB.
- Transactional activation through a platform adapter: activate, verify active state, and restore the previous asset or platform default if activation/verification fails.
- Removal only for Rt-managed inactive assets; protected backups and unknown/orphaned files are not deleted.
- Fixed-size storage and C99 API; no dynamic allocation.

## Adapter contract

The platform layer supplies:
- `activate(context, type, path)`: stage/apply the feature using the actual upstream plugin or system interface.
- `verify_active(context, type, path)`: confirm the selected asset is really active.
- `restore(context, type, previous_path)`: restore the previous asset; a null path means restore the engine's default.
- `remove(context, path)`: delete a managed file only after the core's ownership/state checks.

The core updates its registry only after activation and read-back both succeed. If either fails, it invokes restore. A failed restore returns `RTA_ERR_ROLLBACK`; callers must display a recovery warning and retain the transaction journal.

## Vita filesystem bridge

A VitaSDK-specific, read-only bridge now provides the bounded reader used by the core's streaming SHA-256 API. It validates Vita device paths, rejects traversal/control characters, creates only the Rt-owned `ur0:data/rt/assets` directory tree, and checks that pre-existing path components are directories. It does not expose write, rename, delete or activation operations. A mocked VitaSDK harness compiles and tests the adapter source in host CI, including read offsets, EOF, invalid paths, close failures, directory creation, wrong-type paths and stat errors. The exact API and validation boundary are documented in [Vita Asset Filesystem Bridge](VITA_ASSET_FILESYSTEM_BACKEND.md).

## Safety and deliberate limits

- No arbitrary file writes or deletes occur in the portable core.
- No external path is accepted for activation: an asset must be Rt-managed, compatible and hash-verified.
- Active assets cannot be deleted. Backup assets are protected. Orphaned assets are never deleted automatically.
- Streaming SHA-256 hashing and verification are implemented through an injected bounded reader callback; the Vita filesystem adapter supplies the actual read implementation. The core never loads an entire asset into memory.
- It does not convert formats. CBSM, vita-bootanim, CustomBootSplash and Vita-BootSound each need a specific adapter/converter and their own version/firmware capability checks.
- The Vita bridge currently supports bounded reads and owned-directory bootstrap only. Staged writes, safe no-clobber commit, persistent manifest recovery and Sony Settings integration remain pending. A VitaSDK cross-build and physical-console test have not yet been completed.

## Tests

`tests/test_custom_assets.c` covers registry validation, duplicate IDs/paths, path traversal, type size limits, successful activation, failed activation verification with rollback, rollback failure, external asset refusal, protected backup refusal and deletion failure/success.


## Startup recovery

The portable manifest layer now includes a recovery loader that validates the
primary manifest first and falls back to a separately retained backup only when
the primary cannot be read or fails decoding/integrity validation. The caller
receives an explicit `used_backup` flag; the destination registry changes only
after one complete copy validates. If both copies fail, the primary failure is
returned to preserve the original diagnostic context.

Host regression tests simulate a corrupted primary and a valid backup. This
primitive does not create or rotate backups itself: the platform persistence
backend must preserve the previous manifest before replacing the primary. Vita
backup rotation and crash-recovery integration remain pending.


## Validated backup rotation

The portable manifest API now exposes `rta_asset_manifest_save_rotate`. It
encodes the candidate manifest, checks whether the primary exists, reads and
fully decodes the previous primary, and asks the platform backend to perform a
single crash-consistent primary/backup replacement. Only a valid previous
primary is eligible to become the backup. A missing or invalid primary is
replaced without touching an existing backup, preventing a corrupt file from
poisoning the last known-good copy.

After the backend transaction, the core reads back and validates the new
primary. When a previous primary was preserved, it also verifies the backup's
length, SHA-256 digest and manifest decoding. Backend failure is reported
without pretending the core can roll back a partially completed filesystem
operation.

The backend contract is deliberately strict: `replace_with_backup_atomic`
must make the pair update crash-consistent as one operation, or reject it.
A sequence of ordinary `rename`, `remove` or `write` calls does not satisfy
this contract. The host fake backend exercises successful rotation, preservation of a distinct pre-existing backup when the primary is corrupt, injected transaction failure and readback verification. No Vita implementation of this
pair-transaction callback exists yet.
