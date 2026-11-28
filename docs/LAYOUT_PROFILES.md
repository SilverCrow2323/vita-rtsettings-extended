# Layout Profiles — Preferences Milestone 2

## Scope
This milestone adds a host-testable profile store on top of the stable-ID Preferences registry. It is a data/model layer, not the native Settings screen.

## Behavior
- Up to eight named profiles, each storing a snapshot of menu entry IDs, kinds, parent IDs, visibility and ordering.
- Create a profile from the current layout; update an existing profile from the current layout.
- Rename and delete profiles. The reserved `default` profile cannot be renamed or deleted.
- Apply a profile transactionally to the current registered catalogue.
- Unknown/stale entry IDs are ignored during application. Entry-kind mismatches reject the operation.
- Parent moves are checked by the Preferences registry; invalid/cyclic layouts are rejected without changing the caller's active layout.
- The active profile ID is tracked and persisted.
- Versioned file format, temporary sibling write and rename; malformed input does not replace the in-memory store.

## Important boundaries
Profiles change menu presentation only. They do not enable/disable plugins, start/stop services, change system settings, or install/remove applications. Sony-owned entries remain outside the registry unless a verified firmware adapter registers them.

The module currently stores up to eight profiles in memory. Profile file I/O is portable C stdio; the Vita-specific data directory and UI integration are separate tasks. The file format is internal version 1 and should be migrated explicitly if it changes.

## Build
Run `make test` and `make sanitize`. Host tests cover create/update/apply, rename, protected default behavior, persistence, deletion and transactional rejection of malformed files.
