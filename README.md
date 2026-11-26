# Rt:Settings Extended

A modular extension for the native PlayStation Vita **Settings** application.

Rt:Settings Extended adds integrated feature categories and pages. It is not a separate SPDW control panel.

## Project goals

- Add native-feeling Settings pages with consistent navigation and terminology.
- Keep each feature modular, independently configurable and safe to disable.
- Prefer reversible operations, explicit status reporting and recovery paths.
- Keep all user-facing interface text in English.

## First feature: Tai Saver

Tai Saver protects the configuration in `ur0:tai`.

- Automatic snapshots with configurable retention (2, 3, 4, 5 or Custom).
- Separate Automatic, Known-Good, Failed / KO and Pre-Restore archives.
- Per-file SHA-256 hashes and JSON manifests.
- Snapshot integrity verification and file-level comparison.
- Restore with a verified Pre-Restore safety snapshot and rollback path.
- Automatic rotation applies only to verified Automatic snapshots.

## Repository layout

- `docs/` — architecture, boot integration and Settings integration contracts.
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

A VitaSDK static-library build recipe is provided in `Makefile.vita`. The Vita cross-build, boot adapter, native Settings registration, VPK packaging and physical-device validation are separate milestones. Do not interpret a successful host CI run as proof of Vita compatibility.

## Current development status

**Implemented and host-tested:** Tai Saver C core, SHA-256 manifests, snapshot verification, comparison, archive classification, Automatic-only retention, guarded restore with Pre-Restore snapshot, and the Boot Journal lifecycle state machine.

**Next:** stale-staging recovery, VitaSDK cross-build validation, boot lifecycle adapter, native Settings category/page and packaging.

See:
- [Tai Saver Core](docs/TAI_SAVER_CORE.md)
- [Boot Integration](docs/BOOT_INTEGRATION.md)
- [Native Settings Integration](docs/SETTINGS_NATIVE_INTEGRATION.md)
- [Tai Saver page specification](settings-ui/TAI_SAVER_PAGE.md)

## Primary validation target

PlayStation Vita 1000, firmware 3.65 Ensō, SD2Vita with YAMT. Other models and firmware configurations require separate testing.
