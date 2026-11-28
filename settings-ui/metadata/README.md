# Settings UI Metadata Catalogue

The Preferences registry stores stable identifiers and menu behavior. This metadata catalogue is a separate presentation layer: it maps those identifiers to user-facing text and visual hints without changing the persistence format or stable menu IDs.

## Descriptor fields

| Field | Purpose | Validation |
|---|---|---|
| `entry_id` | Existing Preferences registry ID | Required; stable lowercase key |
| `title` | User-facing row/page title | Required; 1–63 bytes |
| `description` | Short explanatory text | Optional; up to 191 bytes |
| `category_id` | Existing category used for grouping | Optional; must resolve to a category |
| `icon_key` | Renderer-owned semantic icon key | Optional; up to 31 bytes |
| `module_id` | Owning module identifier | Required; stable lowercase key |
| `flags` | Reboot/backend/read-only/experimental hints | Bit flags; never operational commands |

The registry copies every string into bounded storage. Callers may therefore build descriptors from temporary buffers and release them after registration.

## Safety and integration rules

- Metadata registration never creates a menu entry; the stable ID must already exist in the Preferences registry.
- Duplicate metadata IDs are rejected.
- Category references must resolve to actual category entries.
- Titles and descriptions are presentation only. They must never be used as persistence keys or command identifiers.
- `icon_key` is semantic (for example `network`, `storage`, `battery`); the renderer owns the actual artwork.
- Flags describe requirements and presentation hints. They do not start services, change plugin state or trigger a reboot.
- Unknown plugin inventory entries must not be assigned guessed controls through this API.
- Metadata validation is available before rendering or accepting a module catalogue.

## Current boundary

This is a portable, host-testable data model. It does not render Sony Settings pages, load module manifests, discover plugins or implement Vita-specific graphics. Those are separate integration layers.

## Appearance & Customization defaults

The shared metadata catalogue exposes `rtp_metadata_register_appearance()`, which registers the Sony-style Appearance & Customization category and the Boot Animation, Boot Splash, Boot Sound, Startup Warning, Interface Customization, Custom Assets Manager and Quick Menu entries. Quick Menu Widgets and Quick Menu Shortcuts are nested below Quick Menu. Descriptions are plain-language English; backend and reboot flags are declared per entry. This registers metadata/menu entries only—it does not claim that the corresponding Vita backends are installed.
