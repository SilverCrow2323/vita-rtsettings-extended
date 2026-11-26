# Preferences Menu Registry — Milestone 1

## Scope
This milestone implements the platform-independent registry and persistence core for Preferences. It is not yet a graphical Settings page and does not hook Sony's native menu.

## Capabilities
- Register stable-ID categories, settings, tools and actions.
- Store parent category, visibility and ordering.
- Change visibility without changing the underlying feature/service state.
- Reorder entries through an explicit order value.
- Move entries between categories while rejecting missing parents and cycles.
- Reset one entry or the full registry to registered defaults.
- Save and load a versioned preferences file.
- Load transactionally: malformed input leaves the caller's existing registry untouched.
- Save through a sibling temporary file and rename so a failed write does not replace the previous preferences file.

## Data ownership and safety
The registry stores machine IDs, not translated labels. The UI/module registry supplies display text separately. Hiding an entry affects presentation only: it does not disable a plugin, stop FTP, remove an application or uninstall a module.

Only categories can contain entries. Parent IDs must refer to registered category IDs. Moving a category is validated against cycles. Sony-owned menu entries are not registered or modified by this core unless a future firmware-specific integration adapter explicitly exposes them.

## File format
The current format is line-oriented:
```text
RTSPREFS<TAB>1
advanced-options<TAB><TAB>1<TAB>1<TAB>20
system-tools<TAB><TAB>1<TAB>1<TAB>30
vita-shell<TAB>system-tools<TAB>3<TAB>1<TAB>10
```
Fields are stable ID, parent ID, entry kind, visible flag and order. The format is versioned; incompatible or malformed data is rejected instead of partially applied.

Default values are supplied by the registered module catalogue at runtime. Persisted visibility/order values override defaults for known IDs. A later UI integration milestone will merge persisted choices with the installed module catalogue, ignoring removed IDs and assigning defaults to new entries.

## Build and tests
Run `make test` and `make sanitize`. Host tests cover visibility/reset, ordering, parent movement, cycle rejection, duplicate/invalid IDs, save/load and transactional rejection of malformed files.

## Not included yet
- Native Preferences page in Sony Settings.
- Drag-and-drop or button-based ordering UI.
- Named layout profiles and profile switching.
- Sony-owned menu rearrangement.
- Import/export UI.
- Vita-specific persistence path and power-loss durability integration.
