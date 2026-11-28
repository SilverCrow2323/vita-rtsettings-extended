# Rt Manager — native Settings menu contract

## Permanent placement

The menu registry now defines this stable hierarchy:

```text
Settings
└── Rt:Settings Extended
    └── Preferences & Settings Management
        └── Rt Manager
            ├── Plugin Control Centre
            │   └── Plugin Conflict Prevention
            ├── Tai Saver
            ├── Settings Layout
            └── Component Status
```

These are stable menu IDs and user-facing metadata. They are not plugin filenames and are not LiveArea bubbles. The Rt Manager entry belongs inside Sony Settings at the requested location.

## Availability and backend gating

Entries marked `REQUIRES_BACKEND` must be hidden or shown unavailable until the owning module reports a compatible installed backend. Metadata registration alone must never expose an operational control, modify taiHEN or create a false success state.

The hierarchy is deliberately separate from Cockpit: Cockpit remains the installation/maintenance application; Rt Manager is the everyday Settings surface.

## Implementation boundary

This milestone adds the renderer-neutral menu hierarchy and host test coverage. It does **not** implement the Settings-title SUPRX, verified Sony Settings insertion hook, taiHEN installation transaction or Vita-native renderer. Therefore this commit does not yet produce a VPK that inserts these rows into Sony Settings. The SUPRX and installer must be completed and tested before a package may claim to install native Settings entries.

## Required native integration gate

1. Build a user-mode SUPRX for `*NPXS10015` against the target VitaSDK/toolchain.
2. Use a documented and firmware-verified Settings extension point; do not replace Sony root XML or patch unknown NIDs.
3. Render only registered entries whose module and backend are installed and compatible.
4. Keep Rt Manager nested under Preferences & Settings Management.
5. Preserve Sony-owned entries and the existing taiHEN configuration; use a backup, preview, atomic write and rollback for any owned config change.
6. Package the SUPRX and its manifest as a Cockpit-managed component, with exact owned paths and verified payload hashes.
7. Test on a PS Vita 1000 OLED running 3.65 Ensō; until then, report host tests separately from hardware validation.
