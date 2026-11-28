# Rt native Sony Settings adapter

This directory contains the first native Settings SUPRX for Rt:Settings Extended.

## Runtime target

- Process/title: Sony Settings, `NPXS10015`
- taiHEN section: `*NPXS10015`
- Module: `ur0:tai/rtsettings.suprx`
- Firmware target for the first device pass: PS Vita 1000 OLED, firmware 3.65 Ensō.

The module is user-mode. It does not replace HENkaku, does not install a kernel plugin, and does not rewrite Sony's root XML resource on disk.

## Current native page

The module waits for `system_settings_core.suprx`, then installs three narrowly scoped PAF hooks:

- `scePafMiscLoadXmlLayout` in `SceSettings`: adds one Rt root category to the existing `system_settings_plugin` XML in memory.
- `scePafToplevelInitPluginFunctions` in `SceSettings`: wraps the root Settings button callback and handles only the Rt status action.
- `scePafToplevelGetText` in `SceSystemSettingsCore`: resolves the Rt status dialog and hint strings.

The visible path is:

```text
Settings
└── Rt:Settings Extended
    └── Preferences & Settings Management
        └── Rt Manager
            └── Rt Manager Status
```

The status action confirms that the extension is installed. It does not claim that plugin-management, Tai Saver or other feature backends are already connected to the native page. Those controls must be added only with their operational callbacks and validated backends.

## Safety behavior

- XML matching is bounded and checks the exact root plugin marker and root ID.
- The original Sony XML is passed through unchanged unless a complete insertion succeeds.
- Repeated XML loads are idempotent; an existing Rt ID is never duplicated.
- The hook buffer is module-owned and remains valid while the PAF parser consumes it.
- Hook installation is all-or-nothing for the page: the XML hook is installed only after the text and callback hooks succeed.
- All page hooks are released when the Settings core unloads and on module stop.
- No hook failure changes Sony Settings files or taiHEN configuration.

## Build

Run inside the official `vitasdk/vitasdk` container:

```sh
make -C native-settings clean all
```

The output is `native-settings/rtsettings.suprx`.

## Important validation boundary

The PAF import NIDs and lifecycle pattern are based on inspected HENkaku, YAMT and Wi-Fi Turbo implementations. The source compiles in CI only after the VitaSDK job passes. No physical PS Vita test is claimed by this repository change. The exact menu rendering, Settings reload behavior, and coexistence with the user's active plugin chain still require testing on the target 3.65 Ensō console.
