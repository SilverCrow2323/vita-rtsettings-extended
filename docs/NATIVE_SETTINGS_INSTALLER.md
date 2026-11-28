# Rt native Settings — Cockpit installation

## What the VPK installs

The Cockpit VPK contains two separate pieces:

- `eboot.bin`: Rt Vita Cockpit installer/maintenance UI.
- `rtsettings.suprx`: user-mode extension loaded by taiHEN in Sony Settings (`NPXS10015`).

After successful installation and a Vita restart, the intended menu is:

```text
Settings
└── Rt:Settings Extended
    └── Preferences & Settings Management
        └── Rt Manager
            └── Rt Manager Status
```

The first release exposes the native entry and a read-only installation-status action. It does not yet expose the full plugin catalogue, Tai Saver controls, or operational controls for other components.

## Installation procedure

1. Install the VPK using VitaShell or another VPK installer.
2. Launch **Rt Vita Cockpit**.
3. Select **Native Sony Settings / Rt Manager Installer**.
4. Review the destination, config section and backup behavior.
5. Confirm **Install / Enable**.
6. Wait for the success result and restart the Vita.

The installer expects the existing `ur0:tai/config.txt` and an accessible `ur0:tai` directory. It does not create a replacement taiHEN config.

## What the installer changes

- Copies the VPK's `app0:rtsettings.suprx` to `ur0:tai/rtsettings.suprx`.
- Adds exactly one `ur0:tai/rtsettings.suprx` entry under `*NPXS10015`.
- If that exact entry already exists but is commented out, the explicit Install/Enable confirmation activates it.
- Preserves unrelated sections, plugin lines, comments and original line endings.
- Refuses duplicate Rt lines, duplicate `*NPXS10015` sections, ambiguous config states, stale transaction files and a different existing SUPRX.

## Backups and rollback

Before touching the live config or module path, Cockpit creates a verified, permanent Tai Saver **Pre-Restore** snapshot of `ur0:tai` under:

`ur0:data/RtSettingsExtended/TaiSaver/`

When the config changes, an additional byte-for-byte backup is written under:

`ur0:data/RtSettingsExtended/Installer/`

The installer stages and reads back both the module and new config before replacing the live config. Synchronous failures attempt to restore the previous config and remove only the newly installed Rt module. If power is lost after the old config is moved to the rollback slot, the next installer run attempts to restore it before proceeding.

Do not delete the Tai Saver snapshot or installer backup until the new Settings entry has been verified on the console.

## Compatibility and support boundary

Initial target: PS Vita 1000 OLED, firmware 3.65 Ensō.

The SUPRX is user-mode and belongs under `*NPXS10015`. It does not replace HENkaku, does not install a kernel plugin, and does not rewrite Sony's root XML on disk. It adds the Rt entry to the in-memory Sony root XML only after the required hooks are installed.

A successful CI build is not a physical-console compatibility certification. If the root entry does not appear after restart, do not manually copy random XML files or replace `henkaku.suprx`; collect the taiHEN config and logs for diagnosis.

## Not included yet

- Update/repair of a different existing Rt SUPRX version.
- Uninstall and removal of the Rt-owned taiHEN line.
- Tai Saver backup/restore controls inside Sony Settings.
- Plugin enable/disable, dependency resolution and conflict controls in the native page.
- Verified coexistence testing with the full plugin stack on a PS Vita 1000 3.65 Ensō.
