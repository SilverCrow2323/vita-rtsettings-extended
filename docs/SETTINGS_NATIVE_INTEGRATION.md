# Native Settings Integration — Implementation Contract

## Status

This contract defines a hybrid native Settings architecture: features are placed inside existing Sony categories when they naturally belong there and can be extended safely; cross-system tools live under the dedicated **Rt:Settings Extended** category. This is not yet implemented by the Rt SUPRX adapter.

## User-facing target

```text
Settings
├── Devices
│   ├── Bluetooth & Controllers (Rt extension, only after backend validation)
│   └── Storage Devices (YAMT remains owner of its controls; Rt adds non-conflicting diagnostics)
├── Network
│   └── Wi-Fi Turbo (existing upstream page; Rt must not duplicate it)
└── Rt:Settings Extended
    ├── Tai Saver
    ├── Plugin Control Centre
    ├── Plugin Compatibility
    ├── System Diagnostics
    ├── Preferences & Profiles
    └── cross-system tools without a suitable Sony owner
```

The exact Sony XML page identifiers and insertion points must be inspected on firmware 3.65. The labels above describe intended destinations, not guessed internal identifiers.

## Placement rules

- **Devices → Bluetooth & Controllers:** intended for controller inventory and supported device controls. The current Rt model has no live Vita discovery, pairing or input backend; do not expose operational controls yet.
- **Devices → Storage Devices:** YAMT already owns its storage-specific UI. Rt may add read-only diagnostics or non-conflicting actions only after live storage enumeration and a verified insertion point exist.
- **Network → Wi-Fi Turbo:** upstream Wi-Fi Turbo demonstrates this native location. Rt must integrate through its typed adapter and must not create a second page.
- **Rt → Tai Saver:** backup and recovery of taiHEN configuration has no natural Sony category. It remains a dedicated Rt page.
- **Rt → Plugin Control Centre / Compatibility:** generic plugin inventory and compatibility are cross-system functions. Do not hijack HENkaku Settings or third-party plugin pages.
- **Other features:** use an existing Sony page only when the feature genuinely belongs there and the target page has a verified extension point. Otherwise use Rt.

See [Sony Settings placement policy](SONY_SETTINGS_PLACEMENT.md) for the full placement matrix and acceptance gates.

## Integration phases

### Phase A — dedicated category and pages

- Register one Rt:Settings Extended entry in the existing Settings application.
- Open a dedicated root page listing only installed, available feature modules.
- Register Tai Saver as the first module after its live backend and callbacks are complete.
- Keep module metadata separate from the backup engine.
- Hide or disable unavailable actions with an explicit reason; never display fake success.

### Phase B — module registry

Each module declares:
- stable module ID;
- English display name and description;
- page factory / page identifier;
- settings schema version;
- service capability flags;
- minimum framework version.

The framework owns navigation and lifecycle. Modules own their settings and service calls. A module must not directly patch another module's data.

### Phase C — existing Sony page integrations

Each existing Sony page is a separate integration with its own:
- exact target process/module and XML layout identity;
- bounded insertion point and preservation test;
- callback ownership and lifetime;
- firmware-specific compatibility checks;
- hook cleanup and repeated open/close tests.

Never replace an entire Sony XML layout with a reduced copy. Never move or rewrite Sony-owned controls. Do not add a page to Devices or Network merely because the feature sounds related.

## Settings page behavior

Tai Saver's dedicated page is specified in `settings-ui/TAI_SAVER_PAGE.md`. The UI must call the public service API in `include/tai_saver.h`; it must not duplicate filesystem logic.

Long-running operations execute on a worker and report progress/results back to the UI thread. The UI must never block while hashing or copying.

## Configuration ownership

Framework configuration:
- module enable/disable state;
- framework version;
- module registry metadata.

Tai Saver configuration:
- automatic backup enabled;
- Automatic retention count (minimum 2);
- storage root;
- diagnostic verbosity.

Integrity verification and Pre-Restore safety snapshots are mandatory invariants, not user-disableable preferences.

## Error and status model

The UI must distinguish:
- Ready;
- Waiting for storage;
- Backup in progress;
- Backup completed and verified;
- Backup failed;
- Archive verification failed;
- Restore confirmation required;
- Restore in progress;
- Restore completed;
- Restore failed;
- Recovery required.

An archive is eligible for restore only after a fresh verification. A successful backup is not equivalent to Known-Good.

## Compatibility and rollback

- Target first: Vita 1000, firmware 3.65 Ensō.
- Keep the framework plugin independently disableable through taiHEN configuration.
- Never edit taiHEN configuration automatically to install, update or remove Rt:Settings Extended.
- Provide a documented manual removal path and preserve the user's existing plugin entries.
- If Settings crashes or page registration fails, the framework must fail closed without modifying Sony settings data.
- Any hook must be guarded by version checks and a compatibility test; unsupported firmware must leave the original Settings flow untouched.

## Installer and VPK gate

The VPK is a maintenance installer, not a Settings launcher. It may be packaged only when it includes a real SUPRX/resource payload and an installer that validates payload hashes, previews the exact `*NPXS10015` config change, requests explicit approval, preserves unrelated lines, backs up the original, rolls back failures and removes only Rt-owned files on uninstall. A static framework library or empty SUPRX placeholder is not an installable Settings extension.

## Acceptance criteria

- [ ] Rt:Settings Extended appears as a dedicated Settings category.
- [ ] Tai Saver opens as a dedicated page after its backend is complete.
- [ ] Navigation returns cleanly to Sony Settings.
- [ ] Settings remains responsive during backup/verify/restore.
- [ ] Every action calls the shared C service API.
- [ ] Invalid configuration is handled without overwriting the original.
- [ ] Unsupported firmware fails closed.
- [ ] Disabling/removing the framework restores the original Settings behavior.
- [ ] Existing Sony pages retain all original entries and behavior.
- [ ] Every additional Sony-page integration has its own lifecycle and regression tests.
- [ ] VPK install/update/repair/uninstall and rollback pass package-level tests.
- [ ] Physical validation on Vita 1000 / 3.65 Ensō is recorded.
