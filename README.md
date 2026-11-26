# Rt:Settings Extended

A modular extension for the native PlayStation Vita **Settings** application. It adds integrated feature categories and pages; it is not a separate SPDW control panel.

## Expandable installer

Rt:Settings Extended is designed around one expandable installer. The framework is installed once; feature pages are independent modules that can be added in later releases. Users can choose modules during setup, add them later, update or repair them independently, and remove a module without deleting its user data.

The package and module contracts are defined in `installer/`. The Vita installer executable and on-device transaction engine are not yet built; see [Expandable Installer](docs/EXPANDABLE_INSTALLER.md).

## First feature: Tai Saver

Tai Saver protects the configuration in `ur0:tai`:
- Automatic snapshots with configurable retention (2, 3, 4, 5 or Custom).
- Separate Automatic, Known-Good, Failed / KO and Pre-Restore archives.
- Per-file SHA-256 hashes and JSON manifests.
- Snapshot integrity verification and file-level comparison.
- Restore with a verified Pre-Restore safety snapshot and rollback path.
- Automatic rotation applies only to verified Automatic snapshots.

## Repository layout

- `installer/` — expandable package format, module manifests and installer contract.
- `docs/` — architecture, boot integration, native Settings and installer contracts.
- `include/` — public C interfaces.
- `source/tai-saver/` — Tai Saver engine and SHA-256 implementation.
- `source/boot-monitor/` — boot lifecycle journal.
- `tai-saver/` — feature specification and JSON schemas.
- `settings-ui/` — native Settings page design.
- `tests/` — host-side regression tests.

## Build and test

```sh
make
make test
make sanitize
```

A VitaSDK static-library build recipe is provided in `Makefile.vita`. The Vita cross-build, boot adapter, native Settings registration, installer VPK and physical-device validation are separate milestones. A successful host CI run is not proof of Vita compatibility.

## Current development status

**Implemented and host-tested:** Tai Saver C core, SHA-256 manifests, snapshot verification, comparison, archive classification, Automatic-only retention, guarded restore with Pre-Restore snapshot and Boot Journal lifecycle state handling.

**Defined:** expandable package format, package/module JSON schemas, first Tai Saver module descriptor, module selection and transaction/rollback contract.

**Next:** installer transaction engine and Vita UI, boot lifecycle adapter, VitaSDK cross-build validation and native Settings category/page.

See [Tai Saver Core](docs/TAI_SAVER_CORE.md), [Boot Integration](docs/BOOT_INTEGRATION.md), [Native Settings Integration](docs/SETTINGS_NATIVE_INTEGRATION.md), [Expandable Installer](docs/EXPANDABLE_INSTALLER.md), and [Tai Saver page specification](settings-ui/TAI_SAVER_PAGE.md).

## Primary validation target

PlayStation Vita 1000, firmware 3.65 Ensō, SD2Vita with YAMT. Other models and firmware configurations require separate testing.
