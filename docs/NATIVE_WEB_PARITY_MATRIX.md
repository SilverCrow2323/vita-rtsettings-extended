# Native Settings / Web Simulator parity

The web simulator is a design preview. It must not be used to imply that a native Settings control is already wired to a Vita plugin.

| Area | Native SUPRX status at this milestone | Web simulator status |
|---|---|---|
| Rt root category | Implemented | Interactive |
| Preferences & Settings Management | Root path / first status leaf only | Interactive preview |
| Rt Manager status | Read-only native status | Informational preview |
| Plugin inventory | Cockpit read-only audit; not a native control page | Example inventory |
| Network, storage, controllers, display | Not exposed as native control pages yet | Simulated navigation |
| Conflict prevention | Shared compatibility model exists; no web-side activation | Safety explanations only |
| Tai Saver | Verified snapshot creation in Cockpit; full browser/restore UI absent | Informational preview |
| Hardware operations | On-device code only, with guards | Never available in browser |

## Promotion rule

A simulator item may be promoted to implemented-native only after:
1. a native Settings page and callback exist;
2. the corresponding adapter is registered and its metadata is verified;
3. dependencies, firmware and hardware applicability are checked;
4. conflicts are blocked or explicitly classified;
5. native build and host regression tests pass;
6. physical-device testing is recorded separately from CI.

Do not expose a simulated toggle as an operational control. Keep the web demo disconnected from private device state.
