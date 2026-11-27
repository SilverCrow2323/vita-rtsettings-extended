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
