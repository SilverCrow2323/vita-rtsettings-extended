# Sony Settings placement policy

## Product rule

Rt features do not all belong under one new top-level category. A feature should be nested into an existing Sony Settings category when that is the natural owner and the target page can be extended without replacing Sony content. The dedicated **Rt:Settings Extended** category is reserved for cross-system tools and pages that have no appropriate Sony owner.

This document is a placement contract, not a claim that the integrations listed below are already implemented. A row is exposed only after its page XML, callback bridge, lifecycle and failure handling are implemented and validated on the target firmware.

## Placement matrix

| Feature | Intended Sony location | Fallback / ownership | Evidence and gate |
|---|---|---|---|
| Bluetooth & Controllers | Settings → Devices, adjacent to the existing Bluetooth/device controls | Rt category only if the Sony Devices page cannot be extended safely | Must preserve stock device rows and use verified controller APIs; current inventory model has no live discovery/pairing backend |
| Storage & Devices | Settings → Devices → Storage Devices | Keep YAMT as the owner of its storage-specific controls; Rt adds only non-conflicting diagnostics | YAMT documents this native nested page; Rt storage enumeration and YAMT mapping remain pending |
| Wi-Fi Turbo adapter | Settings → Network → Wi-Fi Turbo | No duplicate page in Rt | Upstream Wi-Fi Turbo demonstrates this placement; Rt adapter transport is still pending |
| Tai Saver | Rt:Settings Extended → Tai Saver | Dedicated Rt page; no Sony category naturally owns backup/recovery of taiHEN configuration | Must wait for Vita filesystem backend, Settings callbacks and safe recovery implementation |
| Plugin Control Centre | Rt:Settings Extended → Plugin Control Centre | Do not inject controls into HENkaku Settings or arbitrary third-party pages | Generic inventory and compatibility models are not live plugin controls |
| Plugin Compatibility | Rt:Settings Extended → Plugin Compatibility | Shared read-only diagnostic page | Expose only findings from verified plugin metadata and rules |
| Appearance & Customization | Existing Sony Display & Sound / System subpages only for settings that genuinely map there; otherwise Rt | Rt page for cross-cutting themes, assets and layout profiles | Do not duplicate Sony display controls or claim unsupported PAF callbacks |
| USB / FTP / streaming | Network or Devices only when a specific service has a stable matching Sony page; otherwise Rt | Rt service page | No live Vita service backends currently |
| System Diagnostics | Rt:Settings Extended → System Diagnostics | Dedicated Rt page | Must be read-only until each diagnostic source is verified |
| Preferences & Profiles | Rt:Settings Extended → Preferences & Profiles | Dedicated Rt page | Current preferences and profile models are portable, not yet persisted by a Vita backend |

## Integration rules

1. **Preserve Sony ownership.** Extend the original page layout at a verified insertion point. Never replace a complete Sony XML page with a reduced copy.
2. **Do not duplicate controls.** If a feature already has a native page (for example Wi-Fi Turbo or YAMT storage controls), Rt must not create a competing copy.
3. **Do not hijack third-party pages.** HENkaku, YAMT and other plugins own their pages. Rt can expose its own controls and link to existing pages only when a stable mechanism exists.
4. **Fail closed.** If a target XML, module instance, callback slot or resource lifetime cannot be validated, leave that Sony page untouched and do not register the Rt row.
5. **Keep actions truthful.** A row must not appear as operational until its callback and backend are present. Read-only and unavailable states must be explicit.
6. **Avoid speculative placement.** The Settings hierarchy and XML IDs must be inspected on the exact target firmware before implementation; names in this matrix are semantic destinations, not guessed XML identifiers.
7. **Installer remains maintenance-only.** The VPK installs/removes Rt-owned components and requests explicit approval for taiHEN configuration changes. Daily usage remains in Sony Settings.

## First delivery sequence

- First prove one safe native insertion into the Sony Settings root for the dedicated Rt category.
- Then prove a single read-only page and repeated Settings core load/unload.
- Add the Tai Saver page only after its Vita filesystem backend and callbacks are complete.
- Add existing-category integrations independently, one Sony page at a time, with stock-page preservation tests.
- Package a VPK only when it contains a real SUPRX/resource payload and an installer that can validate, preview, install, roll back and uninstall it. A package containing only the portable static library is not a Settings extension and must not be presented as one.
