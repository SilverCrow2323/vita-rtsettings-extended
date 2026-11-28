# Project architecture

## 1. Product shape

Rt:Settings Extended adds feature pages to the Vita's existing Settings experience. Each feature has a clear settings entry, a focused page hierarchy and a service layer that performs the actual work. A separate, unrelated settings launcher is not the primary interface.

## 2. Layers

### Settings integration
Owns category registration, page navigation, labels, descriptions, controls and status presentation. UI callbacks request operations; they do not implement filesystem logic.

### Feature services
Owns feature-specific behavior. The first service is Tai Saver, responsible for snapshots, manifests, integrity checks, retention and restore preparation.

### Storage layer
Owns paths, directory creation, atomic writes, file enumeration, hashing and safe copy operations. It reports partial failures rather than treating a partially copied archive as valid.

### Boot integration
Starts the automatic backup task during boot. It must avoid racing taiHEN configuration loading or modifying the live ur0:tai directory. The initial implementation is conservative: snapshot only, no automatic repair or rewrite at boot.

## 3. Safety invariants

1. Never modify the live ur0:tai configuration during a normal backup.
2. A snapshot is not complete until all files have been copied and its manifest has been written and verified.
3. Incomplete snapshots are invalid and excluded from restore choices.
4. Restore always creates a separate pre-restore safety snapshot first.
5. Automatic retention never deletes Known-Good or Failed/KO archives.
6. Restore requires explicit user confirmation.
7. No failed backup may be reported as successful.
8. Paths are normalized and checked to prevent traversal outside the Tai Saver storage root.

## 4. Archive classes

- Automatic: timestamped boot snapshots subject to configured retention.
- Known-Good: snapshots explicitly marked by the user as known to work; retained until explicit deletion.
- Failed: snapshots explicitly marked as problematic or captured for diagnosis; retained until explicit deletion.

## 5. Backup transaction

1. Resolve and validate the source directory.
2. Create a uniquely named temporary snapshot directory.
3. Enumerate source files deterministically.
4. Copy files while recording relative path, byte size and cryptographic hash.
5. Write the manifest after the copy completes.
6. Re-read and verify copied files against the manifest.
7. Atomically promote the temporary directory to a completed snapshot.
8. Apply retention to Automatic archives only.
9. Record a concise result for the Settings status page and diagnostic log.

Any failure before step 7 leaves an incomplete record, never a valid restore point.

## 6. Restore transaction

1. User selects a completed snapshot and reviews timestamp, class and verification status.
2. Verify the selected archive again.
3. Create and verify a pre-restore safety snapshot of the current live configuration.
4. Stage selected files in a temporary restore directory.
5. Validate the staged result.
6. Replace the live configuration using the safest supported filesystem strategy.
7. Verify restored files and report the outcome.

If the platform cannot guarantee a safe replacement strategy, the implementation must stop and report that limitation rather than improvise.

## 7. Platform boundary

The repository starts with platform-independent specifications and a modular implementation boundary. Vita-specific APIs, plugin entry points, Settings registration and build dependencies must be confirmed against the selected Vita SDK and firmware before claiming a working VPK.

## 8. First milestone acceptance criteria

- Project structure and build assumptions are documented.
- Tai Saver's archive model and safety rules are implemented and testable.
- Automatic retention is isolated from Known-Good and Failed archives.
- Manifests support integrity verification and file comparison.
- Restore has a mandatory pre-restore safety snapshot.
- Settings UI exposes status, retention and archive management without hiding failures.
