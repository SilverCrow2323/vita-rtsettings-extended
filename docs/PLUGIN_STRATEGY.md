# Rt Plugin Strategy

## Design decision
Rt is an integration and safety layer, not a replacement for every established Vita plugin. Keep proven upstream drivers and patch engines where their scope is mature; build Rt-owned navigation, profiles, diagnostics, compatibility intelligence and missing glue.

One active backend per overlapping hook is the default. Plugins patching the same shell surface, USB transport, brightness pipeline, Bluetooth input path or overclock interface must be treated as a compatibility question, not casually stacked.

## Surface ownership
| Surface | Reuse / evaluate | Rt-owned work |
|---|---|---|
| Graphics / display | VitaGrafix for per-title resolution/FPS patches; Sharpscale for framebuffer scaling; VitaBright for brightness/gamma | Unified Graphics Centre, profiles, capability detection, diagnostics |
| Shell status bar | ShellBat for its narrow battery-percentage use | Rt:ShellBar only if broader indicators justify it; never stack status-bar patchers blindly |
| Quick Menu | Evaluate QuickMenuReborn as widget host; test Quick Menu Plus and PSVshellPlus separately | Rt widgets for connectivity, profiles, storage and diagnostics |
| VSH / performance | Reuse one validated backend such as PSVshell/PSVshellPlus | Rt Performance Centre, profiles and telemetry |
| Bluetooth | Reuse Vita stack and known controller support | Device classification, user categories, input learning and adapters |
| Adrenaline | Keep PSP/PS1 runtime and configuration separate | Quick Menu-only settings page using the existing config codec |
| Plugin management | Do not clone a broad plugin installer catalogue | Inventory, adapters, compatibility rules, previews, backups and recovery |

## Integration rules
1. Reuse a mature upstream engine when its job is clearly bounded.
2. Build adapters only for documented interfaces, known config formats or verified plugin versions.
3. Unknown plugins remain inventory-only. No guessed writes or automatic disable.
4. Menu visibility is distinct from disabling the underlying feature.
5. Risky changes require preview, backup/rollback and post-write verification.
6. Compatibility evidence must record firmware, Vita model, plugin version, taiHEN section, load order and co-installed hooks.
7. Host models/codecs are not live Vita backends; hardware validation is reported separately.

## Target setup constraints
Development target: PS Vita 1000 OLED, firmware 3.65 Ensō. The setup includes reVita and DS34Vita; PSVshell was previously disabled due to memory competition involving reVita and UDCD UVC. Competing performance and USB hooks therefore receive priority in compatibility diagnostics.

## Delivery sequence
1. Plugin Intelligence: extensible compatibility registry, versioned evidence and dependency/conflict reports.
2. Graphics Centre: separate per-title patch, framebuffer scaling and brightness adapters.
3. Rt:ShellBar: one controlled status-bar integration only if it adds value beyond ShellBat.
4. Quick Menu widgets: one tested host; keep Vita-native, VSH and Adrenaline surfaces distinct.
5. Performance Centre: one validated overclock backend and profile switching.
6. Unified profiles: Gaming, Performance, Battery Saver, Streaming, Adrenaline and Custom.

The product goal is one coherent, diagnosable ecosystem—not a pile of overlapping plugins.
