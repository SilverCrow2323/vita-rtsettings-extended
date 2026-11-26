# Plugin Control Centre model

The Plugin Control Centre now has a host-testable, renderer-neutral read model combining three safety layers: taiHEN inventory parsing, explicit plugin-to-adapter registration, and dependency/conflict evaluation over known enabled plugin IDs.

## Row states

- **Unknown** — path is visible, but no conservative filename mapping exists.
- **Disabled** — a recognized plugin entry is commented out.
- **Detected** — recognized and enabled, but no dedicated adapter is registered.
- **Configurable** — recognized, enabled and backed by an explicitly registered adapter.
- **Blocked** — an enabled plugin participates in a blocking dependency/conflict finding.
- **Review** — an enabled plugin participates in a review-level compatibility finding.

The snapshot preserves source path, taiHEN section, source line, occurrence and duplicate count. It also exposes aggregate counts and the complete compatibility report for diagnostics.

## Safety boundaries

- This is a read-only projection of caller-supplied config text; it does not scan Vita storage or write config.txt.
- Compatibility is evaluated only for recognized enabled plugin IDs. Unknown plugins remain visible and are never assumed compatible.
- A registered adapter means only that a dedicated adapter exists in this build. It does not prove that the plugin is installed correctly, running, firmware-compatible or physically tested.
- A blocked/review state takes precedence over configurability; the UI must not offer write operations while the compatibility report blocks the plugin.
- The model does not execute adapter operations. The existing safe module-control contract remains responsible for validating declared controls and backend callbacks.

## API

See plugin_control_centre.h. rtp_plugin_centre_build() creates a bounded snapshot from an inventory and adapter registry. It is deterministic, platform-neutral C99 and suitable for host regression tests.

## Not implemented here

Vita filesystem scanning, binary/version identification, live taiHEN state, transactional config editing, native Sony Settings rendering and hardware validation remain separate tasks.
