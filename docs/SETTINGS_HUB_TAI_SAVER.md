# Settings Hub — Tai Saver vertical slice

## Navigation contract

The first feature tree is registered by `rtp_metadata_register_tai_saver()`:

```text
Rt:Settings Extended
└── Tai Saver
    ├── Protection
    │   ├── Automatic Backup
    │   ├── Back Up Now
    │   └── Last Backup Result
    ├── Automatic Backup Policy
    │   └── Automatic Retention
    ├── Snapshot Archives
    │   ├── Automatic Snapshots
    │   ├── Known-Good Archives
    │   ├── Failed / KO Archives
    │   └── Pre-Restore Archives
    └── Compare & Recovery
        ├── Compare Snapshots
        ├── Verify Snapshot
        └── Restore Snapshot
```

All entries have stable IDs, explicit parent categories, deterministic ordering, user-facing descriptions and icon keys. Mutating/operational rows are marked as requiring a backend. The last backup result is explicitly read-only. Actions are represented as actions rather than misleading toggles.

## Safety boundaries

- Automatic Backup controls only the future backup service; it must not edit `ur0:tai/config.txt`.
- Retention applies only to verified Automatic snapshots.
- Known-Good, Failed / KO and Pre-Restore archives are protected from automatic rotation.
- Restore requires a verified source snapshot and a verified Pre-Restore safety snapshot.
- The Settings model does not invoke filesystem operations by itself. Each action must be bound to a concrete backend and report its result.

## Current implementation boundary

This milestone adds the actual Settings registry and metadata tree to the existing renderer-neutral navigation model. It is not yet a Sony Settings application extension and does not render a native Vita screen. No taiHEN registration hook, Sony Settings binary patch, Vita VPK or physical-console test is claimed by this document.

The next integration gate is the native Settings surface adapter and its installation mechanism. It must be demonstrated against the real Settings application before this tree is described as integrated into Sony Settings. A VitaSDK cross-build job is being added for the portable framework library; this is not an application or VPK build.
