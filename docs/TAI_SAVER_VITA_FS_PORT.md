# Tai Saver — Vita filesystem porting layer

## Delivered

The project now has a small C99 filesystem boundary in `include/rt_fs.h` with implementations in `source/platform/rt_fs.c`.

The Vita branch uses VitaSDK user-mode I/O APIs:
- `sceIoOpen`, `sceIoRead`, `sceIoWrite`, `sceIoLseek`, `sceIoClose`;
- `sceIoSyncByFd`;
- `sceIoGetstat`, `sceIoMkdir`, `sceIoRemove`, `sceIoRmdir`, `sceIoRename`;
- `sceIoDopen`, `sceIoDread`, `sceIoDclose`;
- `sceIoDevctl(..., 0x3001, ...)` for device free-space information.

The API deliberately exposes byte counts for reads and writes. Callers must handle short I/O rather than assuming a request completes in one call. Directory iteration returns 1 for an entry, 0 for end-of-directory, and a negative error. Vita error codes are preserved where the underlying API supplies them.

The host implementation uses POSIX and is covered by `tests/test_rt_fs.c`. The adapter is included in both the host framework build and the VitaSDK static-library build recipe.

## Safety boundaries

- `rtfs_rename` is a thin platform wrapper. It does not claim atomic replacement, no-clobber behavior, or parent-directory durability.
- `rtfs_sync` calls `sceIoSyncByFd(fd, 0)`; this does not establish stronger durability guarantees than VitaSDK documents.
- Vita `SceDateTime` is not converted into a guessed Unix timestamp; `mtime` remains zero until a verified conversion is added.
- The Vita directory entry name is checked for a NUL terminator within the fixed 256-byte field before copying.
- The adapter does not follow directory entries recursively and does not delete anything on its own.
- No kernel I/O APIs or kernel plugin dependency are introduced.

## Important integration status

This is the platform boundary for the Tai Saver migration, **not yet the active backend of `source/tai-saver/tai_saver.c`**. The existing Tai Saver core still contains direct POSIX/stdio calls (including `FILE *`, `fopen`, `fdopen`, `fread`, directory traversal and file metadata). Until those operations are migrated to this boundary and covered by injected failure tests, the Tai Saver filesystem/startup backend must remain marked pending.

The next implementation step is to route the entire Tai Saver core through an injectable filesystem vtable (including manifest streaming I/O, recursive enumeration, file identity/change checks, directory creation, removal and transaction moves). A partial migration must not be enabled on-device.

## Validation gate

- [x] Header and host implementation added.
- [x] Vita implementation uses documented VitaSDK user-mode APIs.
- [x] Host regression test added and wired into `make test`.
- [ ] Host regression and sanitizer CI green on this commit.
- [ ] VitaSDK cross-build green on this commit.
- [ ] Tai Saver core migrated to the adapter; no direct POSIX/stdio filesystem calls remain in the Vita build.
- [ ] Failure injection for short reads/writes, sync, close, directory read/close, rename, mkdir, remove and storage-query errors.
- [ ] Physical validation on Vita 1000 / firmware 3.65 Ensō.

## Header references inspected

The VitaSDK `vitasdk/vita-headers` definitions were inspected for:
- `include/psp2/io/fcntl.h`;
- `include/psp2/io/dirent.h`;
- `include/psp2/io/stat.h`;
- `include/psp2/io/devctl.h`;
- `include/psp2common/kernel/iofilemgr.h`.

The relevant documented directory semantics are explicit: `sceIoDread` returns 0 at end, a positive value when an entry is returned, and a negative value on error. The API comments also identify the file-descriptor sync flag as unknown. These details are reflected in the wrapper contract rather than inferred from POSIX behavior.
