# Settings Entry Catalogue

This is the concise status index for user-facing Rt:Settings Extended entries. The complete feature requirements, controls, safety rules and plugin-selection policy are maintained in [Complete Settings Feature Catalogue](COMPLETE_FEATURE_CATALOGUE.md). The root [README](../README.md#feature-catalogue) also contains the full product scope.

| Entry | Purpose | Current status |
|---|---|---|
| Preferences | Visibility, ordering, reset and named menu-layout profiles | Portable core and profile manager implemented; native page pending |
| Settings UI | Metadata, page snapshots and renderer callback contract | Renderer-neutral core implemented; Vita renderer pending |
| Tai Saver | Automatic backup, retention, verification, comparison and recovery for ur0:tai | Host core implemented; Vita filesystem/startup integration pending |
| Boot Journal | Startup lifecycle and recovery events | Host core implemented; Vita integration pending |
| Framework & Modules | Install, update, repair, disable and remove modular sections | Architecture specified; transaction engine/UI pending |
| Appearance & Customization | Boot animation/splash, boot sound, custom warning, interface appearance and asset library | Shared registry, staged import, SHA-256 verification, verified export and activation/rollback implemented; Custom Warning adapter host-tested; Vita filesystem backend pending |
| Quick Menu | Framework detection, widgets/add-ons, layout, shortcuts and supported behavior controls | Research/specification; adapter pending |
| Connectivity | USB and FTP preferences, status and service lifecycle | Portable lifecycle core implemented; Vita backends pending |
| USB Streaming | Backend detection, streaming state and transport options | Research/compatibility work; implementation pending |
| Plugin Control Centre | Plugin inventory, compatibility and adapter-backed settings | Read-only portable model implemented; live Vita scan/config editor pending |
| Storage & Devices | SD2Vita, memory card, mounts and diagnostics | Snapshot model implemented; live Vita enumeration pending |
| System & Startup | Startup services, boot diagnostics and recovery | Partial host core; Vita integration pending |
| Display & Audio | OLED-aware display, audio and Bluetooth extensions | Research/planning; adapters pending |
| Power & Performance | Supported performance controls and monitoring | Research/planning; adapters pending |
| Network & Bluetooth | Wi-Fi, Bluetooth and connection diagnostics | Wi-Fi Turbo adapter contract implemented; other backends pending |
| System Tools | Detected application shortcuts | Specification; detection and launch adapters pending |
| System Information & Diagnostics | Firmware, module, plugin, service and recovery reports | Planned; implementation pending |

## Status definitions

- **Implemented — host-tested:** portable logic exists and has regression coverage; this does not mean it is available in Vita Settings.
- **Partial:** a supporting model/controller/adapter exists, but the complete user-facing feature is unfinished.
- **Research/specification:** candidate projects or behavior are documented; implementation is not complete.
- **Planned:** no implementation is claimed.
- **Vita-validated:** reserved for a feature tested on real hardware with device model and firmware recorded.

A feature is not considered Vita-available until its platform adapter, native Settings page, package integration, safety behavior and target-device checks are complete.
