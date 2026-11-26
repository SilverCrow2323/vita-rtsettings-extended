# Rt:Settings Extended

**A modular expansion framework for the native PlayStation Vita Settings application.**

Rt:Settings Extended is a project to extend the Vita's existing **Settings** application with additional, integrated configuration pages. Instead of creating one unrelated homebrew application for every utility, it provides a shared framework in Settings and lets each feature register its own page and services as an independent module.

The project is designed to grow over time: install only the sections you want, add new ones later, or install every available section in one go.

> **Project status:** active development. The host-side Tai Saver engine, boot lifecycle journal and expandable package/module specifications are present. The native Settings integration, Vita installer application, startup adapter, VPK packaging and on-device validation are still in development. This repository does not yet provide a finished, installable Vita release.

---

## Contents

- [Vision and goals](#vision-and-goals)
- [How it will look in Settings](#how-it-will-look-in-settings)
- [Expandable installer](#expandable-installer)
- [Module-based architecture](#module-based-architecture)
- [First module: Tai Saver](#first-module-tai-saver)
- [Planned framework behavior](#planned-framework-behavior)
- [Safety principles](#safety-principles)
- [Compatibility target](#compatibility-target)
- [Repository structure](#repository-structure)
- [Build and tests](#build-and-tests)
- [Development status and roadmap](#development-status-and-roadmap)
- [Documentation](#documentation)

## Vision and goals

The goal is to make the native Vita Settings application extensible with a consistent collection of additional pages.

The framework should:

- Add a dedicated **Rt:Settings Extended** entry inside the existing Settings application.
- Provide a root page that lists the installed feature modules.
- Give every module its own page, settings, version and service interface.
- Allow modules to be installed, updated, repaired, disabled and removed independently.
- Keep navigation and presentation consistent with the native Settings experience.
- Keep all user-facing interface text and project documentation in English.
- Perform long-running work in background workers so Settings remains responsive.
- Make operations reversible wherever possible, with clear progress, errors and recovery paths.
- Preserve user configuration and data when modules or the framework are updated or removed.

Rt:Settings Extended is **not** intended to become a separate SPDW control panel or a replacement for the native Settings application. It is an extension of the existing Settings experience.

## How it will look in Settings

The first target is a dedicated category containing the framework's installed sections:

```text
Settings
└── Rt:Settings Extended
    ├── Framework
    │   ├── Installed modules
    │   ├── Module status and versions
    │   └── Framework information
    └── Tai Saver
        ├── Automatic backup
        ├── Backup archive
        ├── Known-Good snapshots
        ├── Failed / KO snapshots
        ├── Compare and verify
        └── Restore
```

This is an illustrative first layout, not a claim that the native page has already been implemented.

As the project grows, additional feature modules will appear as their own entries under **Rt:Settings Extended**. A section that is not installed should not appear as an active page. The framework may show available or missing modules in its module manager, but it must not expose non-functional controls.

The initial integration will add our own category and pages. Placing individual controls directly inside Sony-owned Settings pages is a separate, compatibility-sensitive research task and is not required for the first release.

## Expandable installer

**One installer. Many independently installable Settings sections.**

The project will use a single expandable installer package. The framework is the shared base; each feature is delivered as a module with its own manifest, version, dependencies, payload and data-preservation rules.

### The user can choose how to install

The installer will support both installation styles:

1. **Install selected sections:** choose one or more modules individually. For example, install Tai Saver now and leave other sections unchecked.
2. **Install all available sections:** select every compatible module included in the current package with one action.
3. **Add sections later:** run the same installer again and add modules that were not selected during the first installation.
4. **Update independently:** update the framework or an individual module without reinstalling everything.
5. **Repair:** restore missing or modified files owned by the framework or a selected module.
6. **Disable or remove a module:** disable its Settings page/service, or remove its owned program files while preserving user data by default.

The shared framework is installed whenever it is needed by at least one module. It is not duplicated for every section. Required dependencies are selected automatically and cannot be unchecked. Incompatible modules or unresolved dependencies must be explained before installation.

The installer will include a module-selection screen with actions such as **Select All** and **Deselect All**, plus a review screen showing exactly what will be installed, updated or removed. “Install all” means all compatible modules actually present in that installer package; future modules become available when a newer package/catalog includes them.

### Example of how the installer grows

| Release | Available modules | User choice |
|---|---|---|
| First release | Tai Saver | Install Tai Saver, or the complete initial package |
| Later release | Tai Saver + new Settings modules | Keep the current setup, add only the new section, or install all |
| Future release | More modules | Choose any combination, or select all compatible modules |

You will not need a different installer for every new Settings section. A new module is added to the package/catalog with its descriptor and payload; the installer discovers it and presents it as another selectable option.

### Installer safety

The installer is designed around a reviewable, recoverable transaction:

- Validate package and module manifests.
- Check firmware and framework compatibility, dependencies and conflicts.
- Detect the current installation and identify only files owned by Rt:Settings Extended.
- Show a change plan before writing anything.
- Stage files and verify their hashes and available storage.
- Back up files that are about to be replaced and record each operation in a transaction journal.
- Commit the installed-module registry only after the payload operations succeed.
- Roll back on failure; if rollback is incomplete, preserve the journal and backups and report that recovery is required.

It must never overwrite or delete files that are not declared as owned by the package. User archives and settings are data, not disposable installer payload.

The package format and module descriptors are defined in `installer/`. The actual Vita installer UI and on-device transaction engine are not complete yet.

## Module-based architecture

The project is split into a shared framework and independent feature modules.

### Shared framework

The framework is responsible for:

- The Rt:Settings Extended root category and navigation.
- Module discovery and registration.
- Module status, versions and compatibility checks.
- Shared UI conventions and lifecycle management.
- Common configuration and service interfaces.
- Installer integration and recovery reporting.

### Feature modules

Each module is responsible for its own:

- Stable module ID and display metadata.
- Settings page and page-specific behavior.
- Settings schema and version migrations.
- Service calls and feature logic.
- Declared dependencies and conflicts.
- Exact list of files it owns.
- User-data locations and preservation rules.

A module must not silently edit another module's files or configuration. The framework owns navigation and module lifecycle; the module owns its feature behavior.

### Adding a new module

The intended development flow is:

1. Assign a stable module ID.
2. Create its `module.json` descriptor.
3. Implement its page and service using framework interfaces.
4. Define its settings schema, dependencies and compatibility.
5. Declare every package-owned path and every user-data path.
6. Add tests for installation, update, failure recovery and removal.
7. Add it to the package manifest.

The same installer then offers it as a selectable section. See [Expandable Installer](docs/EXPANDABLE_INSTALLER.md).

## First module: Tai Saver

**Tai Saver** is the first planned user-facing feature. It protects the contents of `ur0:tai` by creating verifiable snapshots and providing safe archive management.

### Planned features

- Automatic backup during startup when enabled.
- Manual backup on demand.
- Configurable automatic retention: 2, 3, 4, 5 or a custom number of snapshots.
- Separate archive classes:
  - **Automatic** — routine snapshots managed by the retention policy.
  - **Known-Good** — snapshots explicitly kept by the user as known working configurations.
  - **Failed / KO** — snapshots retained for diagnostics when a configuration is known to be broken.
  - **Pre-Restore** — safety snapshot created before restoring another archive.
- SHA-256 hashes for files and archive integrity checks.
- File-level comparison between snapshots.
- Archive verification before restore.
- Protected restore with a Pre-Restore snapshot and rollback path.
- Boot lifecycle journal to record backup status, interruptions and failures.
- Staging recovery so interrupted, incomplete work is not mistaken for a valid archive.

Automatic retention must apply only to eligible Automatic snapshots. It must never silently delete Known-Good, Failed / KO or Pre-Restore material.

### Tai Saver storage

The initial data location is:

```text
ur0:data/RtSettingsExtended/TaiSaver/
├── settings/
├── Automatic/
├── Known-Good/
├── Failed/
├── Pre-Restore/
├── staging/
├── recovery/
└── boot/
    ├── boot-state.ini
    └── events/
```

The exact on-device layout may evolve as implementation progresses. The important rule is that user archives and settings remain separate from replaceable program files.

### Startup backup behavior

When automatic backup is enabled, the eventual startup component should wait until storage is mounted and `ur0:tai` is accessible, then create and verify a snapshot in the background. It must not block the Settings interface or rewrite the live taiHEN configuration.

A boot journal records states such as started, waiting for storage, backup running, verified, skipped, failed or interrupted. A successful backup does not automatically mean that the snapshot is Known-Good.

The host-testable journal exists; the actual Vita startup adapter and target-device timing validation remain pending.

## Planned framework behavior

Beyond Tai Saver, the framework is intended to support additional Settings sections as independent modules. The exact feature list will be decided and implemented module by module; a future idea is not considered an available feature until its code, descriptor and tests exist.

The framework should provide a single place to:

- See which modules are installed and their versions.
- Check module status and compatibility.
- Access each installed module's Settings page.
- Understand when a module is unavailable, disabled or requires an update.
- Manage the framework without disturbing unrelated Sony Settings.
- Preserve module data across updates and optional removal.

## Safety principles

Safety is a core design requirement, especially because Tai Saver interacts with taiHEN configuration.

- **No silent taiHEN edits.** Installation, update and removal must not silently rewrite `ur0:tai/config.txt` or change existing plugin entries.
- **No unowned file operations.** The installer may touch only paths explicitly owned by the framework or selected module.
- **Verify before restore.** An archive must pass a fresh integrity check before it can be restored.
- **Pre-Restore is mandatory.** Restoring a snapshot first creates a safety copy of the current configuration.
- **Preserve user data.** Updates and module removal preserve user archives and settings by default.
- **Fail closed.** Unsupported firmware, malformed manifests or incomplete transactions must not result in a partial active installation.
- **Keep evidence.** Failed or interrupted transactions retain their journal and recovery material.
- **Do not block the UI.** Copying, hashing, verification and restore operations run outside the Settings UI thread.
- **Do not claim success prematurely.** The interface reports success only after the operation and its verification have completed.

If a future installation method requires a taiHEN plugin entry, it must show the exact proposed change, explain why it is needed, back up the original and request explicit user confirmation.

## Compatibility target

The first validation target is:

- **Console:** PlayStation Vita 1000 (OLED)
- **Firmware:** 3.65
- **HENkaku Ensō:** installed
- **Storage:** SD2Vita with YAMT, alongside the existing Sony memory card

Other Vita models, firmware versions, storage setups and taiHEN configurations require separate compatibility testing. The target above is a development priority, not a claim that the current code has already been tested on hardware.

## Repository structure

```text
.
├── .github/workflows/       # Continuous integration
├── docs/                    # Architecture and implementation contracts
├── include/                 # Public C interfaces
├── installer/               # Package and module manifests, schemas
│   └── modules/tai-saver/   # First module descriptor
├── settings-ui/             # Settings page specifications
├── source/
│   ├── tai-saver/            # Snapshot engine and SHA-256
│   └── boot-monitor/         # Persistent boot lifecycle journal
├── tai-saver/                # Feature specification and JSON schemas
├── tests/                    # Host-side regression tests
├── Makefile                  # Host build and tests
└── Makefile.vita             # VitaSDK build recipe
```

## Build and tests

The current C core can be built and tested on a desktop host:

```sh
make
make test
make sanitize
```

A VitaSDK static-library build recipe is provided in `Makefile.vita`. It is not yet evidence of a successful Vita cross-build. The VitaSDK toolchain build, native Settings registration, boot adapter, installer VPK and physical-device validation are separate milestones.

A successful host CI run proves only the host tests covered by that workflow. It does not prove that the plugin, Settings hooks or installer work on a physical Vita.

## Development status and roadmap

| Component | Status |
|---|---|
| Tai Saver snapshot engine and SHA-256 manifests | Implemented; host-tested |
| Snapshot verification and comparison | Implemented; host-tested |
| Archive classification and Automatic-only retention | Implemented; host-tested |
| Guarded restore with Pre-Restore safety snapshot | Implemented; host-tested |
| Boot lifecycle journal and interrupted-run detection | Implemented; host-tested |
| Expandable package/module format and JSON schemas | Defined |
| First Tai Saver module descriptor | Defined |
| Installer selection and transaction/rollback contract | Defined; Vita UI/engine pending |
| Staging recovery | Specified; implementation pending |
| VitaSDK cross-build validation | Pending |
| Vita startup adapter and real boot integration | Pending |
| Native Rt:Settings Extended category and module pages | Pending |
| Installer VPK and on-device install/update/remove | Pending |
| Physical Vita testing | Pending |

### Roadmap

1. Complete storage safety and interrupted-staging recovery.
2. Implement and test the installer transaction engine on the host.
3. Validate the code with the VitaSDK toolchain.
4. Implement the Vita installer UI, including individual module selection and **Install All**.
5. Implement the shared framework and native Settings category.
6. Integrate Tai Saver as the first working Settings module.
7. Implement and validate the startup backup adapter.
8. Build VPK packages and test installation, updates, rollback and removal on the target Vita.
9. Add future Settings modules without replacing the installer architecture.

## Documentation

- [Architecture](docs/ARCHITECTURE.md)
- [Tai Saver Core](docs/TAI_SAVER_CORE.md)
- [Boot Integration](docs/BOOT_INTEGRATION.md)
- [Native Settings Integration](docs/SETTINGS_NATIVE_INTEGRATION.md)
- [Expandable Installer and Module System](docs/EXPANDABLE_INSTALLER.md)
- [Tai Saver page specification](settings-ui/TAI_SAVER_PAGE.md)
- [Installer package and module format](installer/README.md)
- [Tai Saver feature documentation](tai-saver/README.md)

---

**The long-term objective:** one stable Rt:Settings Extended framework inside Vita Settings, one expandable installer, and a growing collection of optional sections that users can install individually or all together—without sacrificing the safety of their existing system configuration or user data.
