# Plugin Adapter Framework

## First adapter: Wi-Fi Turbo

The first concrete adapter targets Wi-Fi Turbo's documented LAN remote-control interface. It builds only documented commands and delegates TCP I/O to an injected platform transport. The portable core does not open sockets or assume a Vita networking stack.

### Connection requirements

- The user must explicitly enable Wi-Fi Turbo's **Developer mode (remote control)** in its Settings page.
- Default endpoint: the Vita's LAN IP and TCP port **1339**.
- The endpoint is unauthenticated. Use it only on a trusted LAN; never expose or forward port 1339 to the internet.
- A disabled Developer mode is represented as unavailable; the adapter never enables it remotely.
- Each command is one exchange. A Vita TCP transport implementation is not part of this milestone.

### Implemented operations

| Operation | Command | Behavior |
|---|---|---|
| Read status | `status` | Returns raw status response |
| Keep Wi-Fi on | `keepwifi on/off` | Immediate |
| HUD overlay | `hud on/off` | Immediate |
| Auto-reconnect | `autoreconnect on/off` | Immediate |
| Smart Mesh Roaming | `roam on/off` | Immediate |
| Roam now | `roam now` | Immediate |
| Connect saved networks | `icm` | Immediate |
| Power profile | `profile maxperf/balanced/maxbatt` | Pending explicit confirmation |
| 40 MHz | `ht40 on/off` | Pending explicit confirmation |
| Confirm / revert | `confirm` / `revert` | Explicit user decision |

The adapter never auto-confirms profile or HT40 changes. Wi-Fi Turbo reverts unconfirmed changes after 45 seconds. The Settings UI must present the pending state and ask the user to confirm or revert.

### Safety and limitations

- No taiHEN config edits, installs, or Developer mode changes.
- No arbitrary command execution, speed tests, UDP sinks, logging commands or remote development controls.
- No SSID, access-point MAC or credentials are stored.
- Responses are passed through unchanged. Parsing the upstream status payload is a later protocol-versioned task.
- Platform transport must enforce bounded reads, a timeout, one command per TCP connection and response NUL termination.
- Host tests use a mock transport; this is not physical Vita/network validation.

## Custom Warning adapter

The second concrete adapter is Custom Warning from VitaTweaks. It follows the upstream lookup order (ux0 preset directory, ux0 fixed file, ur0 preset directory, ur0 fixed file), exposes fixed-message or random-preset mode, converts UTF-8 to UTF-16LE+BOM and writes through a backup/atomic-replace/read-back/rollback backend. The portable implementation is in `source/plugin-adapters/custom_warning_adapter.c`; tests use an in-memory filesystem. The Vita filesystem and plugin-active detector are still pending.
