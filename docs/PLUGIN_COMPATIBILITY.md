# Plugin Compatibility and Conflict Engine

## Purpose

This host-testable engine evaluates a proposed set of enabled plugin IDs against explicit dependency and conflict rules. It is a safety gate for future install, enable and service workflows—not a plugin installer and not a taiHEN configuration editor.

## Result severity

- **Block**: a known dependency is missing or two known mutually exclusive implementations are selected. The caller must not apply the proposed change.
- **Review**: the combination may work, but both plugins touch a sensitive shared subsystem. Require a human compatibility review and target-specific evidence before enabling together.

The engine reports findings only. It never chooses which plugin to remove, edits configuration, installs files or claims that a plugin is currently running.

## Initial rules

| Plugin / feature | Related plugin / feature | Result | Reason |
|---|---|---|---|
| YAMT | StorageMgr | Block | Competing SD2Vita/storage mount providers |
| vitastream | UDCD UVC | Block | Competing USB Video Class kernel backends |
| vitastream | VitaUSBStream | Block | Competing USB video backends |
| FTP for Vita | FTP Everywhere | Block | Both claim FTP port 1337 |
| Caffeine for Vita | NoSleep | Block | Overlapping suspend-prevention behavior |
| PSVshellPlus | LOLIcon | Block | Competing CPU/GPU frequency managers |
| PSVshellPlus | NoPowerLimits | Block | Competing performance/clock control |
| reVita-extended | Original reVita | Block | Overlapping controller remapping |
| VitaBrightEX | Other brightness/LUT driver | Review | Shared display pipeline / panel LUT |
| Bluetooth Turbo | Other Bluetooth audio stack hook | Review | Shared Bluetooth audio stack |

FTP for Vita and Caffeine for Vita both require QuickMenuReborn. A missing dependency is a blocking finding.

## Evaluation contract

See `include/plugin_compatibility.h`:

- Input is a set of plugin IDs believed to be enabled.
- Output is a bounded list of findings with type, severity and both involved IDs.
- The function is deterministic and has no filesystem or platform dependencies.
- Repeated plugin IDs do not create repeated findings.
- Unknown plugin IDs are ignored unless a future explicit rule references them. This is intentional: unknown plugins must not be assigned guessed behavior.

## Important boundaries

- A conflict rule is not proof that the plugins are installed or active; input must come from a trusted inventory/state source.
- The initial rules cover only conflicts supported by concrete project research. Generic catalogue prose such as “other plugins that hook the same stack” is not treated as an exact identity.
- Missing dependencies are evaluated against the supplied enabled-ID set. The caller must decide whether a dependency is already installed and loaded from another taiHEN section.
- Firmware, device model, plugin version, binary hashes and runtime state are not yet evaluated.
- No automatic conflict resolution or config editing is included.

## Target profile

For the project's PCH-1000 OLED / 3.65 Ensō / YAMT + SD2Vita target, YAMT remains the selected storage provider. StorageMgr is an alternative and must never be enabled alongside it. Turbo Storage is not declared conflicting with YAMT by this engine; its 3.65 + PCH-1000 + YAMT combination remains conditional and requires separate compatibility validation before activation.
