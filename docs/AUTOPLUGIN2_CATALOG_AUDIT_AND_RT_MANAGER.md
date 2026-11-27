# AutoPlugin II catalog audit and Rt feature selection

Research date: 27 November 2026  
Target: Rt:Settings Extended, PS Vita 1000 OLED, firmware 3.65 Ensō  
Primary reference: https://github.com/ONElua/AutoPlugin2

## User-approved product placement

**Rt Manager is always part of the native Sony Settings application**, under:

**Settings → Rt:Settings Extended → Preferences & Settings Management → Rt Manager**

It is not a separate LiveArea bubble, a separate homebrew launcher, or a VitaDeploy-launched standalone application. The VPK is the installer/repair/uninstaller only. The native Settings plugin provides the persistent UI.

The feature catalogue may expose links to other Sony categories when appropriate, but Rt Manager itself stays under Preferences & Settings Management.

## 1. Scope of the AutoPlugin II audit

The current AutoPlugin II README describes its supported lists as the active entries in:
- AUTOPLUG2/plugins/plugins.lua — Vita catalogue;
- AUTOPLUG2/plugins/plugins_psp.lua — PSP/Adrenaline catalogue.

At the inspected repository revision, the Vita catalogue contains **160 entries across kernel, shell/main and title-specific sections**. This count includes selectable variants and alternate builds as separate entries; it is not 160 unique capabilities. The separate PSP/Adrenaline catalogue contains 11 game-specific remaster entries, not general Vita system utilities. Dependencies such as ioPlus and kuio are represented as dependencies, not separate catalog entries. The README says commented-out or unlisted resource files are not advertised as supported entries.

