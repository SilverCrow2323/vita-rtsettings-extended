# Plugin Help Panels

Every plugin-backed Settings row should expose a compact `?` affordance independently of its action control. Selecting it opens a read-only, scrollable detail panel; it must never toggle, start, stop, or edit the plugin.

## Information layout

1. **Plugin identity** — upstream display name, plugin ID, author/developer and detected version (when available).
2. **Overview** — detailed user-facing explanation of purpose and behavior.
3. **Technical details** — implementation model, files/configuration touched, data formats, protocol or API behavior, and relevant limits.
4. **Configuration** — supported options and where they are stored.
5. **Requirements and compatibility** — dependencies, firmware and plugin-version matrix, with unknown values explicitly marked unverified.
6. **Conflicts and cautions** — known overlaps, precedence rules, reboot requirements and recovery advice.
7. **Credits and license** — upstream author, project contributors, license statement and source/release links.

## Data contract

`plugin_help.h/.c` provides a bounded, renderer-neutral catalogue. Each descriptor is tied to a stable plugin ID and stores the identity, explanatory and technical text, compatibility notes, credits, license and upstream URLs. `rtp_plugin_help_open()` resolves a row's plugin ID into a panel model, including whether the source and release links should be shown.

The UI should show `?` only when a matching help entry exists. Unknown plugins remain visible in the Plugin Control Centre, but no guessed help content or adapter controls are attached to them. A missing help entry must not hide the plugin itself.

## Attribution and accuracy

- Upstream author and project are distinct from the Rt:Settings Extended adapter author.
- Do not infer a version from a repository's latest release; report the installed/detected version only when a version detector provides it.
- Do not claim firmware support from a host test.
- If the exact upstream license or firmware matrix is not confirmed, state that explicitly rather than inventing a value.
- Keep upstream links separate from local adapter documentation.

## Current built-in records

The initial catalogue includes Custom Warning (VitaTweaks) and Vita-Turbo-WiFi. Their records intentionally flag unpinned versions and unverified live-console integration. More plugin records should be added only after source, attribution and behavior have been checked.

## UI integration boundary

This milestone provides the data model and test contract, not the native Sony Settings renderer. The native row renderer must place the `?` button beside the setting title and open the panel without changing the selected setting. The panel should support vertical scrolling and long technical text.
