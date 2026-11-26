# Expandable Installer and Module System

## Goal

Ship one installer for Rt:Settings Extended and grow it by adding independent modules. The framework is installed once; feature modules can be selected, installed, updated, repaired, disabled or removed independently.

## Components

- **Package manifest:** release identity, framework version, firmware compatibility and available module list.
- **Module manifest:** stable ID, version, dependencies, conflicts, page entry, payload ownership and data-preservation rules.
- **Installer UI:** module selection, compatibility warnings, change preview, progress and final report.
- **Transaction engine:** staging, hash verification, journaling, commit and rollback.
- **Installed registry:** framework/module versions and exact files owned by each component.

## Installation modes

| Mode | Result |
|---|---|
| Fresh install | Framework plus selected modules |
| Add modules | Installs newly selected modules and dependencies |
| Update | Replaces framework/module payloads while preserving user data |
| Repair | Restores missing or modified package-owned files |
| Disable module | Keeps files/data but prevents page/service registration |
| Remove module | Removes only module-owned files; preserves data by default |
| Uninstall framework | Removes framework-owned files after dependent modules are handled; preserves user archives |

## Dependency rules

- Module IDs are stable, lowercase identifiers.
- Dependencies are resolved before staging; missing or incompatible dependencies block the plan.
- Conflicts are reported before any file is written.
- Required modules cannot be unchecked.
- Optional modules are independently selectable.
- Payloads may not write outside declared owned paths.
- The installed registry is committed only after payload operations succeed.

## Update and rollback

Updates compare installed and incoming file hashes. Unchanged files are left alone. Replaced files are backed up to a transaction-specific directory. User data is not part of the replaceable payload.

The journal records planned operations and completion state. On failure, operations are replayed backwards. If rollback is incomplete, preserve the journal and backups and report Recovery Required; never clean up evidence automatically.

## First module: Tai Saver

Tai Saver is the first module. Its settings page and service depend on the shared framework. Its archive data lives under `ur0:data/RtSettingsExtended/TaiSaver/` and is preserved on update, module removal and framework uninstall unless the user explicitly chooses otherwise.

## Adding future features

1. Create a stable module ID and a `module.json`.
2. Add its payload and settings schema.
3. Declare dependencies and exact owned paths.
4. Add its page/service registration to the framework registry.
5. Add host tests for install, update, rollback and removal.
6. Add the module to the package manifest.

The same installer automatically lists it as an optional module. No installer rewrite is required for each new Settings section.

## Safety boundary

The package format is defined now. The Vita installer executable is a separate deliverable and must not be represented as complete until its UI, transaction engine, VPK build and on-device tests exist. No install or uninstall path may silently rewrite `ur0:tai/config.txt`.
