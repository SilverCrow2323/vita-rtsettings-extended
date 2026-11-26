# Rt:Settings Extended Installer

## Product model

Rt:Settings Extended ships as one expandable installer package. The installer owns the framework and discovers feature modules from the package manifest. New Settings pages are added as modules in later releases; users do not need a different installer for each feature.

The first package contains the framework shell and Tai Saver. Future modules can be added by publishing a module descriptor and its payload. Example future features are not shipped until implemented.

## Package layout

```text
rtsettings-extended/
├── package.json
├── modules/tai-saver/module.json
├── framework/payload/
└── installer/
```

## Installer behavior

The eventual Vita installer UI will provide fresh install, add modules, independent framework/module updates, disable/remove module, repair owned files and a reviewable change list. Module selection is additive: new features appear as optional modules in later releases.

## Transaction model

1. Validate package and module manifests.
2. Check firmware, framework compatibility, dependencies and conflicts.
3. Inspect the existing installation and identify files owned by Rt:Settings Extended.
4. Build a complete change plan and show it to the user.
5. Stage files outside live destinations and verify hashes/free space.
6. Journal the transaction and back up owned files that will be replaced.
7. Commit staged files, then write the installed-module registry last.
8. On failure, roll back from the journal. If rollback is incomplete, preserve all evidence and report Recovery Required.

Never overwrite or delete files not declared as owned by this package.

## taiHEN safety and user data

- Never silently edit `ur0:tai/config.txt` or change existing plugin entries.
- If a future module needs a taiHEN entry, show the exact proposed line and require explicit confirmation. Back up the original first.
- Preserve settings and archives during updates and module removal.
- Framework uninstall preserves Tai Saver archives by default.

## Module lifecycle

Each module declares a stable ID, version, framework requirement, dependencies, conflicts, page entry, payload file ownership, settings schema and data-preservation policy. Incompatible modules remain uninstalled; partial installs are not accepted.

## Implementation status

This directory defines the expandable package contract and the first Tai Saver descriptor. It is not yet a compiled Vita installer VPK. The on-device installer UI, transaction engine, VitaSDK build and hardware validation remain separate implementation gates.
