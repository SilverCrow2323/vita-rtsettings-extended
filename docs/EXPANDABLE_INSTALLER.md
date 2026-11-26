# Expandable Installer and Module System

## Product goal

Rt:Settings Extended uses **one expandable installer** for the shared framework and every feature module. New Settings sections are delivered as modules, not as separate installers. The installer discovers the modules included in its package and presents the compatible ones to the user.

The user must be able to install one section at a time, choose any combination, or install all compatible sections in the current package.

## User installation choices

The installer UI is planned to offer:

- **Install selected:** install only the checked modules.
- **Select All:** check every compatible optional module in the current package.
- **Deselect All:** clear optional selections.
- **Add modules:** keep the existing framework and installed modules, then add selected new modules.
- **Update:** update the framework, selected modules, or all installed modules.
- **Repair:** restore missing or modified package-owned files.
- **Disable module:** retain files and user data but stop registering its page/service.
- **Remove module:** remove only that module's owned program files; preserve its user data by default.
- **Uninstall framework:** remove the shared framework after dependent modules are handled; preserve user data unless the user explicitly chooses otherwise.

The shared framework is installed automatically whenever a selected module requires it. Required dependencies are selected automatically and cannot be unchecked. A module that is incompatible or has unresolved dependencies must be blocked with a clear explanation.

**Install All applies to all compatible modules shipped in the package being installed.** It does not imply downloading modules that are not present in that package. When a later release adds modules, rerunning the same installer (or its updated package) makes those modules available for individual selection or Install All.

## Components

- **Package manifest:** product identity, release version, framework version, firmware compatibility and available module list.
- **Module manifest:** stable ID, version, dependencies, conflicts, page entry, payload ownership, settings schema and data-preservation rules.
- **Installer UI:** installation mode, module selection, Select All / Deselect All, compatibility warnings, change preview, progress and final report.
- **Transaction engine:** staging, hash verification, journaling, commit and rollback.
- **Installed registry:** framework/module versions, enabled state and exact files owned by each component.

## Installation flow

1. Read and validate the package manifest and module descriptors.
2. Detect an existing Rt:Settings Extended installation.
3. Check firmware, framework compatibility, dependencies and conflicts.
4. Present available modules and preselect required dependencies.
5. Let the user select individual modules or choose Select All.
6. Resolve dependencies and build a complete install/update/remove plan.
7. Show the exact changes, storage requirements and warnings for confirmation.
8. Stage payload files away from live destinations and verify their hashes.
9. Journal the transaction and back up owned files that will be replaced.
10. Commit the selected framework/module payloads.
11. Write the installed-module registry last, after all required operations succeed.
12. Report the result and retain recovery information if any step fails.

No partial module installation should be reported as successful.

## Package and module contracts

Each module has a stable lowercase ID and a `module.json` descriptor. It declares:

- display name and description;
- module version and framework version range;
- minimum/maximum firmware compatibility;
- dependencies and conflicts;
- Settings page entry identifier;
- payload root and exact owned paths;
- settings schema;
- user-data paths and uninstall preservation behavior.

The package manifest lists the modules available in that release and marks required/default-selected modules. The framework itself is shared and is not copied once per module.

The JSON schemas and first Tai Saver descriptor live in this directory.

## Adding future modules

1. Create a stable module ID and `module.json`.
2. Implement the module's Settings page and service integration.
3. Add its payload and settings schema.
4. Declare dependencies, conflicts, compatibility and exact owned paths.
5. Add host tests for install, update, rollback, disable and removal.
6. Add the module descriptor and payload to the package manifest.

The installer then lists the new module automatically. Existing users can install just that module or choose Install All. The installer itself does not need a bespoke rewrite for every new Settings section, provided the module uses the supported framework and manifest contracts.

## Update and rollback

Updates compare installed and incoming file hashes. Unchanged files are left alone. Replaced files are backed up to a transaction-specific directory. User data is not part of the replaceable payload.

The journal records planned operations and completion state. On failure, operations are replayed backwards. If rollback is incomplete, preserve the journal and backups and report **Recovery Required**; never clean up evidence automatically.

The installed registry is updated only after all selected operations succeed. It must accurately describe which modules are installed, enabled and which files are owned by each component.

## First module: Tai Saver

Tai Saver is the first module. Its Settings page and service depend on the shared framework. Its archive data lives under `ur0:data/RtSettingsExtended/TaiSaver/` and is preserved on update, module removal and framework uninstall unless the user explicitly chooses otherwise.

## taiHEN safety and user data

- Never silently edit `ur0:tai/config.txt` or change existing plugin entries.
- If a future module needs a taiHEN entry, show the exact proposed line and require explicit confirmation. Back up the original first.
- Never overwrite or delete files not declared as owned by the selected package/module.
- Preserve settings and archives during updates and module removal.
- Framework uninstall preserves Tai Saver archives by default.
- If a transaction fails, preserve enough information to diagnose and recover it.

## Current implementation status

The package format, package/module JSON schemas, first Tai Saver descriptor and installer behavior contract are defined. The installer UI, module-selection workflow, transaction engine, VitaSDK build, installable VPK and hardware validation are not yet complete. Do not treat the documented workflow as an already available on-device feature.
