# Preferences UI Model

## Purpose
This milestone provides the navigation and presentation model consumed by a future native Settings renderer. It deliberately contains no Vita SDK calls, Sony page hooks, graphics code or assumptions about proprietary Settings widgets.

## Navigation model
- Root view and nested category pages with a bounded 16-frame stack.
- Enter validates that the target exists and is a category.
- Back returns to the previous page and refuses to pop the root.
- Each frame retains a selected stable ID and row index for renderer focus restoration.

## Row model
- Rows are built from the live Preferences registry.
- Only direct children of the current category are shown.
- Hidden entries are omitted; a hidden ancestor suppresses its descendants.
- Rows are sorted by configured order; ties retain deterministic registration order.
- Rows expose stable ID, kind, order and registry index. Localized/display labels and icons belong to the native renderer or module metadata, not this model.

## Editing actions
- Move an entry up/down among visible siblings.
- Change visibility for a category, setting, tool or action.
- Reset an entry to its registered defaults.
- Apply a named Layout Profile through the existing profile engine.
- Category parent changes remain owned by the Preferences registry and profile validation.

## Safety and boundaries
This model never launches applications, edits Sony-owned controls, changes plugin configuration, starts services or performs filesystem operations. It does not claim to be the native Vita Settings UI. A renderer must bind these model operations to the platform's verified page lifecycle and input callbacks.

## Build and tests
The host suite covers root/category navigation, ordering, hidden rows, visibility changes, reset, profile application and root-stack protection.
