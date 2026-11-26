# Complete Settings Feature Catalogue

**Scope snapshot:** 26 November 2026  
**Target profile:** PS Vita 1000 OLED · firmware 3.65 · Ensō · YAMT + SD2Vita

This document is the detailed feature register behind the user-facing catalogue in the repository README. It is intentionally broader than the currently implemented host-side cores. A feature is not considered available on Vita until its backend, Settings page, packaging, safety behavior and target-device validation are complete.

## Presentation contract

Every feature must appear as a normal Settings row or submenu with:
- A concise, human-readable title.
- A one- or two-sentence description explaining what it changes.
- A truthful state: Available, Installed, Active, Inactive, Unsupported, Incompatible, Blocked, Error or Requires reboot.
- Only controls declared by a verified adapter.
- Version/source/dependency details in an information subpage rather than cluttering the main row.
- Restore/default behavior and confirmation for risky changes.

The UI must not expose raw plugin filenames as the primary user experience. It should explain the capability, not require users to understand taiHEN internals.

## Appearance & Customization

### Boot Animation and Boot Splash
- Select and activate a supported boot animation or splash.
- Validate file format, dimensions, color depth, size and plugin compatibility before activation.
- Preview assets when the platform supports preview without reboot.
- Preserve the active working asset and the original default as rollback points.
- Distinguish an animation replacement from a static splash/logo plugin; do not imply that one implementation supports both.
- Explain limitations such as the original Sony boot logo remaining visible.

### Boot Sound
- Select a supported startup sound.
- Validate supported container/codec, WAV sample rate, channel count, metadata and file-size limits.
- Preview through a safe audio path when available.
- Warn about known conflicts with system music/audio hooks and firmware-specific NID behavior.
- Restore the previous sound or disable the feature without deleting imported files.

### Custom Startup Warning
- Detect supported warning-message plugins and their installed versions.
- Offer presets and text editing only where the plugin exposes a documented writable setting.
- Preview the exact rendered warning and preserve the original message.
- Never assume that a plugin named "Custom Warning" supports arbitrary message editing; verify its actual interface first.

### Interface Customization
- Expose supported status-bar, visual indicator, theme/color and shell presentation options.
- Separate settings by plugin/backend and hide controls not supported by the installed version.
- Detect overlapping shell patches and restore the previous state after failed writes.

### Custom Assets Manager
- **Implemented portable foundation:** bounded central asset registry, stable IDs, owner/path/format/size/dimensions/duration/audio metadata, SHA-256 verification state, compatibility, active/backup/orphan state and duplicate detection.
- Track asset ID, type, path, source, author/license metadata when known, hash, format, dimensions/duration, compatibility, active consumer and backup state.
- Validate before activation; the host-tested core rejects unsafe paths, traversal, invalid formats, unsupported sizes and unverified activation.
- Detect duplicate and orphaned assets.
- Delete only assets owned by Rt or explicitly selected by the user; never clean up unknown third-party files automatically.
- Export/import asset bundles with manifests and checksums (**platform file-copy/hash adapter pending**).
- Transactional activation through a backend contract: verify the new active state and restore the previous asset or engine default on failure.

## Quick Menu

- Detect QuickMenuReborn framework version, required files, add-on directory and load state.
- List compatible widgets and add-ons with title, description, author, version, dependency and status.
- Show a widget's controls only when its own adapter declares them.
- Manage order, visibility, enable/disable and shortcuts only where the framework/add-on actually supports those operations.
- Expose behavior options (invocation, layout, timeout, display and interaction) only from verified settings schemas.
- Keep QuickMenuReborn, QuickLauncher and other compatible variants distinct where the user-visible behavior or compatibility materially differs.
- Resolve required framework versions and dependencies before installing add-ons.
- Detect overlapping Quick Menu replacements and shell hooks.
- Do not claim that QuickMenuReborn itself is a universal Settings API: its documented project role is a library/framework for developers to add widgets to the Vita Quick Menu.

## Connectivity and streaming

### USB Connection
- Report connection mode, cable/host presence where detectable and service state.
- Support persistent start/availability preference only where a backend supports it.
- Explain that no physical link exists without the required cable and host.
- Keep ordinary USB mode separate from UVC streaming.

### FTP
- Distinguish VitaShell's built-in FTP workflow, FTP for Vita QuickMenuReborn add-on and FTP Everywhere.
- Show actual backend, bind port, running state and connection information when available.
- Start/stop and autostart only through a verified service adapter.
- Detect port conflicts and avoid simultaneous providers.
- Never describe an add-on toggle as an always-running daemon unless its behavior proves that.

### Wi-Fi
- Wi-Fi Turbo adapter is the first operational plugin adapter in the portable core.
- Show status and documented controls; require explicit user consent for Developer mode/remote endpoint.
- Risky power-profile and HT40 changes require confirmation and a revert path.
- Warn about battery and sleep implications of keeping Wi-Fi active.

### USB Streaming
- Compare vitastream, VitaUSBStream and other maintained UVC backends.
- Track firmware/device evidence separately from author-declared compatibility.
- One active UVC kernel provider at a time unless upstream documents safe coexistence.
- Show host-side driver/software requirements and real streaming state.

## Plugin Control Centre

