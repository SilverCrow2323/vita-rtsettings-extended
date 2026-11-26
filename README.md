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
- [Preferences registry implementation](#preferences-registry-implementation)
- [User-facing Settings metadata](#user-facing-settings-metadata)
- [Settings screen and renderer adapter](#settings-screen-and-renderer-adapter)
- [Layout Profile Manager](#layout-profile-manager)
- [Safe module controls](#safe-module-controls)
- [Connectivity service lifecycle](#connectivity-service-lifecycle)
- [Plugin Control Centre model](#plugin-control-centre-model)
- [Storage & Devices snapshot model](#storage--devices-snapshot-model)
- [Vita plugin research](#vita-plugin-research)
- [Curated Vita plugin catalogue](#curated-vita-plugin-catalogue)
- [Plugin inventory and adapter registry](#plugin-inventory-and-adapter-registry)
- [Plugin adapter framework](#plugin-adapter-framework)
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
├── Appearance & Customization               [new Rt category]
│   ├── Boot Animation / Boot Splash
│   ├── Boot Sound
│   ├── Startup Warning
│   ├── Interface & Asset Manager
│   └── Quick Menu Customization
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

The menu registry should store stable entry identifiers rather than relying on translated display labels or list positions. Unknown or removed identifiers should be ignored safely, and invalid layouts should fall back to a valid default. Layout changes should be validated and committed atomically so an interrupted write cannot leave Settings with an unusable menu. The host-testable registry, named Layout Profiles core, Preferences navigation model and renderer-neutral Settings screen adapter are implemented; see [Preferences Registry](docs/PREFERENCES_REGISTRY.md), [Layout Profiles](docs/LAYOUT_PROFILES.md), [Preferences UI Model](settings-ui/PREFERENCES_UI_MODEL.md) and [Settings Screen Adapter](settings-ui/SETTINGS_SCREEN_ADAPTER.md). The Layout Profile Manager controller and safe module-control descriptor contract are also implemented. A conflict-aware service lifecycle manager now provides truthful status refresh, start/stop and auto-start operations, refusing to start mutually exclusive services when state is stale or conflicting. The Vita-native renderer, verified Sony Settings integration and Vita-specific persistence adapter remain pending.

## User-facing Settings metadata

A separate metadata catalogue now maps stable menu IDs to user-facing titles, descriptions, semantic icon keys, owning modules and presentation flags. It copies bounded strings, rejects duplicate IDs and invalid category references, and never creates or changes operational settings by itself. See [Settings UI Metadata Catalogue](settings-ui/metadata/README.md).

## Settings screen and renderer adapter

The Preferences navigation model and metadata catalogue now produce a complete renderer-neutral page snapshot: page title and description, visible ordered rows, user-facing labels, descriptions, semantic icon keys, presentation flags and selected-row state. A small renderer contract exposes page lifecycle, row drawing and footer callbacks so a future Vita backend can implement the Sony-style presentation without coupling the portable core to unverified proprietary APIs.

This is an integration boundary, not a claim that the Vita-native Settings renderer already exists. The adapter is host-testable through a fake renderer; physical Settings integration remains a separate milestone. See [Settings Screen Adapter](settings-ui/SETTINGS_SCREEN_ADAPTER.md).

## Layout Profile Manager

A host-testable controller now exposes the profile list, profile actions, create, apply, update, rename and confirmed deletion flows. It keeps stable profile IDs separate from display names, protects the built-in `default` profile through the underlying profile store, and returns renderer-neutral view snapshots. See [Profile Manager](settings-ui/PROFILE_MANAGER.md).

## Safe module controls

Module adapters can declare only controls they genuinely support: toggles, finite selectors, stepped sliders and bounded numeric inputs. The shared contract validates option uniqueness, numeric ranges and steps, read-only status, backend read values and write requests. Unknown plugin settings are never guessed or edited. See [Module Control Contract](settings-ui/MODULE_CONTROL_CONTRACT.md).

## Storage & Devices snapshot model

The first Storage & Devices core now converts platform-reported observations into a bounded, renderer-neutral snapshot: SD2Vita, Sony Memory Card, internal and USB device kinds; mount state; mount point; total/free capacity; read-only state; free-space percentage; and duplicate mount-point warnings. Invalid capacity data and duplicate stable IDs are rejected. The model never probes, mounts, formats or changes storage itself. The Vita device-enumeration adapter and YAMT mapping detection remain pending. See [Storage & Devices model](settings-ui/STORAGE_DEVICES.md).

## Plugin Control Centre model

The first read-only Control Centre projection now combines the taiHEN inventory, explicit adapter registry and compatibility engine. Each detected entry is classified as Unknown, Disabled, Detected, Configurable, Blocked or Review; the snapshot preserves path/section/source-line details, duplicate counts and compatibility findings. Unknown plugins remain visible but never receive guessed controls. Blocked/review states override adapter availability.

This is a portable model over caller-supplied config text, not yet a Vita filesystem scanner, config editor or native Settings page. See [Plugin Control Centre model](settings-ui/PLUGIN_CONTROL_CENTRE.md).

## Connectivity service lifecycle

The shared service manager exposes backend-reported state, supported start/stop and auto-start operations, and explicit conflict groups for incompatible connection modes. It refreshes all services before starting one and fails closed if any status cannot be read, rather than assuming an unknown service is stopped. This is the portable lifecycle contract; real FTP/USB/streaming adapters still need to bind verified Vita/plugin APIs. See [Service Manager](settings-ui/SERVICE_MANAGER.md).

## Curated Vita plugin catalogue

The project maintains a normalized shortlist for the target PCH-1000 OLED / firmware 3.65 / Ensō / YAMT setup. Every record separates firmware declared by the author, offline validation, physical tests and reports from other devices. It also tracks dependencies, mutually exclusive alternatives, adapter IDs and installation caveats.

See the [curated catalogue](docs/PLUGIN_CATALOGUE.md), [machine-readable JSON](data/plugin-catalogue.json) and [JSON Schema](schemas/plugin-catalogue.schema.json). This is not an install-all list: candidates require project-side review and device testing. Unknown plugins may be inventoried but do not receive guessed controls.

## Plugin inventory and adapter registry

The first host-testable inventory core now parses supplied taiHEN `config.txt` text, tracks active and commented-out plugin entries, sections, source lines and duplicate paths, and maps recognized filenames to catalogue IDs. A separate adapter registry marks a plugin configurable only when an explicit adapter descriptor is registered; unknown or unsupported plugins remain inventory-only. This layer does not access the Vita filesystem or modify configuration. See [Plugin Inventory](docs/PLUGIN_INVENTORY.md). Version detection, dependency/conflict resolution and transactional config editing remain separate follow-up work.

## Plugin compatibility and conflict engine

The new host-testable compatibility engine evaluates explicit dependency and conflict rules before future install/enable operations. It blocks known mutually exclusive providers (including YAMT vs StorageMgr, vitastream vs known USB video drivers, FTP for Vita vs FTP Everywhere, Caffeine vs NoSleep, and overlapping performance/remapping managers), and emits review-level warnings for shared display-LUT and Bluetooth-audio hooks. FTP for Vita and Caffeine also require QuickMenuReborn. It reports findings only: it never edits `config.txt`, chooses a plugin to remove, or treats unrecognized plugins as compatible by assumption. See [Plugin Compatibility](docs/PLUGIN_COMPATIBILITY.md).

## Plugin adapter framework

The first operational adapter is Wi-Fi Turbo. It supports the documented remote-control operations through an injected transport, requires the user to enable Developer mode, and never enables that unauthenticated LAN endpoint itself. Power-profile and 40 MHz changes remain pending until explicitly confirmed or reverted. The portable core contains no socket implementation; host tests use a mock transport. See [Plugin Adapter Framework](docs/PLUGIN_ADAPTER_FRAMEWORK.md).

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

This is the complete product scope for Rt:Settings Extended. Entries below describe the intended user-facing Settings experience, not a claim that every backend or native page already exists. The implementation status table and roadmap near the end of this README remain authoritative.

### Appearance & Customization

A dedicated, Sony-style Settings category for startup presentation and shell appearance. The goal is to manage the user's assets centrally rather than expose unrelated plugin configuration files.

- **Boot Animation** — enable/disable a supported boot animation, choose an installed animation, preview it where technically possible, inspect format/resolution requirements and restore the original.
- **Boot Splash / Custom Boot Logo** — manage compatible boot splash images, convert or validate assets where a safe converter is available, preview the selected image and restore the default. The original Sony boot logo may remain visible if the underlying plugin cannot replace it.
- **Boot Sound** — enable/disable a supported startup jingle, select a sound file, validate WAV format, sample rate, channel count and file-size limits, preview playback when safe, and restore the default sound.
- **Custom Startup Warning** — choose or edit a supported warning message, select from installed presets, preview the result and restore the original warning. Editing is exposed only for versions with a documented and safely writable configuration.
- **Interface Customization** — supported shell appearance options such as status-bar elements, visual indicators, supported color/theme values and other plugin-backed presentation controls. No arbitrary system-resource patching.
- **Custom Assets Manager** — one central library for boot images, animations, sounds, warning presets and other supported assets. It will track ownership, source, format, dimensions, compatibility, checksums, active use and backup/restore state; detect duplicates and orphaned assets; and avoid deleting files not owned by Rt.
- **Asset import/export** — import from supported storage locations, validate before activation, keep the currently working asset as a rollback point, and export selected assets with metadata.
- **Per-feature fallback** — if a plugin or asset is missing/incompatible, show the feature as unavailable with an explanation instead of displaying non-functional controls.

### Quick Menu

A dedicated Settings page for supported Quick Menu frameworks and add-ons. This is not a promise that every Quick Menu plugin exposes a Settings API.

- **Quick Menu framework status** — detect supported frameworks, installed version, load state, required files and dependencies.
- **Layout and widget management** — list detected widgets/add-ons, enable or disable only through a verified mechanism, change order or visibility where the framework supports it.
- **Quick shortcuts** — configure supported shortcuts and launch actions with duplicate detection and safe target validation.
- **Behavior** — supported display, invocation, timeout and interaction options, only when documented by the installed version.
- **Plugin-specific options** — expose each add-on's own settings through a dedicated adapter; never infer fields from filenames or undocumented config.
- **Variant handling** — compare QuickMenuReborn, QuickLauncher and compatible add-ons; retain alternatives only where their feature set, compatibility or device support differs materially.
- **Conflict checks** — detect overlapping Quick Menu replacements, shell hooks and add-ons requiring a specific framework version.

### Connectivity

- **USB Connection** — persistent preference to make the supported USB connection mode available automatically when the console, cable, host and backend permit it; show actual connection state and host requirements.
- **FTP Server** — start/stop, actual running state, supported port settings, startup preference, access information and backend health.
- **FTP alternatives** — distinguish VitaShell FTP, FTP for Vita / QuickMenuReborn add-on and FTP Everywhere. Keep more than one implementation only when their use cases differ; prevent simultaneous port/provider conflicts.
- **Wi-Fi extensions** — adapter-backed controls for Wi-Fi Turbo, including status, Keep Wi-Fi on, HUD, auto-reconnect, Smart Mesh Roaming and supported reconnect actions. Potentially disruptive power/HT40 operations require explicit confirmation.
- **Service lifecycle** — start, stop, refresh, auto-start preference, error reporting and reboot-required state for each supported service.

“Always On” is a persistent startup preference, not a promise that a physical USB link exists without a cable or that an FTP server runs while the console is powered off.

### USB Streaming

- Detect supported USB video/audio streaming backends and their exact versions.
- Display compatibility, dependencies, active state, host-side requirements and connection status.
- Start/stop streaming and report real backend errors.
- Expose only options declared by the selected backend.
- Compare vitastream, VitaUSBStream and other UVC implementations; select one default and retain alternatives only when they provide meaningful compatibility or feature differences.
- Detect mutually exclusive USB/UVC kernel drivers and refuse unsafe simultaneous activation.

### Plugin Control Centre

- Scan supported taiHEN plugin locations and parse active/commented entries, sections, duplicate paths and missing files.
- Show human-readable plugin name, description, category, version, source, author, load location, compatibility and status.
- Group plugins by function and load location.
- Provide per-plugin settings only through a reviewed adapter with a declared schema, valid values, read/write methods, version checks and restore behavior.
- Enable/disable entries only through transactional config editing with backup, preview, validation and rollback.
- Detect dependencies, conflicts, duplicate providers, missing requirements and reboot needs.
- Support named operational profiles (for example Daily, Streaming, Battery Saver and Troubleshooting) without confusing them with menu-layout profiles.
- Keep unknown plugins visible in inventory and diagnostics, but never invent controls or modify them blindly.

### Storage & Devices

- Identify SD2Vita, Sony Memory Card, internal storage and supported USB devices.
- Show mount state, mount point, total/free capacity, read-only status and storage mapping.
- Detect duplicate mount points, missing devices, unexpected mappings and configuration inconsistencies.
- Integrate with YAMT while preserving the user's existing storage setup.
- Provide safe access to Tai Saver backups and recovery data.
- Add storage health checks and guided diagnostics where the platform exposes reliable information.
- Any mount/unmount, format or migration operation must be a separate, explicitly confirmed and recoverable workflow.

### System & Startup

- Startup services, status, dependencies, ordering and supported autostart preferences.
- Boot journal with startup events, failures, recovery markers and exportable diagnostics.
- Boot configuration inspection with guarded edits and automatic pre-change backup.
- Tai Saver startup backup policy, retention and restore workflow.
- Recovery mode, safe-mode guidance and interrupted-operation recovery.
- Clear distinction between immediate changes and changes requiring reboot.

### Display & Audio

- OLED-aware brightness and display enhancements, including supported VitaBrightEX/LUT controls.
- Supported color temperature, display filters and per-panel options, with unsafe OLED/LCD combinations hidden.
- Supported audio enhancements, Bluetooth audio extensions and device status.
- Audio routing, codec and quality options only when the installed backend exposes validated controls.
- Conflict detection for overlapping display hooks, LUT providers and Bluetooth audio stacks.

### Power & Performance

- Supported CPU/GPU clock profiles and monitoring.
- Battery, temperature and power-state information where reliable APIs exist.
- Sleep prevention / Caffeine controls with explicit battery-impact warnings.
- Performance profiles and per-title controls only through compatible adapters.
- Detect overlapping overclock managers and remappers; never load competing providers together.
- Enforce adapter-declared hardware limits, safe defaults and confirmation for risky changes.

### Network & Bluetooth

- Wi-Fi status and supported Wi-Fi plugin controls.
- Bluetooth device/stack status and supported audio add-ons.
- Connection diagnostics and supported network tools.
- TLS/certificate component status where relevant.
- Clearly separate kernel plugins, user plugins, standalone VPK utilities and PSP/Adrenaline plugins.

### System Tools

- Detected shortcuts for VitaShell, file managers, application managers, system monitors and registered utilities.
- Launch only through verified application IDs or supported launch adapters.
- Do not install duplicate utilities or alter third-party app data.
- A shortcut can be hidden or removed without uninstalling its target application.

### Preferences & Profiles

- Reorder Rt categories and supported entries.
- Hide/show categories, rows, tools and widgets without changing their operational state.
- Move eligible Rt-owned entries between compatible pages.
- Save, rename, duplicate, update, apply and delete named menu layouts.
- Provide reset-to-default and validated import/export.
- Keep menu-layout profiles separate from plugin/service operational profiles.

### Framework & Modules

- Shared framework installed once, with independently installable Settings sections.
- Install Selected / Install All, add later, update, repair, enable/disable and remove.
- Dependency resolution, compatibility checks, free-space preflight and package integrity verification.
- Transaction journal, staged writes, atomic config updates, rollback and recovery after interruption.
- Preserve user data and unrelated files when disabling, repairing or removing modules.

### Tai Saver

- Automatic ur0:tai snapshot at startup, with configurable retention (2/3/4/5/custom).
- Separate archives for known-working configurations and failed/KO configurations kept for diagnosis.
- Backup manifests, SHA-256 verification, snapshot comparison and change summaries.
- Restore with a mandatory pre-restore safety snapshot.
- Manual backup, pin/protect snapshots, export and retention cleanup.
- Never classify a configuration as working solely because a file exists; health classification needs an explicit signal or user confirmation.

### System Information & Diagnostics

- Firmware, model, storage, framework and module versions.
- Plugin inventory, detected version/source and compatibility findings.
- Service health, startup journal and recovery status.
- Exportable diagnostic report with sensitive paths/data handled carefully.
- Guided troubleshooting and safe recovery recommendations.

### Design rule for all features

Every function must be presented as a normal Settings entry: a concise title, plain-language description, clear status, and only the controls supported by the installed backend. The feature catalogue is broad by design; a feature becomes installable only after its implementation, dependency metadata, adapter, UI, safety rules, regression tests and target validation are complete.

## Vita plugin research

Plugin discovery is being built from AutoPlugin II's active Vita catalogue, VitaHomebrewDB, GameBrew, PSX-Place and—most importantly—the original authors' repositories and tagged releases. The first source map and shortlist are in [Vita Plugin Research](docs/PLUGIN_RESEARCH.md). The shortlist is not an install-all list: candidates must pass firmware, dependency, conflict, maintenance and safety review. Distinct variants remain selectable only when they solve a real compatibility or feature difference.

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
│   ├── preferences.h         # Stable-ID Settings menu registry API
│   └── layout_profiles.h     # Named layout profile API
├── installer/               # Package/module manifests and schemas
├── settings-ui/             # Settings page specifications and UI model
├── source/
│   ├── tai-saver/           # Snapshot engine and SHA-256
│   ├── boot-monitor/        # Persistent boot lifecycle journal
│   ├── preferences/         # Stable-ID menu registry and persistence
│   └── layout-profiles/     # Named menu layout profiles
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
| Tai Saver snapshot engine, SHA-256 manifests, verify/compare and guarded restore | Implemented; host-tested |
| Boot lifecycle journal | Implemented; host-tested |
| Preferences registry, named layout profiles and navigation model | Implemented; host-tested |
| Settings metadata and renderer-neutral screen adapter | Implemented; host-tested |
| Safe module-control descriptor contract | Implemented; host-tested |
| Conflict-aware service lifecycle manager | Implemented; host-tested |
| Plugin inventory, compatibility engine and read-only Control Centre model | Implemented; host-tested |
| Storage & Devices snapshot model | Implemented; host-tested |
| Wi-Fi Turbo typed adapter contract | Implemented; mock-transport host-tested; Vita transport pending |
| Plugin catalogue, schema and initial source research | Implemented; research shortlist only |
| Boot Animation / Boot Splash controls | Research/specification; implementation pending |
| Boot Sound controls | Research/specification; implementation pending |
| Custom Warning UTF-16LE adapter (read, validate, write, verify, rollback) | Implemented; host regression coverage added; Vita plugin/filesystem integration pending |
| Custom Assets Manager registry, metadata validation and transactional activation/rollback | Implemented; host regression and sanitizer coverage added |
| Asset import/export, filesystem hashing and platform adapters | Pending; backend-specific work required |
| Quick Menu / QuickMenuReborn framework and add-on management | Research/specification; adapter pending |
| Interface Customization | Project target; implementation pending |
| USB persistent connection service | Lifecycle core exists; Vita backend/UI pending |
| FTP service and provider selection | Lifecycle core exists; Vita backend/UI pending |
| USB streaming backends and host integration | Research/compatibility work; implementation pending |
| Live Vita plugin scanning, binary version detection and transactional config editing | Pending |
| Plugin adapters beyond Wi-Fi Turbo | Custom Warning portable adapter implemented; remaining adapters require upstream schema/API verification |
| Live Vita storage enumeration and YAMT mapping | Pending |
| System & Startup native page and recovery integration | Partial host core; Vita integration pending |
| Display & Audio / Power & Performance adapters | Research and implementation pending |
| System Tools detection and launch adapters | Specification; implementation pending |
| Expandable installer transaction engine and VPK UI | Specification; implementation pending |
| Vita-native Settings renderer and Sony category integration | Pending |
| VitaSDK cross-build and package validation | Pending |
| Physical PCH-1000 / 3.65 / Ensō / YAMT validation | Pending |

### Roadmap

1. Complete storage safety and interrupted-staging recovery.
2. Implement the first end-to-end plugin adapter page using the shared schema, validation, status readback and tests.
3. Build the Vita-native Settings renderer and integrate the renderer-neutral screen model.
4. Complete Tai Saver Vita filesystem/startup integration and its retention/known-good/KO archive UI.
5. Implement transactional taiHEN config editing, version detection and safe plugin enable/disable.
6. Add the Wi-Fi Turbo Vita transport and verify the adapter on the target console.
7. Build the expandable installer transaction engine, dependency resolver and module selection UI.
8. Implement USB/FTP service adapters and real lifecycle/status reporting.
9. Validate streaming backends and implement one default provider plus worthwhile alternatives.
10. Implement Quick Menu framework/add-on inventory and adapter-backed settings.
11. Implement the Custom Assets Manager shared foundation.
12. Add separate adapters for Boot Splash, Boot Sound and Custom Warning after upstream/API and 3.65 validation.
13. Add Interface Customization, Display & Audio, Power & Performance and Bluetooth modules.
14. Complete live Storage & Devices enumeration, YAMT mapping and diagnostics.
15. Implement System Tools detection/launch and System & Startup recovery integration.
16. Add Sony-page integrations only where stable extension points are verified.
17. Run VitaSDK builds, packaging, install/update/rollback regression and physical-device validation.

## Documentation

- [Expandable Installer and Module System](docs/EXPANDABLE_INSTALLER.md)
- [Boot Integration](docs/BOOT_INTEGRATION.md)
- [Native Settings Integration](docs/SETTINGS_NATIVE_INTEGRATION.md)
- [Preferences Registry](docs/PREFERENCES_REGISTRY.md)
- [Layout Profiles](docs/LAYOUT_PROFILES.md)
- [Preferences UI Model](settings-ui/PREFERENCES_UI_MODEL.md)
- [Settings UI Metadata Catalogue](settings-ui/metadata/README.md)
- [Complete Settings Feature Catalogue](docs/COMPLETE_FEATURE_CATALOGUE.md)
- [Settings Entry Catalogue](docs/FEATURE_CATALOGUE.md)
- [Vita Plugin Research](docs/PLUGIN_RESEARCH.md)
- [Boot Appearance Adapter Decisions](docs/BOOT_APPEARANCE_ADAPTERS.md)
- [Custom Warning Adapter](settings-ui/CUSTOM_WARNING_ADAPTER.md)
- [Curated Vita Plugin Catalogue](docs/PLUGIN_CATALOGUE.md)
- [Plugin catalogue JSON](data/plugin-catalogue.json)
- [Plugin catalogue schema](schemas/plugin-catalogue.schema.json)
- [Custom Assets Manager Core](settings-ui/CUSTOM_ASSETS_MANAGER.md)
- [Boot Appearance Adapter Decisions](docs/BOOT_APPEARANCE_ADAPTERS.md)
- [Appearance Asset Manifest Schema](schemas/appearance-assets.schema.json)
- [Managed Appearance Asset Catalogue](data/appearance-assets.json)
- [Tai Saver Core](docs/TAI_SAVER_CORE.md)
- [Tai Saver page specification](settings-ui/TAI_SAVER_PAGE.md)
- [Installer package and module format](installer/README.md)
- [Tai Saver feature documentation](tai-saver/README.md)

---

**Long-term objective:** make Rt:Settings Extended the complete, modular Settings environment for the Vita—covering connectivity, USB, FTP, streaming, plugin configuration, storage, startup, system controls, backup and diagnostics—with one expandable installer and no compromise on safety.
