# Vita Plugin Research — Initial Source Map

**Research snapshot:** 26 November 2026  
**Scope:** taiHEN/kernel and user plugins relevant to PS Vita / PSTV. PSP/Adrenaline-only plugins, standalone VPK utilities and title-specific patches are tracked separately and are not automatically treated as system plugin candidates.

## Sources and how they are used

| Source | Role | Caveat |
|---|---|---|
| [AutoPlugin II](https://github.com/ONElua/AutoPlugin2) | Active install catalogue and dependency hints | Its selectable entries are not proof of universal compatibility |
| [VitaHomebrewDB](https://drdecki.github.io/VitaHomebrewDB/) | Broad recovered plugin metadata and source/download links | Original VitaDB snapshot is dated 2026-07-31; newer additions are separate |
| [GameBrew plugin list](https://www.gamebrew.org/wiki/List_of_Vita_plugins) | Recent discovery and release/version cross-check | Community-maintained; verify against author releases |
| [PSX-Place Vita plugin resources](https://www.psx-place.com/resources/categories/plugins.11/) | Historical releases, discussion and discovery | Resource pages may lag behind upstream repositories |
| Author GitHub repositories/releases | Primary authority for version, compatibility, changelog and binaries | Prefer tagged releases and source code over mirrors |

## First-pass candidates

These are research candidates, not an approved one-click install set. Version selection must be checked against the author's latest release and target firmware before packaging.

| Plugin / family | Area | Initial disposition | Notes |
|---|---|---|---|
| YAMT | Storage | Keep / baseline dependency | User's target already uses YAMT; avoid installing a competing storage driver blindly |
| StorageMgr | Storage | Alternative only | Preserve as a selectable alternative where compatibility or user setup requires it; never run alongside YAMT without verified configuration |
| VitaShell FTP backend / FTP for Vita | Connectivity | Investigate service adapter | Distinguish VitaShell's FTP server from Quick Menu toggles and standalone FTP plugins |
| vitastream / UDCD USB Video Class | Streaming | High-priority candidate | Recent maintained USB video/audio path; check host support and conflict with other USB modes |
| VitaUSBStream | Streaming | Alternative backend | Separate implementation; retain only if it provides materially different host compatibility or quality |
| VitaBright / VitaBrightEX | Display | Variant family | Keep multiple variants only where OLED/LCD or firmware support differs; verify current upstream before selecting default |
| PSVshellPlus / PSVshell | Performance | High-priority candidate | Prefer current maintained Plus line if feature and compatibility checks confirm it supersedes legacy build |
| reVita / reVita-extended | Input | Variant family | Compare maintained fork, supported devices and feature parity before choosing default |
| rePatch reLoaded | Mod support | Keep | Core data redirection use case; verify dependencies and title compatibility |
| kubridge | Runtime dependency | Dependency, not user-facing tweak | Install only when a selected application/plugin requires it |
| ioplus / ioPlus | Runtime dependency | Dependency, not user-facing tweak | Install only for adapters/plugins that explicitly require it |
| iTLS-Enso | Network/TLS | Optional system component | Separate package/workflow; validate firmware and Ensō requirements |
| QuickMenuReborn / QuickLauncher | UX | Evaluate | Keep only if user wants the alternative Quick Menu ecosystem; check conflicts with other Quick Menu patches |
| Revita | Input/performance | Evaluate | Avoid duplicate overlapping remappers/overclock controls |
| VitaGrafix | Graphics | Evaluate | Per-title graphics patching; not a general Settings control |
| NoPowerLimits / LOLITA | Performance | Do not bundle by default | Legacy/overlapping overclock paths; prioritize one actively maintained and safe interface |
| Caffeine for Vita | Power | Evaluate | Simple sleep prevention; useful only if exposed as a supported service toggle |
| Wi-Fi Turbo | Connectivity | High-priority candidate | Recent QoL plugin; verify exact firmware/hardware support and default behavior |
| Turbo Storage | Storage | High-priority candidate | Recent storage-performance work; do not combine with storage drivers until interaction is verified |

## Selection policy

1. **Upstream first:** author repository/release is the authority for the latest version and requirements.
2. **No blind latest-version rule:** the newest release is preferred only when it is stable and compatible with the target firmware.
3. **Variants stay separate when meaningful:** retain selectable variants for different hardware, firmware ranges, feature sets or upstream maintenance—not merely different filenames.
4. **Dependencies are not promoted as features:** ioPlus, kuio and kubridge are dependency records with automatic resolution.
5. **No duplicate control surfaces:** overlapping overclock, USB, storage, remapping or Quick Menu plugins must be compared and conflict-checked.
6. **Inventory before control:** unknown plugins can be listed, but only plugins with a reviewed adapter receive settings controls.
7. **No nightly builds by default:** use tagged releases; allow development builds only in an explicitly experimental channel.
8. **Every record needs provenance:** upstream URL, release URL, version, date checked, supported firmware, plugin type, dependencies, conflicts, hash and review state.

## Next research pass

Build a normalized machine-readable catalogue from AutoPlugin II's active Vita list and VitaHomebrewDB's plugin records, then deduplicate by upstream project and binary identity. Cross-check all shortlisted entries against author releases and known firmware constraints. Add PSP/Adrenaline plugins to a separate catalogue so they never appear as native Vita taiHEN entries.
