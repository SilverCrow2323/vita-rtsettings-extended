# Custom Assets Manager Core

## Purpose

A shared, bounded registry and transaction boundary for boot splash images, boot animations, startup sounds, custom warning assets and supported interface themes. Feature adapters remain responsible for their own file formats and plugin APIs; the core does not pretend all boot engines share one format.

## Implemented portable core

- Registry snapshots with stable IDs, human-readable titles, owner, path, format, size, dimensions/duration, SHA-256, compatibility and ownership metadata.
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

## Safety and deliberate limits

- No arbitrary file writes or deletes occur in the portable core.
- No external path is accepted for activation: an asset must be Rt-managed, compatible and hash-verified.
- Active assets cannot be deleted. Backup assets are protected. Orphaned assets are never deleted automatically.
- The core does not calculate hashes itself; the import/filesystem adapter must hash the staged file and set `hash_verified` only after a successful read-back comparison.
- It does not convert formats. CBSM, vita-bootanim, CustomBootSplash and Vita-BootSound each need a specific adapter/converter and their own version/firmware capability checks.
- No Vita filesystem or Sony Settings integration is claimed by this host-tested core.

## Tests

`tests/test_custom_assets.c` covers registry validation, duplicate IDs/paths, path traversal, type size limits, successful activation, failed activation verification with rollback, rollback failure, external asset refusal, protected backup refusal and deletion failure/success.
