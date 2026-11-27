# PS Vita Settings integration research

Research date: 27 November 2026  
Target project: Rt:Settings Extended  
Target console: PS Vita PCH-1000, firmware 3.65 Ensō

This document compares the main native Settings integration patterns found in public PS Vita projects. It distinguishes (A) adding pages to the Sony Settings application, (B) opening the system Settings dialog from another app, and (C) launching a separate homebrew application with state/arguments.

## Executive findings

There are three separate mechanisms. They should not be conflated:

1. **Native Settings page injection:** a user-mode SUPRX is loaded into the Settings title, hooks the PAF XML layout loader and supplies or edits the XML used to build a page. Optional PAF callback and Registry Manager hooks make controls operational.
2. **System Settings dialog URI:** an app calls `sceAppMgrLaunchAppByUri(..., "settings_dlg:")` (or a known subpage URI such as `settings_dlg:updater`) to open a system-owned Settings dialog. This does not launch an arbitrary homebrew page and does not, by itself, register a custom URI handler.
3. **Launching a homebrew app:** an app uses an application URI such as `psgm:play?titleid=...`, optionally after passing launch state through a bridge/shared state. VitaDeploy uses this for its main toolbox.

Rt can use (1) for the persistent native Settings experience and may use (2) as a shortcut to Sony Settings. A dedicated Rt full-screen manager should use (3) unless a verified Sony Settings applet route is implemented. Do not assume `settings_dlg:rtsettings` can open a custom Rt page.

## 1. VitaDeploy: integrated Settings UI + dialog launcher

Repository: https://github.com/SKGleba/VitaDeploy

