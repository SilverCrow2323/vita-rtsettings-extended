# Plugin Conflict Prevention — rule baseline

The Plugin Compatibility Engine is a read-only evaluator. This milestone expands its built-in baseline; it does not edit taiHEN, install, enable, disable or remove any plugin.

## Blocking rules

A blocking finding prevents an activation plan from being committed:

| Pair / condition | Result |
|---|---|
| YAMT + StorageMgr | Block |
| VitaStream + UDCD UVC | Block |
| VitaStream + Vita USB Stream | Block |
| FTP for Vita + FTP Everywhere | Block |
| Caffeine for Vita + NoSleep | Block |
| PSVshell Plus + LOLIcon | Block |
| PSVshell Plus + NoPowerLimits | Block |
| reVita Extended + reVita Original | Block |
| FTP for Vita without QuickMenuReborn | Block (missing dependency) |
| Caffeine for Vita without QuickMenuReborn | Block (missing dependency) |

## Review-required rules

These pairs are surfaced for explicit review rather than automatically rejected because behavior can depend on exact releases, load order and configuration:

- QuickMenuReborn + Quick Menu Plus
- PSVshell + LOLIcon
- PSVshell + Oclock Vita
- LOLIcon + Oclock Vita
- VitaBright EX + another brightness LUT provider
- Bluetooth Turbo + another Bluetooth audio hook

A review finding is not an assertion that every version pair is incompatible. The activation UI must keep the combination disabled until the user reviews the evidence and confirms the exact versions/configuration. A future evidence database can promote a pair to Block only after reproducible validation.

## Safety boundary

- Rules use stable plugin IDs, never translated labels or guessed file paths.
- Conflict matching is symmetric.
- Unknown plugin IDs remain unclassified; they are not presumed safe.
- A clean report means only that no registered rule matched.
- No static evaluator can guarantee absence of unknown interactions.
- The current engine does not yet parse live taiHEN config or write config previews. Those belong to the separate transactional TaiHEN Config Manager milestone.
- This engine is not yet surfaced as a native Sony Settings page or connected to the Cockpit installer transaction.

## Validation

The host test suite covers the new Quick Menu and performance review pairs in addition to the existing blocking rules, dependency checks, symmetric duplicate rejection and safe baseline case.
