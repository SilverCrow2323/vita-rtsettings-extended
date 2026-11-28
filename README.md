# Rt:Settings Extended

**A native PlayStation Vita Settings extension, with a companion management app and an interactive browser simulator.**

Rt:Settings Extended brings additional, discoverable controls into the existing Sony Settings experience. Rt:Vita Cockpit handles installation, updates, diagnostics, backups and recovery; the native Settings extension is intended for everyday use.

> Unofficial community project. Not affiliated with or endorsed by Sony Interactive Entertainment.

## Project components

| Component | Purpose |
|---|---|
| **Native Settings (SUPRX)** | Adds the Rt:Settings Extended category inside the original Vita Settings application. |
| **Rt:Vita Cockpit** | LiveArea utility for installation, maintenance, diagnostics and component management. |
| **Rt Components** | Individually versioned modules with manifests, compatibility information, dependencies and conflict metadata. |
| **GitHub Pages simulator** | Browser-based interactive preview of the Sony Settings layout and the planned Rt settings hierarchy. |

## Interactive PS Vita Settings simulator

**[Open the PS Vita Settings Simulator](https://silvercrow2323.github.io/vita-rtsettings-extended/)**

The simulator provides:
- a responsive Vita-inspired screen using the repository wallpaper in `assets-psvita/psvita-settings.jpeg`;
- Sony Settings and Rt:Settings Extended navigation layers;
- category and submenu navigation, selection state, demo toggles and confirmation dialogs;
- keyboard controls (Up/Down, Enter, Escape/Backspace), mouse and touch input;
- an illustrative Vita 1000 OLED / firmware 3.65 profile.

The website is a visual and interaction prototype. It does **not** connect to a Vita, read taiHEN configuration, access storage, pair controllers, start FTP, install plugins or change console settings. Values and device status shown in the browser are simulated.

## Native integration and current implementation

The first native Settings integration is a user-mode SUPRX loaded in the Settings title (`*NPXS10015`). It hooks the PAF XML import path and adds the Rt category to the Settings XML in memory; it does not replace Sony's XML files on disk.

Current native navigation includes the Rt root and Preferences & Settings Management / Rt Manager / Rt Manager Status. The native status view is read-only. Cockpit has a read-only taiHEN plugin inventory/audit and a guarded Native Settings installer.

The current implementation does **not** mean every category visible in the website is already available as a native control. Use the [native/web parity matrix](docs/NATIVE_WEB_PARITY_MATRIX.md) to distinguish implemented native behavior from simulator previews and planned work.

## Safety and compatibility

- Preserve the user's existing taiHEN configuration, unrelated plugin entries and external plugin ownership.
- Do not silently switch storage drivers or activate mutually exclusive modules.
- Known conflicts such as YAMT vs StorageMgr and competing overclock managers must be blocked by the compatibility layer.
- Unknown or unverified combinations require review and remain inactive.
- Dependencies, firmware and hardware applicability must be verified before activation.
- Configuration changes require backup, preview, validation and rollback.
- Browser Pages is deliberately disconnected from device operations.

See [simulator architecture and safety boundaries](docs/SETTINGS_SIMULATOR_ARCHITECTURE.md).

## Target and compatibility

Initial development target:
- PlayStation Vita 1000 OLED;
- firmware 3.65 with Ensō;
- taiHEN environment.

Vita 2000 LCD and PSTV-specific options must be gated by detected hardware and validated adapters. A profile shown in the browser is illustrative and is not hardware detection.

## Repository layout

```text
.
├── assets-psvita/       # PS Vita reference imagery
├── cockpit/             # Rt:Vita Cockpit application
├── data/                # Shared simulator and component data
├── docs/                # Architecture, safety and implementation notes
├── include/             # Shared C interfaces
├── native-settings/     # Native Sony Settings SUPRX
├── site/                # GitHub Pages simulator
├── source/              # Shared/core implementation
├── tests/               # Host-side regression and contract tests
└── .github/workflows/   # CI, VitaSDK builds and Pages deployment
```

## Build and validation

Host-side tests and static checks run through GitHub Actions. Vita-targeted components are cross-built using VitaSDK.

The Pages workflow validates the simulator JavaScript, JSON model, safety contract tests and required site files. Deployment runs from `main` after the Pages changes are merged.

For native builds, use the repository Makefiles and the official VitaSDK toolchain. Consult the component-specific READMEs before building or installing individual modules.

## Development status

This is active development. CI success confirms only the checks actually run by the workflow; it does not replace testing on physical Vita hardware. No claim of complete hardware validation is made.

- Native Settings integration: initial root/status implementation.
- Cockpit: installer and read-only plugin audit milestones implemented; additional management workflows remain in development.
- Tai Saver: verified pre-restore snapshot creation implemented; browsing, comparison and full restore workflow remain in development.
- Settings pages: simulator hierarchy is broader than current native implementation.
- GitHub Pages: static browser simulator, no console connectivity.

## Contributing

Keep native behavior, Cockpit operations and browser simulation clearly separated. Do not mark a feature as implemented until its code, tests, compatibility metadata and applicable hardware validation support that status.

## Credits

PlayStation Vita and Sony system software names are trademarks of their respective owners. Plugin authors and upstream projects retain their original copyrights. Individual module credits and source links should accompany their respective manifests and documentation.
