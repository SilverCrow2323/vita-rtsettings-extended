# Native Sony Settings integration — adapter design

## Finding

A native Settings extension is technically feasible on the target firmware family. Existing PS Vita projects load a user-mode taiHEN plugin into the Settings application (`NPXS10015`) and hook the PAF XML-layout loader. The hook can supply an embedded XML layout for a Settings page instead of launching a separate homebrew application.

Reference implementations inspected:

- [YAMT user plugin](https://github.com/SKGleba/yamt-vita/blob/2eb966b6b9af6f76879abd07732c83ce174dad8/plugin/user/user.c) — detects the load/unload of `system_settings_core.suprx`, installs the PAF XML loader hook, replaces the `peripherals_settings_plugin` layout and releases hooks when the Settings core unloads.
- [YAMT peripherals layout](https://github.com/SKGleba/yamt-vita/blob/2eb966b6b9af6f76879abd07732c83ce174dad8/plugin/user/peripherals_settings.xml) — demonstrates native nested `setting_list`, `link`, `toggle_switch`, `list` and `button` controls.
- [h-encore Settings adapter](https://github.com/TheOfficialFloW/h-encore/blob/6d43b4cd662d1012ca851a86d43f3696b2323200/plugin/user.c) and [root Settings XML](https://github.com/TheOfficialFloW/h-encore/blob/6d43b4cd662d1012ca851a86d43f3696b2323200/plugin/system_settings.xml) — demonstrates replacing the `system_settings_plugin` XML and intercepting its plugin callback table for custom button actions.

YAMT documents compatibility with Enso on firmware 3.60 and 3.65. This is relevant evidence for the project's 3.65 target, but does not replace testing our own adapter on the user's console.

## Target architecture

1. Build `rtsettings.suprx` as a user-mode taiHEN plugin. It is not a kernel plugin and does not need to patch kernel services.
2. Load it in the Settings title section:
   ```ini
   *NPXS10015
   ur0:tai/rtsettings.suprx
   ```
3. Hook `sceKernelLoadStartModule` / `sceKernelStopUnloadModule` in `SceSettings` to detect the lifetime of `system_settings_core.suprx`.
4. While the Settings core is loaded, hook `scePafMiscLoadXmlLayout` and identify the `system_settings_plugin` XML by bounded content matching, not by assuming an unchecked fixed byte offset.
5. Preserve the original Sony XML and insert one Rt link into its existing root `setting_list`. Do not replace the complete Sony root page with a reduced custom copy.
6. Embed the Rt page XML in the SUPRX as a binary resource. When the PAF loader requests the Rt page, return that resource and its exact byte length.
7. Add Tai Saver as a nested native `setting_list` with stable row IDs. Route operational buttons through a narrowly scoped PAF event adapter to the Tai Saver service. Do not treat an XML control as functional until its callback or Registry Manager binding is implemented and tested.
8. Release every installed hook when `system_settings_core.suprx` unloads and again on plugin stop. Hook installation failures must leave Sony Settings untouched.

## Relevant import evidence

The inspected YAMT implementation hooks these imports in `SceSettings`:

| Import | Library NID | Function NID | Purpose |
|---|---:|---:|---|
| `sceKernelLoadStartModule` | `0xCAE9ACE6` | `0x2DCC4AFA` | Detect Settings core load |
| `sceKernelStopUnloadModule` | `0xCAE9ACE6` | `0x2415F8A4` | Detect Settings core unload |
| `scePafMiscLoadXmlLayout` | `0x3D643CE8` | `0x19FE55A8` | Supply native XML layout |

The h-encore implementation additionally demonstrates the PAF Toplevel plugin-function hook (`ScePafToplevel`, function NID `0xF5354FEF`) and text lookup hook (`0x19CEFDA7`) for its own custom plugin. These callback-table details are plugin-specific evidence, not a blanket ABI guarantee. Rt must validate the exact plugin identity and callback slot before patching anything.

## Installer contract

The eventual VPK is an installer/repair utility, not a second Settings launcher. It must:

- inspect the existing `ur0:tai/config.txt` before proposing changes;
- back up the original config and show the exact `*NPXS10015` line to be added;
- preserve every unrelated plugin line, section, order and comment;
- refuse duplicate or ambiguous edits and provide a rollback path;
- install the SUPRX to an Rt-owned path and register it only after payload verification;
- remove only its own config line and owned plugin files on uninstall;
- preserve `ur0:data/RtSettingsExtended/TaiSaver/` by default.

No VPK may silently edit taiHEN configuration or install a partial plugin.

## Current implementation status

- [x] Portable Settings registry contains the Tai Saver feature tree.
- [x] Host regression suite and ASan/UBSan cover the tree and its navigation.
- [x] VitaSDK cross-build produces the portable static framework library.
- [x] Existing upstream implementations confirm the native Settings hook pattern and the target Settings title.
- [ ] Rt `rtsettings.suprx` adapter source and embedded XML resource.
- [ ] Safe in-memory insertion of the Rt root link without replacing Sony's existing entries.
- [ ] Native event/callback bridge for Tai Saver operations.
- [ ] VPK installer, update, repair and uninstall transaction.
- [ ] Physical-console verification on firmware 3.65 Ensō.

Until the remaining gates pass, this project must describe the Settings Hub as a registered portable navigation model, not as an installed Sony Settings extension.
