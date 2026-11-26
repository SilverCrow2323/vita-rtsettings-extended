# Settings Entry Catalogue

This is the maintained inventory of user-facing entries for Rt:Settings Extended. It distinguishes implemented host-side cores from planned Vita integration.

| Entry | Purpose | Status |
|---|---|---|
| Preferences | Visibility, ordering, reset and layout profiles | Core implemented; native renderer pending |
| Tai Saver | Backup, retention, verification and recovery for `ur0:tai` | Host core implemented; Vita integration pending |
| Boot Journal | Startup lifecycle and recovery events | Host core implemented; Vita integration pending |
| Framework & Modules | Install, update, repair and remove modules | Installer contract documented; installer pending |
| Connectivity | USB and FTP preferences and service lifecycle | Planned |
| USB Streaming | Streaming backend controls and status | Planned |
| Plugin Control Centre | Plugin inventory and adapter-backed settings | Planned |
| Storage & Devices | SD2Vita, memory card and mount diagnostics | Planned |
| System & Startup | Startup services and recovery | Partial host core: Boot Journal |
| Display & Audio | Supported display and audio extensions | Planned |
| Power & Performance | Supported performance controls | Planned |
| System Information & Diagnostics | Firmware, inventory and reports | Planned |
| System Tools | Detected application shortcuts | Planned |
| Advanced Options | Additional specialist settings | Planned |

## Status definitions

- **Core implemented:** portable C logic exists and is covered by host tests.
- **Partial host core:** some supporting logic exists, but the complete entry/service does not.
- **Specification:** architecture and behavior are documented, implementation is not complete.
- **Planned:** no user-ready implementation is claimed.

A row is not considered Vita-available until its platform adapter, native Settings page, packaging and target-device checks are complete.
