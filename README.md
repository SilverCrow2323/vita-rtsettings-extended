# Rt:Settings Extended

A native-settings extension project for PlayStation Vita.

Rt:Settings Extended extends the existing **Settings** application with additional, integrated categories and pages. It is not a separate SPDW control panel.

## Project goals

- Add native-feeling Settings pages with consistent navigation and terminology.
- Keep each feature modular, independently configurable and safe to disable.
- Prefer reversible operations, explicit status reporting and recovery paths.
- Keep user-facing interface text in English.

## First feature: Tai Saver

Tai Saver protects the configuration in `ur0:tai`.

- Automatic snapshots with configurable retention (2, 3, 4, 5 or Custom).
- Separate Known-Good, Failed / KO and Pre-Restore archives.
- Per-file SHA-256 hashes and JSON manifests.
- Snapshot integrity verification and file-level comparison.
- Restore with a verified Pre-Restore safety snapshot and rollback path.
- Automatic rotation applies only to verified Automatic snapshots. Known-Good, Failed and Pre-Restore archives are never silently removed by that rotation.

## Repository layout

- `docs/` — architecture and implementation notes.
- `include/` — public C interfaces.
- `source/tai-saver/` — Tai Saver engine and SHA-256 implementation.
- `tai-saver/` — feature specification and JSON schemas.
- `settings-ui/` — native Settings integration design.
- `tests/` — host-side regression tests.

## Build and test

Build the host static library and run the regression suite:

```sh
make
make test
make sanitize  # optional; requires AddressSanitizer/UBSan support
```

A VitaSDK static-library build recipe is provided in `Makefile.vita`. It still requires a successful cross-build and testing on a physical or emulated target before a VPK or compatibility claim is made.

## Development status

The Tai Saver C core is implemented and host-tested. The boot plugin entry point, native Settings registration/page, stale-staging recovery, VPK packaging and on-device validation are still pending. See [`docs/TAI_SAVER_CORE.md`](docs/TAI_SAVER_CORE.md) for the API, safety rules, limits and current test coverage.

## Primary validation target

PlayStation Vita 1000, firmware 3.65 Ensō, SD2Vita with YAMT. Other Vita models and firmware configurations require separate testing.