The catalog and code were inspected directly:
- [AutoPlugin II README](https://github.com/ONElua/AutoPlugin2/blob/master/README.md)
- [Vita catalog](https://github.com/ONElua/AutoPlugin2/blob/master/AUTOPLUG2/plugins/plugins.lua)
- [PSP/Adrenaline catalog](https://github.com/ONElua/AutoPlugin2/blob/master/AUTOPLUG2/plugins/plugins_psp.lua)
- [Plugin manager](https://github.com/ONElua/AutoPlugin2/blob/master/AUTOPLUG2/scripts/psvita/pmanager.lua)
- [taiHEN config library](https://github.com/ONElua/AutoPlugin2/blob/master/AUTOPLUG2/scripts/tai.lua)

This is an audit of the active catalog, not a claim that every upstream binary has been tested on this user's console or that every plugin is appropriate to bundle.

## 2. Functional inventory of the Vita catalog

The following groups cover the 160 active Vita entries. Variant families are grouped together where the catalog lists multiple builds.

### A. Storage, mount and filesystem
- YAMT-Vita, StorageMgr, usbmc and usbmc Mod;
- EmergencyMount;
- vita-turbo-storage;
- FdFix;
- reSavedata.

**Rt value:** very high. Detect storage configuration, active mount provider, mount points, free space, missing mount targets, config conflicts and plugin dependencies. Provide a read-only storage report and safe links into Sony Settings → Devices → Storage Devices. Never silently switch the user's storage driver.

### B. Plugin runtime, compatibility and dependency management
- ioPlus, kuio;
- 0syscall6, reF00D;
- rePatch, rePatch reLoaded and alternate rePatch builds;
- NoNpDrm / unofficial NoNpDrm, NoPsmDrm;
- NoPspEmuDrm variants;
- Kubridge;
- iTLS-Enso;
- VitaPresence;
- VitaBtFix;
- Vita3g-disable;
- vshPatch;
- CapUnlocker;
- reAuth;
- plugin variants with mutually exclusive files.

**Rt value:** highest. Build a plugin inventory that parses the actual taiHEN config, detects missing files, duplicate entries, repeated sections, incompatible variants, dependencies, mutually exclusive plugins and suspicious paths. Explain what requires a reboot versus a taiHEN reload. Do not automatically install DRM/entitlement bypass plugins or alter account/authentication behavior. Do not ship third-party binaries.

### C. Performance, overclock, CPU and frame pacing
- PSVshell, PSVshell Mod, PSVshellPlus;
- LOLIcon, lolita500, lolita444;
- NoPowerLimits;
- Cpu Affinity Changer;
- CoreUnlocker80000H;
- Oclock Vita;
- Framecounter;
- BetterTrackPlug;
- RapidMenu;
- LowMemMode;
- VitaGrafix and Sharpscale;
- X1Vita.

**Rt value:** high as a compatibility/status layer, not as a competing overclock driver. Detect active performance plugins, identify mutually exclusive controllers, show installed versions and warn before changing kernel plugin combinations. A future unified performance page may expose settings only through a supported upstream API or an explicitly implemented Rt adapter. Do not write undocumented kernel values.

### D. Controllers, Bluetooth, input and accessories
- ds3vita, ds4vita, ds34vita;
- ds4touch;
- DS3/MiniVitaTV support;
- DSMotion;
- VitaControl, VitaPad, VitaKeyboard;
- viimote;
- vitastick, tvikey;
- VitaBtFix;
- VolLightbarCtrlWithDS4;
- System-Integrated DualSense Driver (PS TV-focused);
- TurboPad Mod;
- AnalogStickDisable variants and AnalogsEnhancer;
- Magic Automatic Fake User Input.

**Rt value:** high. Place supported user-facing controls and controller diagnostics under Sony Settings → Devices. Rt Manager should detect driver combinations, duplicate input hooks, missing companion plugins and device-specific limitations. Do not enable multiple drivers for the same controller class automatically.

### E. Display, OLED/LCD, color, brightness and scaling
- Vitabright;
- noled;
- Staybright;
- Sharpscale;
- LCD Colour Space Changer;
- reScreeny;
- reRescaler;
- VitaGrafix;
- VideoDebug;
- Vflux;
- Volume Control Pack;
- psp2wpp.

**Rt value:** high. Display a capability/compatibility matrix under Sony Settings → Sound & Display. Provide safe detection and links; expose actual controls only when the owning plugin's settings interface is known and validated. Device-specific OLED/LCD differences must be explicit.

### F. USB, streaming, remote access and network
- udcd_uvc variants (including OLED/LCD builds);
- vitastick;
- vitacompanion;
- ftpeverywhere;
- Wi-Fi Turbo;
- InfiniteNet;
- VitaPresence;
- USBDisable;
- Adrenaline USB enabler;
- Vita3g-disable;
- iTLS-Enso.

**Rt value:** high. Provide network/USB status, active plugin detection, conflict warnings and safe shortcuts under Sony Settings → Network or the relevant system category. FTP/USB services must show whether they are active and how to stop them; do not silently expose services on boot.

### G. Shell, LiveArea and user-interface behavior
- LiveArea NoLimits;
- QuickMenuReborn variants and Quick Menu Plus;
- Shellbat variants and Shellsecbat;
- Download Enabler;
- NoLockScreen;
- NoTrophyMsg;
- Custom Warning;
- Custom Boot Splash;
- BootSound;
- TransparentImpose;
- PasteUnlock;
- Quick Power;
- Quick Volume;
- dt2s-vita / DoubleTap2Sleep;
- Fruitpeel;
- DolcePolce;
- pngshot, QAscreenshot, Quietshot, PSOneScrot, TrophyShot;
- Music Premium;
- NoAVLS;
- Monaural;
- volume_profile;
- NoSharedFb.

**Rt value:** medium-high. Inventory and explain shell modifications, detect overlapping Quick Menu replacements and duplicate statusbar/battery plugins, show screenshot behavior and startup impact. Settings shortcuts may live in Sound & Display or Preferences & Settings Management depending on the operation. Do not inject a second Quick Menu or statusbar implementation.

### H. Save data, screenshots, trophies and capture
- déjàvu SaveState;
- reSavedata;
- PSOneScrot;
- pngshot;
- QAscreenshot;
- Quietshot;
- TrophyShot;
- NoTrophyMsg;
- Dumping addcont IDs.

**Rt value:** medium. Rt can provide save-data health/backup status, storage space, screenshot destination and installed capture plugin status. Do not implement save-state manipulation or trophy changes in the first release. Trophy/DRM manipulation is excluded from Rt's management scope.

### I. PSP/Adrenaline and game-specific entries
- Adrenaline kernel fix and Adrenaline USB enabler;
- PSP emulation DRM compatibility variants;
- PocketStationUnlocker;
- PSP/Adrenaline remaster catalog: 11 game-specific entries (Ratchet & Clank Size Matters, GTA, Kingdom Hearts, Metal Gear Solid, Prince of Persia, Resistance, Splinter Cell, Tomb Raider, Warriors and Tony Hawk variants).

**Rt value:** selective. Provide a PSP/Adrenaline compatibility report and plugin detection. Game-specific remaster patches belong in an optional game-mod catalogue, not in the core Rt Settings manager. DRM-bypass and trophy/score manipulation functions are excluded.

### J. Title-specific, experimental, or high-risk
- VitaCheat variants;
- TropHAX, ScoreHax;
- HonRipper;
- FreePSM / restorepsmdev;
- Alien Shooter DLC unlocker;
- Gravity Rush 544p;
- LiveSky;
- ReHomeArcade;
- vita-ps4linkcontrols;
- VitaGrafix and per-title graphics patches;
- other title-ID-specific plugins.

**Rt value:** low for the core Settings manager. Inventory may identify them, but no install/update/uninstall actions in the initial Rt release. They need per-title requirements and a separate compatibility policy. Cheats, trophy manipulation, DRM circumvention and DLC unlockers are not adopted as Rt features.

## 3. What AutoPlugin II does well (and what Rt should improve)

The source shows several useful engineering patterns:
- parse taiHEN sections into a model rather than editing strings blindly;
- index installed plugin filenames by title section;
- detect missing plugin files;
- match installed files to catalog entries;
- model plugins with multiple paths and sections (kernel + main + Settings SUPRX);
- model shared dependencies such as ioPlus and kuio;
- handle mutually exclusive variants;
- show dependency impact before removing a shared dependency;
- handle special multi-component plugins such as YAMT, Sysident, VitaNearestNeighbour and plugins with a Settings page in NPXS10015;
- support search, selection, install/uninstall and catalog metadata;
- display plugin README/documentation on demand;
- distinguish system-owned entries from removable user plugins;
- support multiple config locations and config repair.

### Important cautions found in the implementation

The current tai.lua contains more than a generic parser:
- when the selected config file is missing, it creates a default template containing HENkaku entries;
- it merges other config files and deletes the merged source files;
- its repair logic normalizes duplicate sections and removes empty title-specific sections;
- its sync path has HENkaku-specific repair behavior;
- its auto-ordering mode reorders a list of selected kernel plugins;
- the current visible tai.sync() implementation writes and verifies a temporary file, but then writes the final target directly and deletes the temp file. This is not equivalent to a fully atomic rename-based replacement despite the changelog's “Atomic config write” wording.

Rt must **not** copy these behaviors wholesale. The user has an established live configuration and must not lose plugins, comments, ordering, alternate config sources or custom sections.

## 4. Rt feature selection

### Priority 0 — core Settings management (first macro-section)

All of these belong under:

**Settings → Rt:Settings Extended → Preferences & Settings Management → Rt Manager**

1. **Plugin Inventory**
   - parse active taiHEN config;
   - show section, path, plugin type, present/missing status and whether it is Rt/Sony/third-party;
   - identify active config source (ur0:tai/config.txt, ux0:tai/config.txt, or other supported source);
   - show duplicates, duplicate title sections, broken paths and missing files;
   - never edit during scan.

2. **Plugin Conflict & Dependency Inspector**
   - identify duplicate drivers and known incompatible alternatives;
   - model multi-file plugins and multiple title sections;
   - show ioPlus/kuio dependents;
   - detect mutually exclusive rePatch, PSVshell and NoPspEmuDrm variants;
   - explain restart/reload requirements;
   - unknown conflict means “unverified”, not “safe”.

3. **taiHEN Config Editor (safe mode)**
   - preview every proposed change;
   - preserve comments, whitespace, CRLF/LF, ordering and unrelated sections;
   - write only Rt-owned changes unless the user explicitly selects a different operation;
   - create a verified backup and rollback journal;
   - no default config generation, no automatic HENkaku repair, no auto-ordering;
   - no cross-volume merge/delete;
   - validate output before commit.

4. **Plugin File Health**
   - verify referenced files exist;
   - compare installed files against catalog metadata only when a trusted hash is available;
   - flag orphaned plugin files as candidates, never auto-delete;
   - identify missing dependency files;
   - show file location, size and verification result.

5. **Install / Update / Disable / Uninstall workflow**
   - support multi-file/multi-section manifests;
   - dependency plan and reverse dependency checks;
   - mutually-exclusive variant confirmation;
   - backup before config/file mutation;
   - disable means remove/comment the exact config entry while retaining file, not delete the file;
   - uninstall removes only explicitly selected plugin-owned files after dependency checks;
   - restart-required indicator;
   - never fetch or install a plugin without user confirmation.

6. **Catalog & Plugin Information**
   - metadata, author, upstream documentation, version, supported firmware/device, files, dependencies and conflicts;
   - use a curated signed/verified manifest for Rt-supported entries;
   - do not mirror the entire AutoPlugin catalog as “supported by Rt”;
   - no third-party binaries bundled in Rt's VPK.

7. **Tai Saver**
   - snapshot config and plugin files;
   - compare snapshots;
   - restore selected files/config with preview;
   - retention and free-space checks;
   - keep restore distinct from ordinary plugin uninstall.

### Priority 1 — useful native Sony category integrations

These are links or feature-specific pages, not duplicate Rt Manager instances:
- **Devices → Storage Devices:** YAMT/StorageMgr detection, mount layout, SD2Vita health, storage free space and Tai Saver destination.
- **Devices → Bluetooth & Controllers:** ds3vita/ds4vita/ds34vita/ds4touch/VitaBtFix conflict and device support report.
- **Network:** Wi-Fi Turbo, FTP/USB streaming and connectivity plugin status.
- **Sound & Display:** Vitabright/Sharpscale/noled/Staybright/LCD and scaling plugin status.
- **System:** boot-time plugin health and recovery guidance.

The category integration should only appear when its page and callbacks are implemented. It must not be a decorative link to non-existent pages.

### Priority 2 — advanced tools

- plugin Settings page discovery/aggregator inspired by Tweaks/Plugins;
- plugin startup timing and boot health report;
- profile sets (e.g. normal, streaming, performance) with dry-run diff and rollback;
- curated online catalog with version checking and signed metadata;
- optional PSP/Adrenaline plugin inventory.

Moving third-party pages is opt-in and later; shortcuts first. Never auto-move custom-widget pages.

### Explicit exclusions

Do not adopt as Rt core features:
- DRM circumvention, fake licenses, DLC unlockers or account/authentication bypass;
- trophy/score manipulation and cheat engines;
- game-specific patches as a general system settings feature;
- experimental kernel tweaks with no validated 3.65 support;
- automatic deletion of unknown/orphaned files;
- automatic taiHEN config reset or reconstruction.

These may be detected and labeled in the inventory when useful for conflict analysis, but Rt does not become an installer for every item in the upstream catalog.

## 5. Rt Manager UI structure

The manager is a native Settings page nested exactly here:

**Settings → Rt:Settings Extended → Preferences & Settings Management → Rt Manager**

Suggested native page hierarchy:

- **Overview**
  - active taiHEN config
  - kernel/user plugin counts
  - missing files / conflicts / dependencies
  - last backup and restore status
- **Installed Plugins**
  - Kernel / main / Settings / title-specific filters
  - search, details, enable/disable
- **Compatibility**
  - conflicts, dependencies, firmware/device warnings
- **Config Manager**
  - view, diff, preview, backup, rollback, repair selected issue
- **Plugin Files**
  - missing/orphaned files, hashes, paths
- **Catalog**
  - curated entries and upstream documentation
- **Profiles**
  - save/apply a reviewed plugin configuration
- **Diagnostics & Logs**
  - scan reports, operation history and export

The first page can be implemented with a small read-only set of native controls. Destructive operations must require explicit confirmation and a recoverable backup.

## 6. Implementation plan

1. Close the existing RTFS migration by moving Boot Journal/startup adapter to rt_fs, testing and documenting it.
2. Build the read-only Plugin Inventory model on host tests first: parse sections, comments, paths, duplicates and multi-section entries without rewriting the source.
3. Add dependency/conflict manifest schema and tests for multi-file plugins.
4. Add safe config diff/backup/rollback transaction logic.
5. Integrate the read-only Rt Manager page under Preferences & Settings Management in native Sony Settings.
6. Build a VPK installer/repair/uninstaller only after the SUPRX and UI exist; it installs Rt-owned files and adds only its own taiHEN line.
7. Add curated plugin actions in staged batches after compatibility and provenance checks.

## 7. Evidence and limits

The catalog count and grouping come from the active Vita catalog at the inspected AutoPlugin II repository revision. AutoPlugin's list changes over time. Some entries are variants, some are title-specific, some are experimental, and catalog presence does not prove compatibility with every firmware or device. The separate PSP list is a game-specific remaster catalogue, not a set of generic system plugins.

This document is a source audit and product-selection plan. It does not claim that Rt Manager, plugin installation, the native Settings page, or a VPK has been implemented. No AutoPlugin II source or binaries have been copied into Rt.
