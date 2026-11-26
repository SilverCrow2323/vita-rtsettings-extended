# Plugin Inventory and Adapter Registry

## Scope

The host-testable inventory core parses supplied taiHEN configuration text. It does not open Vita paths, scan plugin binaries, write `config.txt`, enable/disable plugins or install packages. The Vita filesystem adapter and a transactional configuration editor are separate future modules.

## What is recorded

- Active plugin paths and commented-out plugin paths (the latter are marked disabled).
- taiHEN section, source line number and original path.
- Repeated references to the same path, with occurrence and duplicate count.
- Known catalogue IDs for recognized filenames; unrecognized files remain visible as unknown.
- Adapter capability only when an adapter descriptor is explicitly registered.

The parser accepts LF/CRLF, blank lines, comments and repeated sections. Lines which are too long, malformed section headers or invalid section names are counted rather than interpreted as plugins. Only `.skprx` and `.suprx` entries are considered plugin records.

## Safety model

An unknown plugin is inventory-only. A known plugin is also inventory-only until a matching adapter is registered. Registration does not prove that the plugin is installed, compatible or running; it only declares that this build contains an explicit control adapter. The caller must separately validate dependencies, conflicts, firmware and runtime status before exposing operations.

The parser intentionally does not infer a plugin version from a filename. Exact versions require trusted manifests or binary metadata and are a later filesystem-scanner task.

## API

See `include/plugin_inventory.h`. The core exposes bounded inventory storage, lookup by path/occurrence and a separate adapter registry. It is platform-neutral C99 and suitable for host regression tests and later Vita integration.

## Current limitations

- Filename recognition is a conservative alias table, not a binary identity check.
- No filesystem traversal, hash verification or plugin version detection.
- No dependency/conflict resolution yet.
- No config write, backup or rollback in this module.
- Commented entries are classified as disabled only when the comment begins with a plugin filename/path ending in `.skprx` or `.suprx`.

These limitations are deliberate: inventory must not mutate the console or pretend to understand unknown modules.
