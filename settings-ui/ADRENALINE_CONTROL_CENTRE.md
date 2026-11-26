# Adrenaline Control Centre

## Integration target

The PS-button Quick Menu's **Settings** entry opens Adrenaline's own overlay menu. This module brings the editable settings from that overlay into Rt:Settings Extended, without replacing Adrenaline's runtime or VSH/Recovery code.

## Settings backed by adrenaline.bin

The upstream Adrenaline source defines `AdrenalineConfig` and writes it to `ux0:app/PSPEMUCFW/adrenaline.bin`. The current struct is 60 bytes and begins with two magic words. The new portable codec validates that exact signature and size before exposing values.

- Graphics Filtering: Original, Bilinear, Sharp bilinear, Advanced AA, LCD3x, Sharp bilinear without scanlines.
- Smooth Graphics.
- f.lux Filter Color: None, Yellow, Blue, Black.
- PSP screen scale X/Y and PS1 screen scale X/Y.
- Memory Stick Location: ux0, ur0, imc0, xmc0, uma0.
- USB device: Memory Card, Internal Storage, SD2Vita, PSVSD.
- Use DS3/DS4 controller.
- Skip Adrenaline Boot Logo.
- Screen size and screen mode values retained from the config structure.

The config adapter performs explicit little-endian field encoding, validates enum ranges and finite scale bounds, reads the existing 60-byte file, backs up the original bytes, requires an atomic replacement callback, then reads back and byte-verifies the result. It refuses unknown config sizes or magic values; it never rewrites an unfamiliar future format.

## VSH and Recovery

Separate pages will expose documented options from the PSP VSH menu and Recovery menu, including CPU clock, UMD ISO driver, restart VSH, plugin enablement, high-memory layout, BOOT.BIN execution, extended colors, Sony OSK, NoDRM engine, WMA/Flash registry flags and button assignment.

These options are not part of the 60-byte `adrenaline.bin` structure. Until a version-aware adapter for their real storage/API is available, the UI must mark them as **Open in Adrenaline** or **read-only**, not write guessed files or claim that a setting was changed.

## Safety and compatibility

- Target Adrenaline config version must match the two upstream magic values and exact structure size.
- Back up before changing any setting.
- Require atomic replacement and read-back verification from the Vita filesystem adapter.
- Changes that affect Memory Stick location may require Adrenaline to reinsert the memory stick; users must be warned.
- Restart/reboot requirements are shown per setting.
- If the config is absent, malformed, truncated or from a different format, show diagnostics and do not write.

## Host model milestone

`settings-ui/adrenaline_config.c/.h` implements config encode/decode, validation, load/save callbacks and read-back integrity checks. Host tests cover round-trip, wrong size/magic, invalid options, backup failure and corrupt readback.

No Vita filesystem adapter, native Settings renderer, physical console validation or direct VSH/Recovery writer is included yet.

## Upstream references

- [TheFloW / Adrenaline source](https://github.com/TheOfficialFloW/Adrenaline)
- [Adrenaline user menu source](https://github.com/TheOfficialFloW/Adrenaline/blob/master/user/menu.c)
- [Adrenaline config structure](https://github.com/TheOfficialFloW/Adrenaline/blob/master/user/main.h)
- [Adrenaline Recovery options](https://github.com/TheOfficialFloW/Adrenaline/blob/master/cef/recovery/options.h)