Relevant source:
- [cfgu/user.c](https://github.com/SKGleba/VitaDeploy/blob/64808f2a90733a1919d475542074da92f2f41981/cfgu/user.c)
- [cfgu/system_settings.xml](https://github.com/SKGleba/VitaDeploy/blob/64808f2a90733a1919d475542074da92f2f41981/cfgu/system_settings.xml)
- [launcher/main.c](https://github.com/SKGleba/VitaDeploy/blob/64808f2a90733a1919d475542074da92f2f41981/launcher/main.c)
- [README](https://github.com/SKGleba/VitaDeploy/blob/64808f2a90733a1919d475542074da92f2f41981/README.md)

### Native Settings implementation

VitaDeploy embeds a custom `system_settings.xml` resource in its user plugin. The plugin:

- waits for `vs0:app/NPXS10015/system_settings_core.suprx` by hooking `sceKernelLoadStartModule`;
- hooks `scePafMiscLoadXmlLayout` in `SceSettings`;
- recognizes the root `system_settings_plugin` layout and supplies its embedded Settings XML;
- hooks Registry Manager get/set/introspection functions for its own `/CONFIG/...` keys;
- hooks `scePafToplevelInitPluginFunctions` and wraps the Settings button callback;
- releases the hooks when the Settings core unloads and on plugin stop.

The XML defines a native VitaDeploy page with nested `setting_list`, `button`, `list`, `list_item`, `toggle_switch` and `label` controls. Its callback dispatches operations such as launching VitaShell, selecting firmware/install options, formatting, rebooting and app downloads. This is a genuine native Settings UI, not a WebView or a separate LiveArea UI.

Important caveat: this implementation replaces the root `system_settings_plugin` layout with a VitaDeploy-owned XML resource. Rt should **not** copy this replacement strategy for its first release, because the product requirement is to preserve Sony's existing root categories and other extensions. We should splice one Rt-owned entry into the existing XML instead.

### The `settings_dlg:` launcher

VitaDeploy's `launcher/main.c` does the following:

- if the companion kernel bridge is not loaded, launches VitaDeploy's own `app0:main.self` using `sceAppMgrLoadExec`;
- if the bridge is already loaded, calls `sceAppMgrLaunchAppByUri(0xFFFFF, "settings_dlg:")` to open the in-app Settings dialog.

Separately, its Settings-page buttons launch the main VitaDeploy application using `psgm:play?titleid=SKGD3PL0Y`. Before launching, it writes launch mode/submenu/options through its bridge, so the app opens in the requested sub-tool.

**Interpretation:** `settings_dlg:` is the system's Settings-dialog URI. It opens the system-owned Settings UI. VitaDeploy's custom page exists because its SUPRX/XML integration is installed into Settings; the URI itself is not a custom-page registration API. The `psgm:play` path launches VitaDeploy itself, with app state selected beforehand.

Rt opportunity:
- add a native Rt category/page via the Settings plugin;
- add an optional shortcut in an Rt/VitaDeploy integration or another app to open the Settings dialog;
- if a richer manager is needed, launch a dedicated Rt app with an explicit subpage/action argument and return to the caller cleanly.

## 2. Tweaks/Plugins: the most relevant aggregator pattern

Repository: https://github.com/chazzarchibald-ops/vita-tweaks-plugins

Relevant source:
- [README](https://github.com/chazzarchibald-ops/vita-tweaks-plugins/blob/9dc8cd742ad801bb27b66d4a2738900e0e6f7050/README.md)
- [main.c](https://github.com/chazzarchibald-ops/vita-tweaks-plugins/blob/9dc8cd742ad801bb27b66d4a2738900e0e6f7050/main.c)

This project adds a **Tweaks/Plugins** row directly below HENkaku Settings and groups other plugins' Settings pages under it. It can discover pages added by plugins, move them into the aggregate list, remove them from their original locations, and expose switches to keep/move individual pages.

Its README documents several important implementation details:

- It hooks the same `scePafMiscLoadXmlLayout` import used by HENkaku and most Settings plugins.
- It must run last in the XML hook chain to see all edits made by other plugins. It uses two hooks in a deliberate chain-order arrangement and recommends loading its SUPRX first in the `*NPXS10015` section.
- It probes original Sony page XML from memory and passes it through the Settings XML import stub so other plugins' hooks can modify it; it then identifies non-Sony entries.
- It caches page XML when probing is unavailable.
- It preserves HENkaku Settings and entries with custom widgets by default.
- It has a crash guard: if Settings crashes during initialization repeatedly, it disables its modifications and leaves Settings untouched.
- It does not operate under `*NPXS10016`, described as the Settings pop-up other apps open for a single page.
- It includes logging, a page cache, configuration options, an installer VPK and self-update/uninstall behavior.

This is a very strong research reference for Rt's future **Plugin Settings Aggregator**. We should not immediately make it a dependency or duplicate its code. Its probing, hook-order and crash-recovery design need a separate source audit and firmware test. The "last in chain" trick is implementation-specific and must not be assumed safe without reproducing the exact hook behavior on our build.

### What Rt should take from it

A future page catalogue can distinguish:
- Sony-owned rows;
- HENkaku Settings;
- plugin-created page/subpage;
- plugin custom widgets;
- unknown/unclassified rows.

A user can choose:
- keep the plugin page where its author placed it;
- show a shortcut under Rt;
- optionally aggregate/move a compatible page after testing;
- never move custom-widget pages automatically.

For the first macro-section, a simple Rt link is safer than moving third-party pages.

## 3. HENkaku Settings and h-encore

Repository: https://github.com/TheOfficialFloW/h-encore

Relevant source:
- [plugin/user.c](https://github.com/TheOfficialFloW/h-encore/blob/6d43b4cd662d1012ca851a86d43f3696b2323200/plugin/user.c)
- [plugin/system_settings.xml](https://github.com/TheOfficialFloW/h-encore/blob/6d43b4cd662d1012ca851a86d43f3696b2323200/plugin/system_settings.xml)

HENkaku Settings is a native Settings extension. h-encore's plugin source demonstrates the same Settings lifecycle pattern: wait for `system_settings_core.suprx`, hook PAF XML loading, supply a Settings XML page, and intercept PAF plugin callbacks for custom controls. It also demonstrates the top-level `system_settings_plugin` route and the HENkaku page plugin identity `idu_settings_plugin`.

This is the historical reference for how HENkaku's own page is constructed. Rt should not replace or rewrite HENkaku's page; it should add a separate Rt-owned entry while leaving HENkaku untouched.

## 4. YAMT: nested page in an existing Sony category

Repository: https://github.com/SKGleba/yamt-vita

Relevant source:
- [user plugin](https://github.com/SKGleba/yamt-vita/blob/2eb966b6b9af6f76879abd07732c83ce174dad8b/plugin/user/user.c)
- [peripherals layout](https://github.com/SKGleba/yamt-vita/blob/2eb966b6b9af6f76879abd07732c83ce174dad8b/plugin/user/peripherals_settings.xml)

YAMT is the reference for putting a plugin's settings under an existing Sony category: its UI appears under Settings → Devices → Storage Devices. The user plugin waits for the Settings core, hooks the XML loader, edits the Devices page and registers its own page controls. It also hooks Registry Manager functions for persistent values and releases hooks on core unload.

This directly supports the user's requirement that some Rt controls belong inside existing Sony categories rather than all being top-level Rt entries.

## 5. Plugin installers and taiHEN configuration

### AutoPlugin II

Repository: https://github.com/ONElua/AutoPlugin2

Relevant source:
- [tai.lua](https://github.com/ONElua/AutoPlugin2/blob/03c638f3edd740cc586aa46388183f4b4a4598a5/AUTOPLUG2/scripts/tai.lua)
- [plugin manager](https://github.com/ONElua/AutoPlugin2/blob/03c638f3edd740cc586aa46388183f4b4a4598a5/AUTOPLUG2/scripts/psvita/pmanager.lua)

AutoPlugin II parses taiHEN configuration into sections and plugin entries, supports insertion/deletion and duplicate cleanup, and handles critical HENkaku entries. Its code also illustrates why a generic "rewrite config from defaults" installer is dangerous: the config contains multiple title sections and user-specific ordering. Its own reset-config UI explicitly warns that all installed plugins will be uninstalled.

Rt's installer should instead use a minimal, reversible edit:
- detect whether `ux0:tai/config.txt` or `ur0:tai/config.txt` is active (with clear ambiguity handling);
- preserve bytes, comments, line endings, ordering and unrelated entries;
- add only the Rt SUPRX line under the intended title section;
- preserve HENkaku and other Settings plugins;
- create a verified backup and provide rollback;
- remove only Rt's own line/files on uninstall.

### taiHEN title sections

`*NPXS10015` is the full Settings application. `*NPXS10016` is a Settings dialog/popup context used when other apps invoke Settings pages. They are not interchangeable. A plugin that only changes the full Settings menu may intentionally do nothing in the popup context. Some plugins must target both, depending on their purpose.

## 6. `settings_dlg:` and URI research

Useful references:
- [Freakler/vita-uriCaller](https://github.com/Freakler/vita-uriCaller/blob/86b37208cb8098a2d5d208f9287dff87c4575644/README.md)
- [VitaSDK appmgr.h](https://github.com/vitasdk/vita-headers/blob/c00157c2abd6dfa05c126e7f2f4efb29e8431ba6/include/psp2/appmgr.h)
- [VitaXMB URI usage](https://github.com/dekiller82/VitaXMB/blob/b2512e46a8562ed4f2203b63647e89bd2b96b9ef/src/core/menus.h)

Known system dialog URIs include:
- `settings_dlg:` — Settings dialog/root;
- `settings_dlg:updater` — system updater page;
- additional service-specific routes exist, but should be treated as undocumented/firmware-dependent unless independently verified.

VitaSDK documents `sceAppMgrLaunchAppByUri(int flags, const char *uri)`; its header notes that flags other than `0x20000` open LiveArea. Community code commonly uses `0xFFFFF` for direct launch. Rt must verify the appropriate flag and return behavior on firmware 3.65 instead of blindly copying a magic constant.

A custom Rt route such as `settings_dlg:rt` is **not established** by the references. The safe supported paths are:
- open the generic Sony Settings dialog with `settings_dlg:`;
- add Rt's page through the native Settings XML/SUPRX integration;
- launch Rt's own app by its title ID/URI if a standalone full-screen manager is desired.

## 7. Additional Settings plugin examples discovered

- [Wi-Fi Turbo](https://github.com/juanjsebgarcia/vita-turbo-wifi): native page under Network; uses a Settings SUPRX and hooks XML, Registry Manager and PAF callbacks.
- [Bluetooth Turbo](https://github.com/juanjsebgarcia/vita-turbo-bluetooth): adds controls within Devices/Bluetooth and demonstrates per-device row/value callback wrapping.
- [Storage Turbo](https://github.com/juanjsebgarcia/vita-turbo-storage): Settings integration for storage-related controls.
- [Engram](https://github.com/chazzarchibald-ops/engram-vita): native Settings integration and a separate installer; its docs discuss `NPXS10015` vs `NPXS10016`, hook order and recovery behavior.
- [VitaTools / mcfredir](https://github.com/SKGleba/VitaTools): illustrates that some system operations have a separate Settings-page path (`NPXS10015`) and a popup path (`NPXS10016`).
- [VitaTweaks](https://github.com/TheOfficialFloW/VitaTweaks): collection of system tweaks, but a generic `*main` plugin is not automatically a Settings page. Each plugin must be evaluated separately.
- [Vita URI Caller](https://github.com/Freakler/vita-uriCaller): useful catalogue of known system URI schemes, including Settings dialog routes.

## 8. Proposed Rt architecture

### A. Native Settings plugin

One user-mode `rtsettings.suprx`, initially loaded only in `*NPXS10015`. It should:
- wait for the Settings core module;
- install narrowly scoped, verified PAF hooks;
- splice one Rt entry into the existing Sony root list, not replace the root XML;
- offer the first read-only Rt page;
- release hooks and page resources on Settings core unload;
- fail closed if expected module, XML identity, callback table or buffer bounds do not match.

### B. Sony-category integrations

Add feature-specific rows only where they naturally belong and where their controls are implemented:
- Devices → Bluetooth & Controllers;
- Devices → Storage Devices;
- Network → network-related adapters;
- Sound & Display → display/audio options.

Keep these as optional nested adapters. The Rt root category remains the home for cross-system features such as Tai Saver, plugin management, diagnostics and profiles.

### C. Plugin Settings Aggregator (future milestone)

Use the Tweaks/Plugins research as a separate optional feature:
- discover compatible plugin pages;
- present shortcuts first, without moving pages;
- add opt-in move/keep rules only after page-type and callback compatibility checks;
- preserve custom widgets and button behavior;
- use logging, bounded caches and a crash guard.

### D. Applet/launcher path

For an optional Rt manager:
- `settings_dlg:` can open the system Settings dialog;
- `psgm:play?titleid=<Rt app title>` can launch the Rt manager app;
- pass a validated action/page request through a dedicated shared-state or bridge interface;
- do not claim a custom Settings URI works until an actual registered route is implemented and tested.

### E. VPK installer

The installer must not replace `henkaku.suprx`, replace Sony's Settings XML, reset taiHEN config or write a complete default config. It should install only Rt-owned files and make a previewed, backed-up, reversible edit to the active taiHEN config.

## 9. Implementation priority

1. Finish Boot Journal migration to the existing `rt_fs` boundary.
2. Implement and test the minimal `rtsettings.suprx` lifecycle and Settings-core detection.
3. Implement safe root XML insertion and a read-only Rt page.
4. Test load/unload/reopen cycles and compatibility with HENkaku, YAMT, Wi-Fi Turbo and other Settings plugins.
5. Build the first installer VPK with preview, backup, rollback, repair and uninstall.
6. Only then add the operational Tai Saver Settings callbacks and nested Sony-category integrations.
7. Consider a separate Rt full-screen manager/app-launch bridge and optional plugin-page aggregator as later milestones.

## Scope and evidence limits

This is a source-based design audit, not a device validation report. It does not establish that every hook/NID is stable on every firmware, that `settings_dlg:` can open an arbitrary custom page, or that a third-party plugin's page can safely be moved without breaking its callbacks. All native hooks, PAF callback slots, XML layouts, URI flags and lifecycle behavior must be validated on the exact 3.65 target. No third-party binary or source has been copied into Rt.
