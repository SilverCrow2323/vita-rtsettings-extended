# Settings Screen and Renderer Adapter

This milestone connects the Preferences navigation model and user-facing metadata catalogue to a renderer-neutral page contract.

## Page construction
- Resolves the current category title and description from registered metadata.
- Builds visible rows from the live Preferences registry, preserving ordering and visibility rules.
- Resolves row titles, descriptions, semantic icon keys and presentation flags from metadata.
- Uses a readable fallback label derived from the stable ID when optional metadata is absent.
- Preserves selected-row state by stable ID.

## Renderer contract
A platform renderer implements four callbacks: begin page, draw row, draw footer and end page. The portable core owns page data and ordering; the renderer owns fonts, spacing, icons, focus visuals, controller input and platform lifecycle.

The callback contract deliberately contains no Vita SDK or proprietary Settings symbols. A Vita implementation must be based on verified firmware-specific APIs and must not claim native Sony integration until that backend exists.

## Safety
Metadata flags are presentation hints only. This layer does not start services, change plugin state, launch applications, edit Sony settings or perform filesystem operations. Rendering fails closed when the metadata catalogue is invalid or a required renderer callback is missing.

## Host verification
The dedicated screen test builds a root page and a nested Connectivity page, checks metadata resolution, and exercises renderer callbacks through a fake backend.