- Inventory active and commented taiHEN entries, sections, duplicate paths and missing binaries.
- Resolve friendly name, description, author, version, source and category from verified catalogue metadata.
- Detect installed version from trusted package metadata or binary metadata only when reliable.
- Per-plugin controls require an adapter with explicit schema, valid values, read/write methods, version checks, backup and rollback.
- Enable/disable edits require a preview, atomic config update and recovery copy.
- Compatibility checks cover dependencies, mutually exclusive providers, duplicate hooks and unsupported combinations.
- Unknown plugins remain visible and searchable but read-only.
- Separate Vita taiHEN plugins from PSP/Adrenaline plugins, title-specific patches and standalone VPK utilities.

## Storage & Devices

- SD2Vita, Sony Memory Card, internal storage and USB device inventory.
- Mount state, mount point, capacity, free space and read-only state.
- YAMT configuration and mapping inspection.
- Duplicate mount detection, missing-device warnings and storage diagnostics.
- Tai Saver archive browser.
- Mount/unmount, format and storage migration are separate guarded workflows with explicit confirmation and recoverable backups.

## System & Startup

- Startup services and dependency/order information.
- Boot journal and exportable startup diagnostics.
- Boot configuration inspection and guarded changes.
- Tai Saver automatic startup snapshots and retention policy.
- Recovery status, interrupted transaction detection and rollback.
- Clear reboot-required state.

## Display & Audio

- OLED-aware brightness and LUT controls, including reviewed VitaBrightEX options.
- Safe panel-specific display controls; do not apply LCD assumptions to OLED.
- Audio enhancement and Bluetooth audio plugin status.
- Codec, routing and quality options only when supported by the detected backend.
- Conflicts for overlapping LUT, brightness and Bluetooth stack hooks.

## Power & Performance

- Supported CPU/GPU profiles and monitoring.
- Battery, temperature and power-state information when available.
- Sleep-prevention service controls with battery-impact warnings.
- Per-title settings only when adapters and title IDs are validated.
- Conflict detection across overclock and remapping managers.
- Bounded values and explicit confirmation for risky changes.

## Network & Bluetooth

- Wi-Fi status, supported extensions and diagnostics.
- Bluetooth stack/device status and optional audio add-ons.
- TLS/certificate component status where relevant.
- Separate kernel/user plugin load types and dependencies.

## System Tools

- Detected application shortcuts for VitaShell, file managers, application managers and monitoring tools.
- Verified application IDs and launch adapters only.
- No duplicate app installation and no ownership of third-party app data.

## Preferences and profiles

- Category and row ordering, visibility, eligible Rt-entry moves and reset.
- Named menu-layout profiles with create/update/rename/apply/delete.
- Validated import/export and recovery defaults.
- Operational plugin/service profiles remain separate from visual menu layouts.

## Framework & Modules

- Shared runtime plus independently installable sections.
- Install Selected, Install All, add later, update, repair, enable/disable and remove.
- Dependency and conflict resolution, package verification and storage preflight.
- Staged writes, transaction journal, atomic config changes and rollback.
- Preserve user data and unrelated files.

## Tai Saver

- Automatic ur0:tai backup at every startup.
- Configurable retention: 2, 3, 4, 5 or custom.
- Separate working/known-good and failed/KO archives for analysis.
- SHA-256 manifests, verify, compare and restore.
- Mandatory pre-restore safety snapshot.
- Manual backup, pin/protect, export and cleanup.
- A configuration is not marked working merely because it exists; health classification requires explicit evidence or user confirmation.

## System Information & Diagnostics

- Firmware/model/system component details.
- Framework/module versions and plugin inventory.
- Compatibility and conflict reports.
- Service/boot status and recovery guidance.
- Exportable diagnostic bundle with privacy-aware path handling.

## Custom Assets Manager implementation status\n\nThe portable C99 registry and activation/rollback boundary are implemented in `include/custom_assets.h` and `source/custom-assets/custom_assets.c`. Host tests cover validation, duplicate detection, path traversal, audio constraints, ownership, protected backups, activation read-back, rollback and rollback failure. This does not yet implement Vita filesystem import/export, hash calculation, format conversion, CBSM/bootanim/BootSound/Custom Warning backends, or the native Settings page.\n\n## Plugin and version selection policy

1. Discover candidates from AutoPlugin II, VitaHomebrewDB, GameBrew, PSX-Place and community release threads.
2. Verify each candidate against the original author's repository, release notes, tags and binary assets.
3. Prefer the latest stable version that is compatible with the target device/firmware; newest does not automatically mean best.
4. Keep older or alternative variants only when they solve a real compatibility, feature, hardware or maintenance difference.
5. Keep nightly/development builds in a clearly marked experimental channel, never as the default.
6. Record upstream URL, exact version/tag, release date, supported firmware evidence, dependencies, conflicts, asset filename, SHA-256 and verification date.
7. Separate author-declared support, offline checks, other users' reports and our own physical-device tests.
8. No install or configuration write without backup, preflight, validation and rollback.

## Status vocabulary

- **Implemented — host-tested:** portable logic and regression tests exist; not necessarily available on Vita.
- **Partial:** a supporting model/controller exists but the full feature is not complete.
- **Specification:** behavior and interface are documented.
- **Research:** candidate implementations are being compared.
- **Planned:** no implementation is claimed.
- **Vita-validated:** only after a real target device test with model and firmware recorded.

See the [README feature catalogue](../README.md#feature-catalogue), [plugin research](PLUGIN_RESEARCH.md) and [curated plugin catalogue](PLUGIN_CATALOGUE.md).
