# Rt:Settings Extended Installer

## Product model

Rt:Vita Cockpit is the LiveArea installation, configuration, update and maintenance front end for the Rt ecosystem. It detects installed Rt:Components, lets users select all compatible modules or individual sections, explains dependencies and conflicts, and provides recovery tools.

Rt:Settings Extended is the native Sony Settings experience installed and maintained by Cockpit. It owns the persistent Settings pages and services; it is not a second launcher.

Rt Manager always lives at **Settings → Rt:Settings Extended → Preferences & Settings Management → Rt Manager**.

## Native Settings placement

Features are not forced into one Rt menu. They are placed in existing Sony categories when that is the natural owner and a verified insertion point exists:
- Devices: Bluetooth/controller extensions and non-conflicting storage diagnostics;
- Network: integrations for existing network tools such as Wi-Fi Turbo, without duplicating their pages;
- Sound & Display: validated display and audio controls;
- Rt:Settings Extended: Tai Saver, generic plugin management, compatibility, diagnostics, profiles and cross-system tools.

YAMT retains ownership of its storage-specific controls. HENkaku and other third-party plugins retain ownership of their pages. The full placement and safety matrix is in [Sony Settings placement policy](../docs/SONY_SETTINGS_PLACEMENT.md).

## Package layout

```text
rt-ecosystem/
├── cockpit/                 # VitaSDK LiveArea app and VPK
├── package.json             # package/release manifest
├── modules/<module-id>/     # independently selectable modules
├── framework/payload/       # shared native framework payload
└── installer/               # schemas and package contracts
```

## Runtime split

Cockpit's core is C/VitaSDK. Lua may be embedded for optional, non-critical presentation scripting, but the installer, filesystem transactions, trust verification, taiHEN configuration handling, recovery and SUPRX lifecycle must not depend on a separately installed Lua runtime.

The Cockpit startup animation is distinct from an Ensō boot animation. The latter is managed only by an optional boot adapter that validates the format and engine, backs up current files and requires a recovery path.

## Installer behavior

The intended native integration is a user-mode taiHEN SUPRX loaded under the Settings title `*NPXS10015`, with PAF XML adapters that preserve stock Sony menus and add Rt-owned pages. See [Native Sony Settings integration](../docs/NATIVE_SETTINGS_ADAPTER.md).

The write-capable installer must:
1. validate package and module manifests;
2. check firmware, framework compatibility, dependencies and conflicts;
3. inspect the existing installation and identify Rt-owned files;
4. show a complete change plan before writing;
5. stage files outside live destinations and verify hashes/free space;
6. back up replaced Rt-owned files and the original taiHEN config;
7. show the exact proposed `*NPXS10015` line and require explicit approval;
8. preserve every unrelated plugin section, line, order and comment;
9. commit staged files and the module registry only after payload verification;
10. roll back from its transaction journal on failure;
11. remove only Rt-owned config lines and files on uninstall.

Never overwrite or delete files not declared as owned by this package.

## taiHEN safety and user data

- Never silently edit `ur0:tai/config.txt` or change existing plugin entries.
- Refuse ambiguous sections, duplicate Rt entries or malformed config rather than guessing.
- Preserve settings and archives during updates and module removal.
- Framework uninstall preserves Tai Saver archives by default.

## Implementation status

The portable framework library and the first Cockpit read-only audit shell are separate deliverables. The initial Cockpit shell detects registry records and displays component details; it does not yet install modules or edit taiHEN.

The complete transaction engine, trusted online update index, Rt SUPRX, embedded native page resources, write-capable first module and physical-console validation remain implementation gates. A VPK must be described by its actual capability: a Cockpit shell VPK is not yet the full Rt ecosystem installer.
