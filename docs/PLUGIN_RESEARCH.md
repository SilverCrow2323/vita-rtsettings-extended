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

## Normalized catalogue

The first curated, machine-readable dataset now lives in [Plugin Catalogue](PLUGIN_CATALOGUE.md), with JSON records and a JSON Schema. It is intentionally a selected shortlist, not a mirror of every entry in an installer app. The target profile is PCH-1000 OLED / firmware 3.65 / Ensō / YAMT + SD2Vita.

The catalogue separates declared firmware support, offline validation, physical testing and reports from other users. A release that declares 3.65 is not automatically considered tested on our exact hardware.

## Discovery-source comparison

- **AutoPlugin II 2.20** is useful as a discovery/install catalogue and its changelog shows dependency-aware uninstall and safer config writes. It is not treated as the authority for latest upstream versions. Community guidance also warns against trusting automatic installation blindly; Rt will implement its own reviewed transaction and rollback flow. [AutoPlugin II](https://github.com/ONElua/AutoPlugin2)
- **VitaHomebrewDB** provides a broad recovered feed (182 plugin records in the snapshot reviewed), but its preserved feed is not current enough to choose versions by itself. Use it for discovery and source links.
- **GameBrew** is useful for recent additions and release cross-checks, but version tables can lag or disagree with upstream.
- **Brewology** is useful for recent release dates and package manifests; downloads still resolve to upstream author releases where available.
- **PSX-Place** remains a historical discovery/forum source. It is not a substitute for checking the author's repository and tagged release.
- **Author GitHub releases and source** decide the selected version, binary manifest, firmware claims, changelog and checksums.

## First-pass candidates

These are research candidates, not an approved one-click install set. Version selection must be checked against the author's latest release and target firmware before packaging.

| Plugin / family | Area | Initial disposition | Notes |
|---|---|---|---|
| YAMT | Storage | Keep / baseline dependency | User's target already uses YAMT; avoid installing a competing storage driver blindly |
| StorageMgr | Storage | Alternative only | Preserve as a selectable alternative where compatibility or user setup requires it; never run alongside YAMT without verified configuration |
| VitaShell FTP backend / FTP for Vita | Connectivity | Investigate service adapter | Distinguish VitaShell's FTP server from Quick Menu toggles and standalone FTP plugins |
| vitastream | Streaming | 2.3 recommended candidate | Offline firmware validation includes 3.65; retail physical test documented on 3.60 only; mutually exclusive with other UVC kernel drivers |
| VitaUSBStream | Streaming | Alternative backend | Separate implementation; retain only if it provides materially different host compatibility or quality |
| VitaBrightEX | Display | 1.3 + LUT Editor 4.2 recommended candidate | Better fit for OLED/LCD split and OLED panel LUT safety; OLED filter operations are intentionally restricted |
| PSVshellPlus | Performance | 1.4 optional | Latest tagged release found; mature but dated, and must not coexist with other overclock managers |
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
| Wi-Fi Turbo | Connectivity | 0.7.2 recommended candidate | Author explicitly lists 3.65 among physically tested firmware; Keep Wi-Fi on has battery/sleep implications |
| Turbo Storage | Storage | 0.4.0 conditional | Latest release physically tested on PCH-1000/3.60; 3.65 is alpha and prior user report was PCH-1100/v0.3.0; target YAMT interaction unverified |

## Target profile shortlist

See [Plugin Catalogue](PLUGIN_CATALOGUE.md) for all 16 normalized records, versions, adapter IDs, dependencies, conflicts and per-plugin caveats. Key decisions for the PCH-1000 OLED / 3.65 / YAMT setup:

- Wi-Fi Turbo 0.7.2: 3.65 physically tested by the author.
- vitastream 2.3: offline validated for 3.65, physical retail test documented on 3.60 only.
- Turbo Storage 0.4.0: 3.65 alpha; do not enable by default on our target.
- Bluetooth Turbo 0.2.0: declared support 3.60–3.74, physical test only on 3.60.
- VitaBrightEX 1.3: OLED-aware and explicitly fixes the previous OLED LUT/filter issue.

## Selection policy

1. **Upstream first:** author repository/release is the authority for the latest version and requirements.
2. **No blind latest-version rule:** the newest release is preferred only when it is stable and compatible with the target firmware.
3. **Variants stay separate when meaningful:** retain selectable variants for different hardware, firmware ranges, feature sets or upstream maintenance—not merely different filenames.
4. **Dependencies are not promoted as features:** ioPlus, kuio and kubridge are dependency records with automatic resolution.
5. **No duplicate control surfaces:** overlapping overclock, USB, storage, remapping or Quick Menu plugins must be compared and conflict-checked.
6. **Inventory before control:** unknown plugins can be listed, but only plugins with a reviewed adapter receive settings controls.
7. **No nightly builds by default:** use tagged releases; allow development builds only in an explicitly experimental channel.
8. **Every record needs provenance:** upstream URL, release URL, version, date checked, supported firmware, plugin type, dependencies, conflicts, hash and review state.


## Appearance, boot and Quick Menu research pass

A first source-discovery pass has identified these upstream families. This is a discovery shortlist only: it does not yet certify a current binary version, 3.65 compatibility or an Rt configuration adapter.

| Feature | Candidate / source | What is established | Rt decision |
|---|---|---|---|
| Quick Menu framework | [QuickMenuReborn](https://github.com/Ibrahim778/QuickMenuReborn) | Upstream describes a developer library/framework for adding widgets to the Vita Quick Menu. Its install notes require the plugin, a QuickMenuReborn data folder and its RCO resource. | Evaluate as a framework dependency, not as a ready-made universal Settings page. Verify current release/tag, widget APIs and installed add-ons before exposing controls. |
| Quick Menu variants/add-ons | [AutoPlugin II catalogue](https://github.com/ONElua/AutoPlugin2) | The catalogue lists QuickMenuReborn variants/add-ons including DolceWiFi, QuickLauncher and vitaQmBluetooth. | Compare each add-on independently; retain only variants with meaningful feature or compatibility differences. |
| Custom startup warning | [VitaTweaks / Custom Warning](https://github.com/TheOfficialFloW/VitaTweaks) | AutoPlugin II lists Custom Warning by TheOfficialFloW. | Inspect upstream source and actual configuration interface before promising arbitrary message editing. |
| Boot splash | [Custom Boot Splash](https://store.brewology.com/vita/custom-boot-splash/) and [VitaSplashDrop source](https://github.com/Mag-Aegis/VitaSplashDrop) | The documented plugin displays a boot splash image; the original Sony boot logo cannot be hidden by this plugin. Installation involves boot configuration details, not merely a normal user plugin entry. | High-sensitivity boot integration candidate. Verify upstream lineage, current source/release and Enso 3.65 behavior; never overwrite boot configuration without verified backup and rollback. |
| Boot sound | [Vita-BootSound](https://github.com/teakhanirons/Vita-BootSound) | Documentation describes a WAV boot jingle with constraints (1 MiB maximum, up to 48 kHz, stereo maximum) and notes possible System Music/audio-hook issues on firmware above 3.60. | Candidate for asset-management integration, but firmware 3.65 audio compatibility and current maintenance must be checked before adapter approval. |

### What this means for the roadmap

- **QuickMenuReborn** is a framework candidate. Its developer library role must not be confused with a complete user-facing Quick Menu settings API.
- **Custom Warning**, **Custom Boot Splash** and **Vita-BootSound** are separate implementations with different configuration and boot-loading paths. They must not be merged into one generic plugin adapter.
- Boot splash and boot sound changes need pre-change backups, validation, an explicit reboot notice and a recovery path.
- The Custom Assets Manager should be shared by these features, while each plugin retains its own adapter and capability schema.
- The current source discovery does not establish that these projects have the latest working release for firmware 3.65. The selection policy below still requires an upstream release/source audit and target-specific validation.

## Next research pass

Expand the curated dataset in batches by category (network/storage, Bluetooth/input, display/power, shell/game-specific), verify each record against author release assets and checksums, and add a separate PSP/Adrenaline catalogue. Keep the installer catalogue separate from the inventory of everything discovered.

## Verified boot presentation candidates

A second source pass compared the upstream repositories and current discoverable release information. These are candidates, not a declaration that the project has already validated their binaries on the target Vita.

| Candidate | Verified upstream facts | Selection / adapter direction |
|---|---|---|
| [CBS-Manager](https://github.com/Yoti/CBS-Manager) | Upstream README supports a static image, an animation and a console-info animation. It installs CBSM into Ensō boot configuration and taiHEN config, then exposes **Settings → Theme & Background → Boot Screen**. Static and animated files use CBSM/CBANIM formats. It explicitly warns that the initial Sony boot logo cannot be hidden; 3.65-update365 users have a boot_config caveat. | **Preferred combined boot-screen candidate for further validation.** Build a CBSM-specific adapter and converter workflow; back up both boot_config and taiHEN config before install/update/remove. Exact latest binary/release provenance and 3.65-update365 handling remain to be checked. |
| [vita-bootanim](https://github.com/SKGleba/VitaTools/tree/master/vita-bootanim/source) | Community documentation lists v1.1, reports testing on 3.65 and support on Ensō 3.60–3.73. Uses generated `boot.rcf` in `ur0:tai/`; max animation size is 108 MiB and L-trigger can skip faulty animations. | Keep as a dedicated animation alternative if its maintenance/format pipeline offers a benefit over CBSM. Do not install two boot animation engines simultaneously. |
| [Custom Boot Splash](https://github.com/Mag-Aegis/VitaSplashDrop) | Static splash implementation; requires a converted image and boot_config insertion. Original Sony logo remains visible. | Legacy/static alternative only. Prefer CBSM if the same hardware/firmware validation passes; never stack both boot hooks. |
| [Vita-BootSound](https://github.com/teakhanirons/Vita-BootSound) | Latest GitHub release found is 1.0 (2018). WAV-only, 1 MiB maximum, up to 48 kHz and two channels; documented possible System Music audio-hook issue above firmware 3.60. | Not a default recommendation for the 3.65 target. Keep conditional until an actual 3.65 audio test and conflict analysis pass. |
| [Custom Warning](https://github.com/TheOfficialFloW/VitaTweaks) | Upstream README confirms the text is stored in `ux0:tai/custom_warning.txt` as Unicode Little Endian with BOM `FF FE`; empty BOM-only file suppresses text. The plugin replaces the Ensō epilepsy warning. | A concrete text adapter is possible: UTF-16LE+BOM validation, preview, backup/restore and safe atomic write. Verify exact plugin version and whether path can be redirected before supporting ur0/SD2Vita variants. |

### Current version/source findings

- [AutoPlugin II](https://github.com/ONElua/AutoPlugin2) latest release endpoint currently resolves to **2.20**, published 7 November 2026. Its changelog includes Wi-Fi Turbo 0.7.0 and LiveArea NoLimits 2.1.0, plus atomic `tai.sync()` writes and dependency-aware ioPlus uninstall. This is a useful catalogue cross-check, not the source of truth for each plugin's newest upstream release.
- [QuickMenuReborn](https://github.com/Ibrahim778/QuickMenuReborn) latest release endpoint currently resolves to **V2.5**, published 16 May 2022. It remains a developer widget framework, not a general settings API. Its age means add-on-specific support and ABI assumptions must be checked carefully.
- [CBS-Manager](https://github.com/Yoti/CBS-Manager) has no GitHub release returned by the repository's latest-release endpoint during this pass; treat its current branch/source as an untagged candidate, not a versioned stable package.
- [Vita-BootSound](https://github.com/teakhanirons/Vita-BootSound) latest release endpoint resolves to 1.0. No newer tagged release was found in the repository release listing.

### Consequences for Rt

1. Use one shared asset registry and import/backup library.
2. Keep separate adapters for CBSM, vita-bootanim, Custom Warning and BootSound; their formats, config locations and activation mechanisms differ.
3. For CBSM, boot animation and boot splash are capabilities of the same engine, not independent plugins to stack.
4. For BootSound, mark 3.65 compatibility as unverified and default to unavailable until a target test passes.
5. For Custom Warning, support the actual UTF-16LE+BOM file format rather than exposing a generic arbitrary text editor.
6. Do not ship a boot engine merely because AutoPlugin II lists it. Check author source, current artifact, checksum, dependencies and rollback behavior first.

