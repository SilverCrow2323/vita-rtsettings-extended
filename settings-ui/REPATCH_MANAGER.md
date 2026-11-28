# RePatch & ReAddcont Manager

Integrated native Settings module based on the feature set of Eyeman's rePatch Manager, not a binary or code transplant.

## Features
- Separate **Game Patches** (`ux0:/rePatch/`) and **DLC / Add-on Patches** (`ux0:/reAddcont/`) views.
- Detect active folders and disabled folders using the established `DIS` suffix convention.
- Show title ID and optional friendly title.
- Enable/disable individual entries.
- Reject path traversal, malformed IDs, duplicate entries and destination collisions.
- Verify source/destination state after every rename; report backend errors instead of showing false success.
- Future extensions: multiple mount roots, title database refresh, bulk operations with preview, per-title backup/restore and rePatch plugin health.

## Safety
The host module does not touch the filesystem. It consumes a scan snapshot and a backend. The backend's `rename_no_replace` callback contract requires a truly atomic no-clobber rename; a check-then-rename implementation that can overwrite a destination is not acceptable. The UI must ask for confirmation before changing state and refresh the scan after the operation.

## Current implementation
`settings-ui/repatch_manager.c/.h` implements bounded snapshot parsing and state transitions for game and DLC folders. Host tests use a fake filesystem and cover enable/disable, duplicate destination protection, backend failure and traversal rejection.

The current adapter is not yet connected to Vita filesystem APIs. Title-name lookup, directory enumeration, a UI renderer and physical testing remain pending.

## Upstream attribution
The original RePatch Manager by Eyeman is GPL-3.0. This module is a clean-room compatible feature implementation based on its documented user workflow; it does not copy its Lua source, assets or game database. Upstream: https://github.com/Eyeman420/rePatch-Manager
