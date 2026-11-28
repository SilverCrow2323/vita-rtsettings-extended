# Native Settings Integration — Implementation Contract

## Status

This is the integration contract for Rt:Settings Extended. It separates what the project wants from what has actually been demonstrated on the target firmware.

## User-facing target

```text
Settings
└── Rt:Settings Extended
    └── Tai Saver
```

The first release adds a dedicated Rt:Settings Extended category and a Tai Saver subpage. It does not replace Sony settings and does not inject arbitrary controls into Sony-owned pages.

## Integration phases

### Phase A — dedicated category and pages

- Register one Rt:Settings Extended entry in the existing Settings application.
- Open a dedicated root page listing installed feature modules.
- Register Tai Saver as the first module.
- Keep module metadata separate from the backup engine.
- Hide or disable unavailable actions with an explicit reason; never display fake success.

This is the first native UI target.

### Phase B — module registry

Each module declares:
- stable module ID;
- English display name and description;
- page factory / page identifier;
- settings schema version;
- service capability flags;
- minimum framework version.

The framework owns navigation and lifecycle. Modules own their settings and service calls. A module must not directly patch another module's data.

### Phase C — extending existing Sony pages

Adding individual controls inside Sony-owned pages is a separate, compatibility-sensitive feature. It requires page-construction hooks, stable control identifiers, callback ownership and lifecycle testing for each target page.

Do not treat page regrouping as proof that individual rows can be injected into arbitrary Sony pages. Do not move or rewrite Sony-owned controls. Phase C is not a dependency for the first release.

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

Configuration writes must be transactional: validate a complete new document, write a temporary file, sync it, then rename. On malformed settings, preserve the original file and load safe defaults with a visible warning.

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

## Research and validation gate

Before implementing the native page, audit the selected Settings-page registration mechanism and record:
1. exact target process/module and plugin load section;
2. page registration API or hook points;
3. ownership and lifetime of page/control objects;
4. callback threading rules;
5. cleanup behavior when Settings closes;
6. firmware-specific differences;
7. a test that opens and closes Settings repeatedly without leaks or crashes.

The project may use a proven page-registration framework as a reference, but must not copy assumptions from a plugin regrouping feature into a general-purpose Sony-page injection claim.

## Acceptance criteria

- [ ] Rt:Settings Extended appears as a dedicated Settings category.
- [ ] Tai Saver opens as a dedicated page.
- [ ] Navigation returns cleanly to Sony Settings.
- [ ] Settings remains responsive during backup/verify/restore.
- [ ] Every action calls the shared C service API.
- [ ] Invalid configuration is handled without overwriting the original.
- [ ] Unsupported firmware fails closed.
- [ ] Disabling/removing the framework restores the original Settings behavior.
- [ ] No Sony-owned setting is moved, replaced or silently modified.
