# Custom Warning Adapter

## Upstream behavior

VitaTweaks Custom Warning replaces the Ensō startup epilepsy warning. Its upstream source searches:
For each mount in order (ux0, then ur0), it first checks the custom_warning directory for random warning files, then falls back to that mount's custom_warning.txt. Therefore the exact lookup order is ux0 directory, ux0 fixed file, ur0 directory, ur0 fixed file.

The file is Unicode UTF-16 Little Endian with BOM bytes FF FE. A BOM-only file displays no custom text. The adapter follows this precedence instead of assuming that a fixed text file is always the active source.

## Implemented portable adapter

- Plugin-active check before exposing or changing settings.
- Detects preset-rotation directories and fixed-file mode using the upstream mount precedence.
- Lists up to 16 safe preset filenames and reads individual presets.
- UTF-8 to UTF-16LE+BOM conversion and reverse conversion.
- Rejects malformed UTF-8, invalid surrogate pairs, embedded NUL/control characters, odd byte counts, missing BOM and oversized files.
- Atomic write through an injected backend, mandatory pre-write backup, read-back byte verification and rollback on failure.
- Supports adding a preset explicitly; refuses accidental overwrite when add mode is selected.
- Does not delete presets or alter taiHEN config/plugin load entries.

## Platform backend contract

The Vita backend must implement plugin detection, directory listing, bounded file reads, backup including an absent-file marker, staged atomic write/replace and restore. The portable core does not perform Vita filesystem I/O.

## Safety notes

- The plugin searches ux0 before ur0. An existing preset directory takes precedence over the fixed text file.
- When preset rotation is active, the fixed-message editor is disabled; the user must edit/add a file in the active preset directory.
- Restoring the original file is distinct from writing an empty BOM-only warning.
- The upstream plugin's exact binary release and 3.65 behavior still require target validation.
