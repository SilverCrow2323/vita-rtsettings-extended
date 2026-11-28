# Expandable Installer and Module System

## Product goal

Rt:Settings Extended uses one expandable installer for the shared framework, feature modules and compatible plugin/service adapters. The installer is designed to grow as new Settings sections are added.

The user can install one section at a time, choose any combination, or install all compatible modules included in the current package.

## Installation choices

- **Install Selected:** install only the checked modules.
- **Select All / Install All:** select every compatible optional module in this package.
- **Deselect All:** clear optional selections.
- **Add Later:** add modules without reinstalling existing modules.
- **Update:** update the shared framework or selected modules independently.
- **Repair:** restore missing or modified files owned by selected components.
- **Enable / Disable:** change module availability while preserving its data.
- **Remove:** remove a module's owned program files; preserve user data by default.

The shared framework is installed once and reused. Required dependencies are selected automatically and cannot be unchecked. Incompatible modules and unresolved dependencies are blocked with an explanation.

“Install All” means all compatible modules actually included in the package. When a later package adds modules, users can add only those new modules or choose Install All again.

## Planned module catalogue

The package format is designed to accommodate the full Rt:Settings Extended feature scope. Initial planned module groups include:

| Module group | Planned sections |
|---|---|
| Connectivity | USB connection preference, FTP server auto-start, service status |
| USB Streaming | Backend detection, enable/disable, streaming status and supported options |
| Plugin Control Centre | Plugin inventory, supported plugin settings, profiles and diagnostics |
| Storage & Devices | SD2Vita, memory card, mount status, free space and diagnostics |
| System & Startup | Startup services, boot status, module lifecycle and recovery |
| Display & Audio | Supported system and plugin-backed display/audio controls |
| Power & Performance | Supported performance controls and monitoring |
| Tai Saver | Automatic backups, retention, archive verification, comparison and restore |
| System Information | Firmware, module versions, compatibility and diagnostic reports |

The table is a roadmap, not a declaration that these modules are already implemented.

## Plugin adapter packages

Plugin settings are not safely interchangeable. An adapter must declare the exact plugin identity/version range and the settings it supports. It defines value types, allowed ranges, validation, apply behavior, reboot requirements and backup/restore behavior.

The installer must not install a generic adapter that guesses unknown plugin configuration. An unrecognized plugin may be inventoried, but controls appear only when a compatible adapter is available.

Adapters may be bundled with the framework or shipped as independently selectable compatible modules. Their dependencies and conflicts are declared in their manifests.

## Package components

- **Package manifest:** product identity, release version, supported framework versions and list of available modules.
- **Module manifest:** stable ID, version, dependencies, conflicts, firmware compatibility, Settings page entry, payload ownership and data-preservation rules.
- **Plugin adapter manifest:** supported plugin identity/version and validated settings schema.
- **Installer UI:** selection, Select All, compatibility warnings, change preview, progress and result report.
- **Transaction engine:** staging, hash verification, journaling, commit and rollback.
- **Installed registry:** installed/enabled state, versions and exact files owned by each component.

## Installation transaction

1. Validate the package, manifests and payload hashes.
2. Detect existing framework/modules and their versions.
3. Check firmware, dependencies, conflicts and free storage.
4. Present module selection and automatically select required dependencies.
5. Resolve the requested selection into an exact install/update plan.
6. Show changes, storage requirements, service impacts and reboot requirements.
7. Stage payloads away from live destinations and verify them.
8. Journal the transaction and back up owned files that will be replaced.
9. Commit the selected framework, modules and adapters.
10. Write the installed registry only after all required operations succeed.
11. Report success or failure; preserve journal and recovery data if rollback is incomplete.

No partial module installation may be reported as successful.

## Update, repair, disable and removal

Updates replace only files owned by the selected component. Unchanged files should remain untouched. Repair restores missing or modified owned files.

Disabling a module must unregister its Settings page and stop its managed service where safe; it must not delete user configuration. Removal deletes only declared program files and preserves user data by default. Removing the shared framework requires dependent modules to be handled first.

## Service and plugin configuration safety

The installer and module manager must:

- Never silently rewrite `ur0:tai/config.txt` or existing plugin entries.
- Never alter an unknown plugin's settings.
- Show the exact proposed configuration change and require confirmation where a taiHEN entry is needed.
- Preserve plugin configuration and user data on updates.
- Detect known service conflicts, including mutually exclusive USB/streaming backends.
- Clearly indicate when a service or configuration change requires a reboot.

## Adding a new module

1. Assign a stable lowercase module ID.
2. Add its descriptor and payload.
3. Implement its Settings page and service/adapter integration.
4. Declare compatibility, dependencies, conflicts and owned paths.
5. Define settings migration and user-data preservation.
6. Add tests for install, update, failure recovery, disable and removal.
7. Include it in the package manifest.

The installer then offers it automatically as another selectable section. The installer does not need a bespoke rewrite for every new module, provided the module follows the supported contracts.

## Current implementation status

The expandable module concept and documentation are defined, and the repository already contains the first Tai Saver descriptor/schema work. The host-reference transaction engine now implements install, update and removal plans with staged payload verification, owned-file backups, a transaction journal, registry-last commit and rollback. Its regression suite covers install/update/rollback/remove, unowned-file protection, hash mismatch, traversal, symlink destinations and injected registry failure. This engine is filesystem-rooted for host verification; it is not the Vita runtime backend. The native Vita filesystem adapter, installer UI, service manager, plugin adapter registry, VitaSDK build, installable VPK and on-device validation remain pending.


## Host transaction serialization

The host reference engine takes an exclusive advisory lock in the target's `.rtse/installer.lock` before reading the registry or applying/rolling back a transaction. The lock is opened without following symlinks where the platform supports `O_NOFOLLOW`, verified as a regular file, and held across the entire transaction. This prevents two installer processes from both passing the same stale registry check and interleaving file/registry commits. The Vita runtime backend must provide an equivalent platform-native lock; this Python reference implementation is not itself the Vita installer.
