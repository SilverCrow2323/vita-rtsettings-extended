# Rt:Settings Extended

A native-settings extension project for PlayStation Vita.

Rt:Settings Extended extends the existing **Settings** application with additional, integrated categories and pages. It is not a separate SPDW control panel.

## Project goals

- Add native-feeling settings pages with consistent navigation and terminology.
- Keep each feature modular, independently configurable and safe to disable.
- Prefer reversible operations, explicit status reporting and recovery paths.
- Keep user-facing interface text in English.

## First implementation milestone: Tai Saver

Tai Saver protects the configuration in ur0:tai by creating a backup at each boot.

- Automatic boot-time snapshots.
- Configurable retention for automatic snapshots (2, 3, 4, 5 or a custom count).
- Separate Known-Good and Failed/KO archives.
- File hashes and manifests for integrity checks and comparisons.
- Restore and compare workflows.
- An additional safety copy before any restore or destructive operation.

**Retention safety rule:** automatic rotation applies only to ordinary Automatic snapshots. Known-Good and Failed/KO archives are preserved separately and are never silently removed by automatic rotation.

## Repository layout

- docs/ — architecture, integration notes and UX decisions.
- tai-saver/ — backup, archive, integrity and recovery subsystem.
- settings-ui/ — native Settings integration design.
- include/ and source/ — shared headers and implementation as the codebase is brought online.

## Status

Initial project bootstrap. Active development branch: feat/tai-saver-core. Hardware validation and final plugin integration are pending; no compatibility claim is made until tested on-device.

## Target environment

Primary validation target: PlayStation Vita 1000, firmware 3.65 Ensō, SD2Vita with YAMT. Other Vita models and firmware configurations require separate testing.
