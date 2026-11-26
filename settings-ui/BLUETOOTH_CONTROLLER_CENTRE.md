# Bluetooth & Controller Control Centre

## Product direction

A dedicated **Connectivity → Bluetooth & Controllers** page will unify controller inventory, driver health, supported capabilities, pairing guidance, compatibility checks and links to remapping profiles. It is an orchestration and diagnostics layer, not a replacement Bluetooth stack.

## Curated driver strategy

| Driver | Role | Project policy |
|---|---|---|
| VitaControl | Broad Bluetooth controller provider | Preferred provider candidate: DS3, DS4, DualSense, DualSense Edge, Xbox One and Switch Pro are listed upstream |
| MiniVitaTV | Makes Vita behave like PS TV for official DS3/DS4; up to four local controllers | Alternative PS-TV mode; may be combined with 8BitVita only |
| ds34vita | DS3/DS4/DualSense provider | Legacy/alternative provider; never load alongside VitaControl or MiniVitaTV |
| ds3vita / ds4vita | Older individual providers | Inventory and migration guidance only; avoid as default when a maintained combined provider is available |
| X1Vita | Xbox One 1708-specific provider | Legacy alternative; not compatible with other DS-family drivers; newer Xbox Series / BLE controllers are not supported by the Vita Bluetooth hardware |
| 8BitVita | 8BitDo-specific provider | Optional specialized provider; upstream documents compatibility with MiniVitaTV and up to two simultaneous controllers |
| reVita | Remapping and profiles | Optional companion layer, not a Bluetooth transport driver |
| ds4Touch / DS34Motion | Optional DS4 touch / motion support | Separate enhancement adapters; only expose features when installed and compatible |

The preferred default is **VitaControl + optional reVita**, subject to target-device validation. MiniVitaTV is a distinct PS-TV compatibility/local-multiplayer mode and conflicts with VitaControl. Its documented 8BitVita pairing is the only multi-provider exception currently accepted by the project. Driver selection must require an explicit plan and reboot where required.

## Intended user experience

- **Controller inventory:** model, connection/paired state, selected provider, driver health, supported input capabilities and battery percentage only when a backend actually reports it.
- **Pairing assistant:** opens the native Sony Bluetooth device settings and displays model-specific pairing instructions. Pair/unpair itself remains owned by the system Settings page unless a documented safe API is found.
- **Driver manager:** detect installed/active providers, detect duplicates/conflicts, show version/source/firmware notes, and guide a safe switch with backup + reboot + recovery instructions.
- **Input test:** live buttons, analog axes, triggers, touchpad and motion only where a tested input backend exposes them.
- **Profiles:** global and per-game profiles via reVita adapter; map buttons, sticks, touch zones, gyro, turbo and combos only where supported.
- **Advanced diagnostics:** controller count, provider, plugin load order, taiHEN entries, duplicate providers, known app compatibility and last errors.
- **Privacy:** never display or log full Bluetooth MAC addresses by default. Battery and controller details remain unknown when the backend cannot read them.

## Safety and compatibility rules

1. The native Sony Bluetooth pairing flow remains the pairing authority.
2. Never install, enable, disable or reorder a kernel plugin based only on a filename guess. Require recognized plugin IDs and a reviewed adapter.
3. Controller providers are mutually exclusive by default. The explicitly documented exception is the MiniVitaTV + 8BitVita combination. VitaControl, MiniVitaTV, ds34vita, ds3vita, ds4vita and X1Vita must not be stacked together. reVita is a remapping companion, not a Bluetooth transport driver.
4. MiniVitaTV has a distinct PS-TV behavior profile and can affect native Vita controls and application compatibility. It must be an explicit mode, not the default.
5. Do not promise battery, rumble, touchpad, gyro or trigger support for every controller. Display only capabilities reported by a verified backend and tested per device/provider.
6. No controller pairing, unpairing, Bluetooth radio reset or plugin mutation is implemented by the current host model.
7. Unknown controllers/providers remain visible as unknown and read-only.

## Host model milestone

`settings-ui/bluetooth_controllers.h/.c` implements a bounded renderer-neutral read model:
- controller identity, model, state, provider, capabilities and optional battery;
- aggregate connected/paired/unavailable/error counts;
- validation for IDs, enum ranges and battery bounds;
- exclusive driver-provider conflict validation.

It accepts observations supplied by a future backend. It does not scan Bluetooth devices, pair controllers or read Vita plugin state. Host tests exercise valid DS3/DS4/Xbox observations, unknown battery values, invalid input, duplicate IDs and mutually exclusive providers.

## Implementation roadmap

1. **Completed in this milestone:** portable controller inventory model and exclusive-driver stack validator.
2. Vita inventory adapter: taiHEN config + installed plugin/version detection, without writes.
3. VitaControl adapter: live controller list and capabilities only if upstream/public API permits; otherwise mark backend unavailable and link to native Settings.
4. reVita adapter: profile inventory and safe configuration bridge, respecting reVita's own schema.
5. Pairing assistant and controller input test, gated by tested APIs.
6. Safe driver migration wizard with backup, conflict preflight, staged config update, rollback and explicit reboot.
7. Physical validation on Vita 1000 OLED / 3.65 Ensō and per-controller compatibility matrix.


## Upstream references

- [VitaControl](https://github.com/Hydr8gon/vitacontrol) — broad Bluetooth controller driver and supported-device list.
- [MiniVitaTV](https://github.com/TheOfficialFloW/MiniVitaTV) — PS-TV compatibility mode and up-to-four DS3/DS4 local controllers; upstream README identifies it as beta.
- [ds34vita](https://github.com/MERLev/ds34vita) — combined DS3/DS4 controller driver.
- [reVita](https://github.com/MERLev/reVita) — input remapping, touch/gyro handling, profiles and external-controller integration.
- [reVita external controller compatibility notes](https://github.com/MERLev/reVita/wiki/External-Controllers) — practical provider comparison and limitations.
- [X1Vita](https://github.com/Ibrahim778/X1Vita) — Xbox One 1708 compatibility and limitations.
- [8BitVita](https://github.com/shadowknight1620/8BitVita) — 8BitDo model list and MiniVitaTV compatibility.

Upstream capabilities are research inputs, not guarantees for every Vita firmware, controller revision or installed plugin combination. Project support status will be recorded separately from upstream claims.


## Device classification and unknown-controller learning

The model now distinguishes device category from controller make/model: controller, audio, computer, phone, tablet, keyboard, mouse, printer, remote, other and unknown. A backend may provide the Bluetooth Class of Device and service-derived capabilities; the UI may let the user override an uncertain classification without changing the reported raw evidence.

Unknown controllers can use Input Learning. The portable recorder stores observed event kind, raw code, first and last values, label and observation count for buttons, axes, triggers, touch, gyro, accelerometer, hat and unknown events. It does not create guessed controls. A real Vita input adapter must expose the events before the recorder can capture them.


## Discovery and manual classification

The backend may classify devices from the Bluetooth Class of Device (CoD). The host model includes a conservative CoD classifier for computer, phone, audio, controller/joystick, remote, keyboard, mouse and printer. The original CoD is retained in each device record. Service Discovery Protocol (SDP) results and service-derived capabilities can refine this classification when the Vita backend exposes them.

Users can override the category per device. The snapshot records whether a category came from CoD inference or a manual override, while preserving the original CoD and raw capabilities for diagnostics. A category override changes only the UI classification; it does not install a driver or change Bluetooth services.

The device inventory capacity is bounded independently of the Vita's active connection limit, so a discovery scan can list more remembered/discovered entries than can be connected simultaneously.
