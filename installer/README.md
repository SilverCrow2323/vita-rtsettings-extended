# Rt:Settings Extended Installer

## Product model

Rt:Settings Extended ships as one expandable maintenance installer. The installer owns the framework and discovers feature modules from the package manifest. New Settings pages are added as modules in later releases; users do not need a different installer for each feature.

The VPK is not the user-facing Settings experience. Once installed, daily use happens inside the Sony Settings application.

## Native Settings placement

Features are not forced into one Rt menu. They are placed in existing Sony categories when that is the natural owner and a verified insertion point exists:
- Devices: Bluetooth/controller extensions and non-conflicting storage diagnostics;
- Network: integrations for existing network tools such as Wi-Fi Turbo, without duplicating their pages;
- Rt:Settings Extended: Tai Saver, generic plugin management, compatibility, diagnostics, profiles and cross-system tools.

YAMT retains ownership of its storage-specific controls. HENkaku and other third-party plugins retain ownership of their pages. The full placement and safety matrix is in [Sony Settings placement policy](../docs/SONY_SETTINGS_PLACEMENT.md).

## Package layout

```text
rtsettings-extended/
├── package.json
├── modules/tai-saver/module.json
├── framework/payload/
└── installer/
```

## Installer behavior

The intended native integration is a user-mode taiHEN SUPRX loaded under the Settings title `*NPXS10015`, with PAF XML adapters that preserve stock Sony menus and add Rt-owned pages. See [Native Sony Settings integration](../docs/NATIVE_SETTINGS_ADAPTER.md).

The installer must:
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

The package contract is specified, and the portable framework library cross-builds with VitaSDK in CI. That library is not a SUPRX and is not an installable VPK.

The actual Rt SUPRX, embedded native page resources, installer UI, config transaction engine, VPK packaging and physical-device validation are still implementation gates. Do not distribute an empty placeholder VPK or label a portable-library artifact as an installer. A VPK release must contain a working Settings integration and a tested maintenance installer.
