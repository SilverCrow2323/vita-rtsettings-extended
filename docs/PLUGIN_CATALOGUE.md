# Curated Vita Plugin Catalogue

**Snapshot:** 26 November 2026  
**Target profile:** PS Vita 1000 OLED · firmware 3.65 · Ensō · YAMT + SD2Vita

This is the first normalized, machine-readable shortlist—not a dump of every plugin ever published. Five high-priority release packages now include upstream asset URLs and SHA-256 digests, copied from GitHub release metadata. It records selected upstream projects, release versions, firmware evidence, dependencies, conflicts, target-specific caveats and future adapter IDs.

- Machine-readable data: [`data/plugin-catalogue.json`](../data/plugin-catalogue.json)
- Validation schema: [`schemas/plugin-catalogue.schema.json`](../schemas/plugin-catalogue.schema.json)
- Discovery and source policy: [Plugin Research](PLUGIN_RESEARCH.md)

## Status summary

| Status | Meaning |
|---|---|
| Recommended candidate | Strong fit and upstream release; still requires our own install/regression validation |
| Conditional | Useful, but hardware/backend/safety checks are required first |
| Optional | Specialist feature, never installed by default |
| Dependency | Installed only for a selected dependent plugin |
| Alternative | Competing implementation; not installed alongside the selected provider |
| Review | Discovered but not approved |

## Curated entries

| Plugin | Version | Area | Status for this project | Firmware 3.65 evidence |
|---|---:|---|---|---|
| Wi-Fi Turbo | 0.7.2 | Connectivity | Recommended candidate | Explicitly hardware-tested on 3.65 |
| vitastream | 2.3 | USB streaming | Recommended candidate | Offline validated; physical retail test documented on 3.60 only |
| Turbo Storage | 0.4.0 | Storage | Conditional | 3.65 is alpha; old 0.3.0 user report was on PCH-1100, not our PCH-1000 |
| Bluetooth Turbo | 0.2.0 | Bluetooth | Conditional | Declared 3.60–3.74; physical test on 3.60 only |
| VitaBrightEX + LUT Editor | 1.3 + 4.2 | Display | Recommended candidate | NID-based 3.60–3.74+; exact target panel/configuration still to validate |
| PSVshellPlus | 1.4 | Performance | Optional | Release is mature but dated; test firmware and conflicts |
| reVita-extended | 1.1 | Input | Optional | Separate alternative; firmware/package review pending |
| FTP for Vita | 1.0.0 | Connectivity | Recommended candidate | Firmware matrix not independently verified; QuickMenuReborn 2.5 required |
| Caffeine for Vita | 1.0.1 | Power | Optional | Firmware matrix not independently verified; QuickMenuReborn 2.5 required |
| LiveArea NoLimits | 2.1.0 | Shell | Optional | Check upstream release before packaging |
| Screenshot NoShard | 1.2.0 | Shell | Optional | Check path-redirection conflicts |
| rePatch reLoaded | 2.1 | Modding | Optional | Game/title-specific utility, not a Settings control |
| kubridge | 0.3.1 | Runtime | Dependency | Install only for declared dependents |
| ioPlus | 0.1 | Runtime | Dependency | Install only for declared dependents |
| YAMT | Existing installation | Storage | Preserve as baseline | Target setup already uses YAMT; detect exact installed build |
| StorageMgr | 2.0 | Storage | Alternative | Never install alongside YAMT |

## Important target-specific decisions

### Connectivity

- **Wi-Fi Turbo 0.7.2** is the strongest first adapter candidate. Firmware 3.65 is explicitly listed as tested. Keep-Wi-Fi-on is not a harmless always-on switch: it prevents automatic standby, affects battery use and changes sleep behavior.
- **FTP for Vita 1.0.0** is a QuickMenuReborn add-on, not an always-running daemon. It starts disabled after reboot. It conflicts with FTP Everywhere because both bind port 1337.
- **vitastream 2.3** is the preferred USB video/audio candidate. Do not install it alongside `udcd_uvc`, VitaUSBStream or another UVC kernel driver. The project still needs a physical 3.65 test.

### Storage

- Keep the installed **YAMT** setup. Inventory its config and mount mappings before any changes.
- **StorageMgr** remains a mutually exclusive migration alternative, not a second storage plugin.
- **Turbo Storage 0.4.0** is not approved for default installation on this PCH-1000/3.65/YAMT combination. The latest release was physically tested on PCH-1000/3.60; its 3.65 line is not hardware-verified on our target. Keep acceleration off until a controlled test and backup.

### Bluetooth

- **Bluetooth Turbo 0.2.0** has an attractive feature set but only a 3.60 retail hardware test is documented. 3.65 must be treated as unverified.
- AAC, aptX and LDAC are separate optional add-ons, not mandatory files. The base plugin works without them.
- Other Bluetooth patches must be conflict-reviewed before simultaneous activation.

### OLED display

- **VitaBrightEX 1.3** is the preferred candidate for the OLED target over blindly installing the older original Vitabright line. Its latest release explicitly fixes the OLED LUT/filter mistake; OLED filter controls are intentionally restricted to safe operations. Do not apply LCD colour-filter assumptions to the PCH-1000.

## Exclusions and variants

- Do not bundle **LOLIcon**, **NoPowerLimits** or multiple clock controllers beside PSVshellPlus. One overclock control stack at a time.
- Do not install **YAMT** and **StorageMgr** together.
- Do not install multiple USB UVC kernel drivers together.
- Do not install **FTP Everywhere** together with **FTP for Vita**.
- Do not install **NoSleep** together with **Caffeine for Vita**.
- Keep original **reVita** and **reVita-extended** as alternative variants only if a feature/compatibility comparison proves a user-visible difference. Never load both.
- Keep Vita plugins, PSP/Adrenaline plugins, title-specific patches and standalone VPK utilities in separate catalogues. They must not be mixed into the native taiHEN plugin inventory.

## How this catalogue becomes an installer

Every record has a stable ID and can later be enriched with:
- exact asset names, SHA-256 digests and release tag;
- supported device models and firmware evidence;
- dependency graph and mutual-exclusion groups;
- config-file mutations and rollback plan;
- adapter ID and supported settings schema;
- last verification timestamp and maintainer activity.

The installer must resolve dependencies, inspect existing files/configuration, stage files, verify SHA-256 hashes, commit config changes atomically, write a transaction journal and support rollback. The current JSON stores hashes for Wi-Fi Turbo, vitastream, Turbo Storage, Bluetooth Turbo and VitaBrightEX. Missing digests must be calculated and verified before an installer can accept those packages. The plugin manager may inventory unknown plugins, but it must never invent settings controls for them.

**Review state:** candidate data only. No plugin binaries have been downloaded or installed by this milestone, and no physical Vita test is claimed.
