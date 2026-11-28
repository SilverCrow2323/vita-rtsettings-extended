# Build and validation evidence

**Evidence date:** 28 November 2026  
**Branch:** `feat/tai-saver-core`  
**Scope:** host regression build, sanitizers, VitaSDK static-library cross-build and Rt Vita Cockpit VPK packaging.

## Verified GitHub Actions runs

### Tai Saver Core CI — run 46016522747

- **Host build, regression suite and sanitizers:** passed.
  - Static library build and regression tests passed.
  - AddressSanitizer and UndefinedBehaviorSanitizer run passed.
- **VitaSDK cross-build:** passed.
  - Official `vitasdk/vitasdk` container built the static framework library.
  - CI uploaded the `librtsettings.a` artifact.

Run: https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46016522747

### Rt Vita Cockpit VPK — run 46016522776

- VitaSDK container initialized successfully.
- Cockpit VPK build passed.
- CI uploaded the VPK artifact.

Run: https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46016522776

## What this evidence establishes

The current branch has reproducible host compilation, regression/sanitizer coverage, a VitaSDK static-library cross-build and a CI-produced Cockpit VPK. These results validate the tested build paths and the source revision used by those runs.

## What this evidence does not establish

- No physical PS Vita installation, boot, LiveArea launch or interaction test is claimed.
- The CI VPK artifact is not a published GitHub Release and is not hardware-certified.
- Host-side Vita filesystem mocks do not replace testing against the real `ur0:` / `ux0:` filesystem and VitaSDK runtime.
- Plugin adapters remain subject to per-plugin version, firmware, dependency and conflict verification; catalogue presence is not permission to install or enable a plugin.

## Release gates still required

1. Re-run the host regression and sanitizer workflow on the final PR head.
2. Re-run the VitaSDK static-library and Cockpit VPK builds on the same final head.
3. Inspect the downloaded VPK and checksum/artifact contents independently.
4. Install on a physical PS Vita with the supported firmware/storage configuration.
5. Exercise Tai Saver backup, interrupted-write recovery and restore using a disposable test configuration before any production `ur0:tai` operation.
6. Verify Settings entry placement, plugin help panels, asset import/export, profile operations and service conflict handling on-device.
7. Record firmware, device, storage layout, test procedure and observed results before preparing a stable release.

A green CI run is necessary but not sufficient for release readiness.


## First Tai Saver utility VPK — 28 November 2026

### Rt Vita Cockpit VPK — run 46115106849

- **Build:** passed with the official `vitasdk/vitasdk:latest` container.
- **Package:** `rt-vita-cockpit-tai-saver.vpk` extracted from the uploaded workflow artifact.
- **Artifact:** [GitHub Actions run #46115093537](https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46115106849)
- **Artifact ID:** `15250479092`
- **VPK size:** 105,108 bytes.
- **SHA-256:** `39736d3418d1ce251cd9b27d09eda7be75592d36d928c6b1faca0ff221e9760e`

The VPK links the Tai Saver core into the Cockpit executable and adds a confirmed create + verify Automatic snapshot flow. It contains `sce_sys/param.sfo` and `eboot.bin`. The archive was downloaded, extracted and inspected in the build workspace.

The Tai Saver utility writes its archive under `ur0:data/RtSettingsExtended/TaiSaver/` and reads `ur0:tai/` as its source. It does not edit the live taiHEN configuration. This is a CI-built VPK, not a physical-device-tested package.

The host regression suite completed successfully and the VitaSDK static-library cross-build completed successfully in the same validation cycle. The AddressSanitizer/UndefinedBehaviorSanitizer job in run `46115093542` subsequently completed successfully.


## Native Rt Manager installer VPK — 28 November 2026

### Final native Settings build — commit `18787ea76896642b8abfc2e9b2feb73256b5a4d1`

- **Host build and regression suite:** passed.
- **AddressSanitizer / UndefinedBehaviorSanitizer:** passed.
- **VitaSDK cross-build:** passed, including the native Settings SUPRX.
- **Cockpit VPK build:** passed using the official VitaSDK container.
- **PR:** #1 remains Draft.

CI runs:

- Host, sanitizer and VitaSDK cross-build: https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46138549229
- Cockpit VPK build: https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46138549187
- VPK artifact ID: `15260657469`

### Independently inspected package

- VPK filename: `rt-vita-cockpit.vpk`
- Title ID: `RTV000001`
- VPK size: 115,846 bytes.
- VPK SHA-256: `435d37ab85d5a786c68ea28ad16b068e9cdd221d70074a9f8596d922aef488da`
- Embedded `rtsettings.suprx`: 15,141 bytes.
- Embedded SUPRX SHA-256: `3733840765ebcea3d9d906f23ab72f069850f4c588ffe41e04f933322448096a`
- Package entries inspected: `sce_sys/param.sfo`, `eboot.bin`, `rtsettings.suprx`.

### What this VPK now does

The VPK is the Rt Vita Cockpit installer. After explicit confirmation in Cockpit, it:

1. Reads the embedded `app0:rtsettings.suprx` payload.
2. Checks the existing `ur0:tai/config.txt` and the exact `*NPXS10015` section.
3. Refuses duplicate/ambiguous Rt entries, duplicate title sections, stale transaction state, or a different existing `rtsettings.suprx`.
4. Creates a verified permanent Tai Saver Pre-Restore snapshot of `ur0:tai`.
5. Writes an additional config backup when the config will change.
6. Stages and verifies the SUPRX and updated config, then commits the module and its single taiHEN line with synchronous rollback on errors.
7. Enables an existing commented Rt line only after the user confirms Install/Enable.

The native menu target is:

```text
Settings
└── Rt:Settings Extended
    └── Preferences & Settings Management
        └── Rt Manager
            └── Rt Manager Status
```

The root XML is modified in memory only. Existing Sony XML files and unrelated taiHEN entries are not rewritten.

### Remaining release boundary

This is the first native Settings integration milestone, not the complete Rt Settings suite. Only the Rt Manager Status action is exposed in Sony Settings. Plugin Control Centre, conflict/dependency controls, Tai Saver operations, other feature pages, update/uninstall and repair of a different existing SUPRX are not yet exposed.

**No physical PS Vita test is claimed.** The VPK and SUPRX are CI-built and inspected, but the menu placement, nested navigation, callback behavior, coexistence with the user's active plugin chain, config installation on the real filesystem and reboot behavior still require a controlled test on the target PS Vita 1000 OLED running 3.65 Ensō.
