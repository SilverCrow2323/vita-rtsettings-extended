# Build and validation evidence

**Evidence date:** 27 November 2026  
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

### Rt Vita Cockpit VPK — run 46115093537

- **Build:** passed with the official `vitasdk/vitasdk:latest` container.
- **Package:** `rt-vita-cockpit-tai-saver.vpk` extracted from the uploaded workflow artifact.
- **Artifact:** [GitHub Actions run #46115093537](https://github.com/SilverCrow2323/vita-rtsettings-extended/actions/runs/46115093537)
- **Artifact ID:** `15250479092`
- **VPK size:** 105,108 bytes.
- **SHA-256:** `339fd5e497c5c99287acfac51f468146b227348ffbdccb48ae362f154594942f`

The VPK links the Tai Saver core into the Cockpit executable and adds a confirmed create + verify Automatic snapshot flow. It contains `sce_sys/param.sfo` and `eboot.bin`. The archive was downloaded, extracted and inspected in the build workspace.

The Tai Saver utility writes its archive under `ur0:data/RtSettingsExtended/TaiSaver/` and reads `ur0:tai/` as its source. It does not edit the live taiHEN configuration. This is a CI-built VPK, not a physical-device-tested package.

The host regression suite and VitaSDK static-library cross-build completed successfully in the same validation cycle. The host AddressSanitizer/UndefinedBehaviorSanitizer step was still running at the time of this record; do not treat that sanitizer job as passed until its workflow result is complete.
