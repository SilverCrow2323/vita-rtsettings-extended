# Bluetooth & Controller Control Centre

## Product direction

Bluetooth is a first-class page inside the native Settings experience. Its advanced features stay nested inside Bluetooth; there is no separate top-level controller application.

## Proposed menu hierarchy

- **Bluetooth**
  - Paired / discovered devices
  - Add device (native Sony pairing assistant; pairing authority remains Sony Settings)
  - Connected device details
  - Controllers
    - Controller inventory and provider status
    - Input test
    - Profiles and remapping (reVita adapter)
    - **Advanced Settings**
      - Device category override
      - Unknown-controller input learning
      - Raw event / capability inspection
      - Provider and driver diagnostics
      - Compatibility and conflict report
  - Audio devices
  - Other devices
  - Bluetooth diagnostics

The menu exposes only actions supported by the active backend. Unsupported actions are visibly unavailable with an explanation, not simulated.

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
- **Advanced settings:** manual device classification, input learning for unknown controllers, raw event inspection and detailed provider diagnostics.
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
- exclusive driver-provider conflict validation;
- category classification and user override with provenance;
- input learning recorder for buttons, axes, triggers, touch, gyro, accelerometer, hat and unknown event kinds.

It accepts observations supplied by a future backend. It does not scan Bluetooth devices, pair controllers or read Vita plugin state. Host tests exercise valid DS3/DS4/Xbox observations, unknown battery values, invalid input, duplicate IDs and mutually exclusive providers.

## Implementation roadmap

1. **Completed:** portable controller inventory, exclusive-driver stack validator, conservative device classification, manual category override and input-learning data model.
2. Vita inventory adapter: taiHEN config + installed plugin/version detection, without writes.
3. VitaControl adapter: live controller list and capabilities only if upstream/public API permits; otherwise mark backend unavailable and link to native Settings.
4. reVita adapter: profile inventory and safe configuration bridge, respecting reVita's own schema.
5. Vita Bluetooth input adapter for live input capture; unknown devices remain read-only until this exists.
6. Safe driver migration wizard with backup, conflict preflight, staged config update, rollback and explicit reboot.
7. Physical validation on Vita 1000 OLED / 3.65 Ensō and per-controller compatibility matrix.

## Upstream references

- [VitaControl](https://github.com/Hydr8gon/vitacontrol)
- [MiniVitaTV](https://github.com/TheOfficialFloW/MiniVitaTV)
- [ds34vita](https://github.com/MERLev/ds34vita)
- [reVita](https://github.com/MERLev/reVita)
- [reVita external controller compatibility notes](https://github.com/MERLev/reVita/wiki/External-Controllers)
- [X1Vita](https://github.com/Ibrahim778/X1Vita)
- [8BitVita](https://github.com/shadowknight1620/8BitVita)

Upstream capabilities are research inputs, not guarantees for every Vita firmware, controller revision or installed plugin combination.
