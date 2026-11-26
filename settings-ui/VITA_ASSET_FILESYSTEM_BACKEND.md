# Vita Filesystem Bridge — Custom Assets Manager

## Scope of this milestone

The first VitaSDK-specific filesystem bridge is now present:

- Bounded, offset-based file reads through `sceIoOpen` + `sceIoPread` + `sceIoClose`.
- A callback with the exact signature expected by `rta_asset_hash_reader`.
- Fail-closed path validation for Vita device paths; traversal segments and control characters are rejected.
- Bootstrap of the Rt-owned `ur0:data/rt/assets` directory, accepting an existing directory but rejecting a non-directory at any level.
- A conservative file-existence probe that never interprets an arbitrary I/O error as proof that a path is absent.

The bridge is intentionally read-only at this stage. It does not import, overwrite, rename, delete, activate, or persist manifests. Those operations remain behind the portable file-backend contract until their no-clobber and recovery semantics are implemented and tested.

## VitaSDK API basis

The implementation uses the documented user-mode filesystem API:

- `sceIoOpen`, `sceIoPread`, and `sceIoClose` for bounded reads: [VitaSDK File Control Library](https://docs.vitasdk.org/group__SceFcntlUser.html).
- `sceIoMkdir` and `sceIoGetstat` for directory bootstrap and type checks: [VitaSDK File Status Library](https://docs.vitasdk.org/group__SceStatUser.html).
- `SCE_S_ISDIR` to distinguish an existing directory from a file or another filesystem object.

No kernel APIs are used. The bridge is compiled only in the Vita target and is not included in the host library.

## Public interface

`include/vita_asset_fs.h` exposes:

- `rta_vita_asset_read`: returns 1 when bytes are read, 0 at EOF, and -1 on invalid input or I/O failure.
- `rta_vita_asset_ensure_directory`: creates only `ur0:data`, `ur0:data/rt`, and `ur0:data/rt/assets`.
- `rta_vita_asset_file_exists`: returns 1 when the path is found and -1 when it cannot be confirmed. This is deliberately fail-closed because VitaSDK's generic stat error does not let this wrapper safely classify every failure as “missing”.

Example hash integration:

```c
char digest[65];
uint64_t size = 0;

if (rta_vita_asset_ensure_directory() == 0 &&
    rta_asset_hash_reader(rta_vita_asset_read, NULL,
                          "ux0:data/appearance/custom.png",
                          digest, &size) == RTA_OK) {
    /* digest and size now describe the file read from the Vita filesystem. */
}
```

## Safety decisions

- Reads are bounded; the entire asset is never loaded into memory.
- The path must use one of the supported Vita device prefixes and must not contain traversal or control characters.
- File descriptors are closed on every read path.
- A close failure is reported as an I/O failure rather than silently accepting an uncertain read.
- Directory creation is limited to the project-owned asset directory.
- Existing paths are accepted only when `sceIoGetstat` confirms they are directories.
- No write or rename operation is exposed yet. In particular, a stat-then-rename sequence is not treated as a sufficient no-clobber guarantee.

## Not yet implemented / validated

- Staged file copy and atomic commit.
- Persistent, versioned asset manifest and crash recovery.
- Safe no-clobber semantics for destination files.
- Vita-side plugin detection and activation adapters.
- A successful VitaSDK cross-build and physical-console validation.

Host-side Custom Assets Manager tests continue to validate the portable registry/import/export contract. This bridge itself requires a VitaSDK build and on-device test before it can be considered device-validated.
