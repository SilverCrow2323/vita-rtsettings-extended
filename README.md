# Rt:Settings Extended

**The definitive extended Settings environment for the PlayStation Vita.**

Rt:Settings Extended aims to turn the native Vita Settings application into a complete, expandable control centre: Sony system options, taiHEN plugin controls, connectivity services, USB/FTP tools, streaming options, storage management, diagnostics and safety utilities—organized in one consistent interface.

This is not intended to be another standalone homebrew launcher or a small collection of unrelated tweaks. The goal is to make the Settings application the central place where users can inspect, configure and manage the capabilities installed on their Vita.

> **Development status:** active development. The Tai Saver host-side core and boot lifecycle journal have been implemented and host-tested. The complete Settings interface, plugin adapters, USB/FTP/streaming services, expandable installer UI, Vita startup integration, VPK packaging and physical-device validation are still under development. The feature catalogue below describes the project target, not a claim that every feature is already available.

---

## Contents

- [The overall vision](#the-overall-vision)
- [The Settings experience](#the-settings-experience)
- [Native integration and dedicated Settings categories](#native-integration-and-dedicated-settings-categories)
- [Preferences and menu customization](#preferences-and-menu-customization)
- [System Tools and application shortcuts](#system-tools-and-application-shortcuts)
- [Feature catalogue](#feature-catalogue)
- [Plugin Control Centre](#plugin-control-centre)
- [Connectivity, USB, FTP and streaming](#connectivity-usb-ftp-and-streaming)
- [Expandable installer and modular sections](#expandable-installer-and-modular-sections)
- [Module architecture](#module-architecture)
- [First implemented core: Tai Saver](#first-implemented-core-tai-saver)
- [Safety, compatibility and reliability](#safety-compatibility-and-reliability)
- [Initial hardware target](#initial-hardware-target)
- [Repository structure](#repository-structure)
- [Build and tests](#build-and-tests)
- [Development status and roadmap](#development-status-and-roadmap)
- [Documentation](#documentation)

## The overall vision

The project aims to provide one coherent Settings environment for the whole console.

The extended Settings experience should bring together:

- Native system settings and additional controls that are safe to expose.
- A central manager for installed taiHEN plugins and their configurable options.
- Persistent USB connection and FTP service preferences.
- USB streaming controls and connection status.
- Storage, SD2Vita and memory-card information and tools.
- Startup behavior, backup, recovery and diagnostics.
- Network, power, display, audio, input and system information extensions.
- An expandable module installer so users can choose exactly which sections to install.

The design principle is simple: **one Settings application, one consistent interface, many integrated capabilities.**

We will build this in layers. Sony-owned Settings pages will not be patched indiscriminately. The first integration target is our own Rt:Settings Extended category, with clear navigation to each installed section. Extending or injecting controls into Sony-owned pages is a separate reverse-engineering and compatibility task.

## The Settings experience

The target is a native-feeling Settings experience: existing Sony categories remain familiar, compatible extensions are integrated in place where technically safe, and new Rt categories cover features that have no sensible Sony destination.

The following is the logical information architecture. Exact placement and labels may vary slightly after firmware/API research:

```text
Settings
├── Network
│   └── Supported connectivity extensions
├── Sound & Display
│   └── Supported display, graphics and audio extensions
├── System
│   └── Supported system and startup extensions
├── Advanced Options                         [new Rt category]
│   ├── Connectivity Options
│   ├── Plugin Feature Settings
│   ├── Runtime Configuration
│   ├── Boot Configuration
│   ├── Service Management
│   ├── Compatibility Options
│   └── Experimental Features
├── System Tools                             [new Rt category]
│   ├── VitaShell                            [only when detected]
│   ├── File Explorer                        [only when detected]
│   ├── Application Manager                  [only when detected]
│   ├── System Monitor                       [only when detected]
│   └── Other registered tools
├── Preferences                              [new Rt category]
│   ├── Menu Layout
│   ├── Visibility
│   ├── Ordering
│   ├── Profiles
│   └── Reset & Backup
└── Rt:Settings Extended                     [framework and module hub]
    ├── Overview
    ├── Framework & Modules
    ├── Connectivity
    ├── Plugin Control Centre
    ├── Storage & Devices
    ├── System & Startup
    ├── Display & Audio
    ├── Power & Performance
    ├── Tai Saver
    └── System Information & Diagnostics
```

The diagram is a target layout, not a claim that every entry is already implemented or that every Sony page can be extended without reverse engineering. The framework should prefer placing a control in its natural existing Sony category when a stable, compatible integration point exists. Otherwise, it should create a clearly named Rt category rather than force an awkward placement.

Sections and entries appear only when their module, application or backend is detected and compatible. The interface must distinguish **Available**, **Installed**, **Hidden**, **Disabled**, **Incompatible**, **Service unavailable** and **Requires reboot** states. Hidden is a presentation preference; Disabled is an operational state.

Every page should use consistent controls, explanations, confirmation prompts, status indicators and error reporting. Long-running operations must run outside the Settings UI thread.

## Native integration and dedicated Settings categories

The project uses two complementary integration strategies.

### Integrate into existing Sony categories

Where the firmware exposes a reliable and maintainable extension point, Rt controls should appear beside the Sony settings they relate to. Examples include network-related options under Network, display and graphics options under Sound & Display, and supported startup/system options under System.

This must be implemented conservatively. We will not blindly patch Sony resources, overwrite system files or assume that a menu hook is stable across firmware versions. Every integration point needs compatibility checks and a fallback.

### Add professional Rt categories when no natural Sony location exists

Some functions are broader utilities or advanced controls. They deserve their own entries in the main Settings list:

- **Advanced Options** — specialist configuration that does not belong in a standard Sony page.
- **System Tools** — launch points for useful installed system/homebrew utilities.
- **Preferences** — customization of the Settings navigation and visibility.
- **Rt:Settings Extended** — framework status, installed modules, diagnostics and the complete extension hub.

These entries must follow the visual and interaction conventions of the native Settings application: restrained typography, consistent spacing, familiar list navigation, clear hierarchy, concise descriptions and predictable back navigation. Avoid flashy launcher-style tiles, unrelated visual themes or a separate homebrew dashboard aesthetic.

## Preferences and menu customization

**Preferences** gives the user control over the organization of the Settings application. It is a first-class module and one of the defining features of Rt:Settings Extended.

The user should be able to customize both the main Settings list and the contents of supported categories.

### Main menu layout

- Reorder Rt-created categories in the Settings root.
- Move eligible Rt entries between supported Rt categories.
- Choose whether supported Rt categories appear in the main Settings list or under the Rt hub.
- Hide or show individual Rt-created categories.
- Restore the default layout at any time.

### Individual menu entries

- Hide or show individual settings rows and submenus.
- Reorder entries inside supported Rt pages.
- Move eligible Rt-owned entries between compatible Rt pages.
- Restore an individual page to its default ordering.
- Reset all menu visibility and ordering preferences.

### Sony menu boundaries

Sony-owned entries may be hidden, moved or reordered only when the firmware integration layer explicitly supports that operation and can restore the original state safely. The project must not promise arbitrary rearrangement of Sony's internal menus. If an entry cannot be modified safely, it remains in its original position and Preferences explains why.

### Profiles and persistence

- Save multiple layouts, for example **Default**, **Minimal**, **Advanced** and **Troubleshooting**.
- Switch layouts without changing plugin activation, service state or application installation.
- Persist preferences across reboots.
- Export/import a validated preferences backup when that feature is implemented.
- Provide **Reset to Defaults** and per-category reset actions.

### Critical distinction: visibility is not functionality

Hiding a menu item must never uninstall its module, disable its plugin, stop a service, change a system setting or delete user data. Showing a hidden item must not silently enable it. Preferences controls presentation only; operational settings remain owned by their respective modules.

The menu registry should store stable entry identifiers rather than relying on translated display labels or list positions. Unknown or removed identifiers should be ignored safely, and invalid layouts should fall back to a valid default. Layout changes should be validated and committed atomically so an interrupted write cannot leave Settings with an unusable menu.

## System Tools and application shortcuts

**System Tools** is a curated application-launch section inside Settings—not a replacement for the LiveArea and not an attempt to duplicate every homebrew bubble.

It provides convenient access to useful utilities already installed on the console. Initial candidates include:

- **VitaShell** — file management and supported USB/FTP workflows.
- **File Explorer** — a dedicated file browser, if a compatible application is installed.
- **Application Manager** — application and homebrew management, if available.
- **System Monitor** — system information and monitoring tools, if available.
- Additional utilities registered by compatible modules.

### Detection and launch behavior

- Detect applications using reliable identifiers, registered launch metadata or verified installation manifests.
- Show only applications that are actually installed and recognized.
- Display a clear unavailable state if a previously registered application has been removed.
- Launch the selected application directly only where the Vita's supported launch mechanism permits it reliably.
- If direct launch is not supported, provide a truthful fallback, such as opening the application's LiveArea or explaining the limitation.
- Do not install duplicate copies of VitaShell or other utilities merely to populate this list.
- Do not delete, modify or take ownership of third-party application data.
- Allow compatible modules to register additional tools through a documented manifest.

System Tools entries are shortcuts, not plugin toggles. Removing a shortcut from the list must not remove the application itself. Preferences may hide or reorder these shortcuts independently.


## Feature catalogue

The following catalogue describes the intended scope. It will be delivered incrementally as tested modules and adapters.

### Connectivity

- **USB Connection — Always On preference:** an option to keep the USB connection service available/automatically enabled where the Vita's USB mode and installed system components permit it. The page will show connection state and explain when a cable, host or reboot is required.
- **FTP Server — Always On:** automatically start the FTP service when its supported backend is available, with configurable port, access state, startup behavior and clear running/stopped status.
- Manual start/stop controls for supported services.
- Startup behavior and service health indicators.
- Explicit warnings when a service is unavailable because its required plugin/application is not installed.

“Always On” is a persistent preference, not a promise that a physical USB link can exist without a cable or that an FTP server can operate when the console is powered off. Services must start only when their backend and system state allow it.

### USB Streaming

A dedicated page for supported USB streaming backends:

- Enable/disable USB streaming.
- Show backend/plugin detection and compatibility.
- Display connection and streaming state.
- Configure supported backend options, such as video mode or transport settings, only when exposed by that backend.
- Start/stop streaming and show errors.
- Explain host-side requirements and reboot requirements.

USB streaming must be treated separately from ordinary USB connection mode and FTP. Their drivers and plugin dependencies can conflict or be mutually exclusive on some configurations; the framework must detect known conflicts and never activate incompatible modes together without an explicit, informed user decision.

### Plugin Control Centre

A central place to inspect and manage configurable plugins:

- Scan known taiHEN plugin locations and identify installed plugin entries.
- Group plugins by category and load location where possible.
- Show plugin name, path, version and detected status when metadata is available.
- Expose settings for plugins that provide a supported configuration adapter.
- Provide per-plugin enable/disable controls only where the framework can safely manage the relevant configuration.
- Offer profiles, such as a daily-use profile, streaming profile or troubleshooting profile, once profile switching is implemented safely.
- Validate configuration changes, show a preview and preserve a restorable backup.
- Detect missing files, duplicate entries and known conflicts where reliable rules exist.
- Keep diagnostic information for unsupported or unrecognized plugins.

**Important technical limit:** there is no universal safe way to invent settings for an arbitrary plugin. A plugin's adjustable options can be exposed only if its configuration format, API or a dedicated adapter is understood. Unknown plugins will still be listed when detectable, but will not receive fabricated sliders or switches. The framework must never guess undocumented values or silently rewrite plugin configuration.

### Storage & Devices

Planned capabilities include:

- Internal memory card and SD2Vita status.
- Mounted storage devices, mount points and free-space information.
- Storage health and configuration diagnostics where the platform exposes reliable data.
- Safe access to Tai Saver archives and recovery data.
- Device and USB connection information.
- Warnings before operations that may affect storage availability.

### System & Startup

- Startup service status and lifecycle information.
- Automatic service preferences.
- Boot journal and startup diagnostics.
- Module startup dependencies and ordering.
- Recovery status after interrupted operations.
- Clear reboot-required indicators.

### Display & Audio

An extended home for supported display and audio options, including plugin-backed enhancements. Controls will be available only where a documented system API or supported plugin adapter exists.

### Power & Performance

A home for supported power, performance and monitoring extensions. Plugin-specific overclocking or performance controls must respect hardware limits, backend capabilities and safe defaults. The framework must not expose arbitrary unsafe values.

### Tai Saver

Backup, verification, comparison and recovery for `ur0:tai`. See [First implemented core: Tai Saver](#first-implemented-core-tai-saver).

### System Information & Diagnostics

- Firmware and system component information.
- Installed framework and module versions.
- Detected plugin/backend inventory.
- Service status and diagnostic reports.
- Compatibility checks and recovery guidance.

The catalogue is intentionally broad. A feature becomes an installable module only after its implementation, dependencies, UI, safety rules and tests are ready.

## Plugin Control Centre

The Plugin Control Centre is a core pillar of the project, not an afterthought.

The framework will use a registry of plugin adapters. Each adapter describes:

- Which plugin or plugin family it supports.
- How to detect the installed version and active state.
- Which settings it can safely read and change.
- Valid values and constraints for each setting.
- Whether a change takes effect immediately or requires a reboot.
- How to validate, back up and restore its configuration.
- Known conflicts with other plugins or services.

The UI can then generate the appropriate controls—switches, selectors, numeric fields or sliders—from validated metadata. A slider is shown only when the adapter defines a meaningful numeric range and safe step size.

For plugins without an adapter, the manager provides inventory and diagnostic information rather than pretending to control them. New adapters can be distributed as framework updates or compatible modules.

## Connectivity, USB, FTP and streaming

These services need explicit lifecycle management rather than isolated toggles.

Each service should expose a common status model:

- Not installed
- Available
- Stopped
- Starting
- Running
- Stopping
- Error
- Blocked by conflict
- Requires reboot

Persistent preferences such as **FTP Server: Start automatically** or **USB service: Keep enabled when available** are stored separately from live service state. If startup fails, the UI must show the failure and preserve the user's preference without falsely displaying the service as running.

The service manager must coordinate shared resources and known conflicts. It must not start two mutually exclusive USB backends simultaneously.

## Expandable installer and modular sections

**One installer. Install one section at a time—or install everything available.**

The framework is shared; each Settings section is a separate module. The installer will offer:

1. **Install Selected** — choose individual modules.
2. **Install All** — select every compatible module included in the package.
3. **Add Later** — rerun the installer to add modules without reinstalling existing ones.
4. **Update** — update the framework or selected modules independently.
5. **Repair** — restore missing or modified files owned by the selected components.
6. **Disable / Enable** — change module availability without deleting its user data.
7. **Remove** — remove only the selected module's owned program files, preserving user data by default.

The shared framework is installed once and reused by every module. Required dependencies are selected automatically. The installer must check firmware compatibility, module dependencies, known conflicts and available storage before changing the console.

“Install All” means all compatible modules included in that installer package. New modules become available through a later package/catalog update; users can then add just the new section or choose Install All again.

### Installer transaction safety

- Validate the package and module manifests.
- Detect the current installation and installed module versions.
- Present an exact change plan before installation.
- Stage payload files and verify their hashes.
- Back up replaced framework/module-owned files.
- Journal each transaction step.
- Commit the installed-module registry only after the payload is successfully installed.
- Roll back on failure and preserve recovery material if rollback is incomplete.
- Never delete files outside the declared ownership list.
- Preserve user settings, plugin configurations and Tai Saver archives unless the user explicitly authorizes a data deletion.

See [Expandable Installer and Module System](docs/EXPANDABLE_INSTALLER.md).

## Module architecture

The project consists of a shared framework, service backends, plugin adapters and independent feature modules.

### Shared framework

- Native Settings entry and navigation, including safe Sony-page integration and dedicated Rt categories.
- Preferences engine for stable-ID menu ordering, visibility, profiles and reset.
- System Tools application registry, detection and launch adapters.
- Module discovery and registration.
- Service manager and lifecycle.
- Plugin adapter registry.
- Shared configuration and UI conventions.
- Compatibility and conflict detection.
- Installer integration and transaction recovery.
- Diagnostics and status reporting.

### Modules and adapters

A module owns its page, settings schema, service integration, dependencies and declared files. Plugin adapters provide narrowly scoped, validated access to plugin-specific options.

This distinction matters: a Settings page can group controls from several plugins, but the code that understands and changes each plugin's settings belongs in its adapter. That keeps the interface unified without coupling every page to undocumented plugin internals.

## First implemented core: Tai Saver

Tai Saver is the first feature engine being developed. It protects the contents of `ur0:tai` using verifiable snapshots and safe archive management.

Planned user-facing functions:

- Automatic backup at startup when enabled.
- Manual backup.
- Configurable automatic retention: 2, 3, 4, 5 or a custom number.
- Separate **Automatic**, **Known-Good**, **Failed / KO** and **Pre-Restore** archives.
- SHA-256 integrity verification.
- File-level snapshot comparison.
- Verified restore with a mandatory Pre-Restore safety snapshot.
- Boot lifecycle journal for success, skipped, failed and interrupted runs.
- Staging recovery so incomplete snapshots are never treated as valid archives.

Automatic retention applies only to eligible Automatic snapshots. It must never silently delete Known-Good, Failed / KO or Pre-Restore archives.

Initial data location:

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

The snapshot engine and boot journal have host-side implementations and tests. The Vita startup adapter and complete Settings page are still pending.

## Safety, compatibility and reliability

- No silent edits to `ur0:tai/config.txt` or existing plugin entries.
- No arbitrary controls for unknown plugins.
- No writes outside explicitly owned paths.
- No conflicting USB/streaming services started together.
- No restore without archive verification and a Pre-Restore snapshot.
- No automatic deletion of user archives or configuration.
- No false “running” state when a service failed to start.
- No blocking file operations on the Settings UI thread.
- No unsupported firmware or hardware assumptions presented as verified compatibility.
- Every risky operation has a preview, confirmation, journal and recovery path appropriate to its impact.

Where a plugin change requires a reboot, the UI must say so before applying it. If a service cannot be safely stopped or restarted dynamically, the interface must not pretend otherwise.

## Initial hardware target

First validation target:

- PlayStation Vita 1000 (OLED)
- Firmware 3.65
- HENkaku Ensō
- SD2Vita with YAMT
- Existing Sony memory card

Other models, firmwares and storage configurations require independent testing. This is the first development target, not a claim of completed hardware validation.

## Repository structure

```text
.
├── .github/workflows/       # Continuous integration
├── docs/                    # Architecture and integration contracts
├── include/                 # Public C interfaces
├── installer/               # Package/module manifests and schemas
├── settings-ui/             # Settings page specifications
├── source/
│   ├── tai-saver/           # Snapshot engine and SHA-256
│   └── boot-monitor/         # Persistent boot lifecycle journal
├── tai-saver/               # Feature specifications and schemas
├── tests/                   # Host-side regression tests
├── Makefile                 # Host build and tests
└── Makefile.vita            # VitaSDK build recipe
```

The repository layout will expand as the service manager, plugin adapters, Settings UI and installer are implemented.

## Build and tests

The current host-side C core can be built and tested with:

```sh
make
make test
make sanitize
```

A VitaSDK build recipe exists, but it is not yet evidence of a successful Vita cross-build. The native Settings integration, service backends, plugin adapters, installer VPK and physical-device tests are separate milestones.

A passing host CI run validates only the tests actually executed by that workflow. It does not prove that native Settings hooks or device-level USB/FTP/streaming behavior work on hardware.

## Development status and roadmap

| Area | Status |
|---|---|
| Tai Saver snapshot engine and SHA-256 manifests | Implemented; host-tested |
| Snapshot verification, comparison and archive retention | Implemented; host-tested |
| Guarded restore with Pre-Restore safety snapshot | Implemented; host-tested |
| Boot lifecycle journal | Implemented; host-tested |
| Expandable installer and module architecture | Specified |
| Preferences menu customization (ordering, visibility, profiles) | Project target; implementation pending |
| System Tools application registry and launch shortcuts | Project target; implementation pending |
| Native Sony menu injection / extension points | Research and compatibility work pending |
| Plugin Control Centre and adapter contract | Project target; implementation pending |
| USB persistent connection controls | Project target; implementation pending |
| FTP always-on service integration | Project target; implementation pending |
| USB streaming integration | Project target; backend compatibility work pending |
| Storage, system, display, audio and power sections | Project target; implementation pending |
| Staging recovery | Specified; implementation pending |
| Installer transaction engine and VPK UI | Pending |
| VitaSDK cross-build validation | Pending |
| Native Settings category and pages | Pending |
| Vita startup/service integration | Pending |
| Physical Vita validation | Pending |

### Roadmap

1. Finish storage safety and interrupted-staging recovery.
2. Define the Preferences menu registry, persistence format and safe layout transaction model.
3. Define System Tools application detection and launch-adapter contracts.
4. Implement the shared service and plugin-adapter contracts.
5. Implement the installer transaction engine and module selection UI.
6. Validate the code with the VitaSDK toolchain.
7. Build the Rt:Settings Extended root category and navigation.
8. Integrate Tai Saver as the first working module.
9. Add Connectivity: USB persistent preference and FTP auto-start, with real service-state reporting.
10. Add USB streaming through a tested backend and conflict-aware service manager.
11. Build the Plugin Control Centre and adapters for supported plugins.
12. Add storage, system, display/audio, power and diagnostics modules incrementally.
13. Implement Sony-page integrations where verified extension points exist.
14. Test installation, update, rollback, service startup and removal on the target Vita.

## Documentation

- [Expandable Installer and Module System](docs/EXPANDABLE_INSTALLER.md)
- [Boot Integration](docs/BOOT_INTEGRATION.md)
- [Native Settings Integration](docs/SETTINGS_NATIVE_INTEGRATION.md)
- [Tai Saver Core](docs/TAI_SAVER_CORE.md)
- [Tai Saver page specification](settings-ui/TAI_SAVER_PAGE.md)
- [Installer package and module format](installer/README.md)
- [Tai Saver feature documentation](tai-saver/README.md)

---

**Long-term objective:** make Rt:Settings Extended the complete, modular Settings environment for the Vita—covering connectivity, USB, FTP, streaming, plugin configuration, storage, startup, system controls, backup and diagnostics—with one expandable installer and no compromise on safety.
