# Layout Profile Manager Controller

A platform-neutral controller for the Preferences profile-management screens.

- Lists saved profiles and exposes a Create Profile action.
- Opens a profile action page with Apply, Update, Rename and Delete.
- Applies profiles through the existing merge-safe Layout Profiles core and resets navigation to the root.
- Requires a separate confirmation step before deletion; Back/cancel never deletes.
- Keeps stable profile IDs separate from editable display names.
- Exposes immutable view snapshots for the eventual Vita renderer.

The controller does not draw graphics, persist files by itself, or perform service/plugin actions. Call the existing profile save/load APIs at the application persistence boundary.
