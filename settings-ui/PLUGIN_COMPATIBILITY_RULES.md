# Plugin Compatibility Rule Registry

The compatibility evaluator supports a bounded caller-owned rule registry. Optional modules and verified adapters can contribute explicit dependency and conflict knowledge without changing the evaluator's central source or editing taiHEN configuration.

- Missing dependency: enabled plugin A without required plugin B emits a blocking finding.
- Conflict: simultaneous A and B emits a blocking or review finding.
- Rule IDs are stable plugin IDs, never display labels or guessed file paths.
- Conflict duplicate detection is symmetric; registering A/B and B/A is rejected.
- Dependencies are blocking by design. A plugin with a missing required component must not be presented as safe to activate.
- The evaluator is read-only. It does not install, enable, disable, remove or rewrite plugins.
- Duplicate enabled IDs do not multiply findings.
- Invalid IDs, malformed rules and capacity exhaustion fail closed.

The built-in catalogue retains the current baseline: YAMT/storageMgr; VitaStream/UDCD UVC and Vita USB Stream; FTP for Vita/FTP Everywhere; Caffeine/NoSleep; PSVshell Plus/Lolicon and NoPowerLimits; reVita Extended/reVita Original; VitaBright EX/other brightness LUT; Bluetooth Turbo/other audio hook; FTP for Vita and Caffeine requiring QuickMenuReborn.

These are explicit project rules, not a universal compatibility guarantee. Firmware, plugin version, model, configuration and load order can change outcomes. Live scanning, binary/version identification, evidence links and hardware validation remain separate milestones.
