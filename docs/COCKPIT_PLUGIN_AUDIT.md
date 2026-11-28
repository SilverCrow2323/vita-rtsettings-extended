# Cockpit — live plugin inventory and conflict audit

The Cockpit **Plugin Control Centre** now reads the current `ur0:tai/config.txt` from the Vita and builds a read-only inventory using the shared plugin inventory and compatibility models.

## Displayed information

- Plugin path and taiHEN section.
- Config line number and duplicate occurrence.
- Row state: Unknown, Disabled, Detected, Blocked or Review.
- Aggregate counts and known blocking/review compatibility findings.

The scan is bounded to 64 KiB and rejects unreadable, oversized or malformed input rather than presenting a partial inventory as complete. The view is scrollable and rescans when opened.

## Safety boundary

This Cockpit page does not install, remove, enable or disable plugins. It does not claim that a taiHEN entry proves the plugin binary is present or running. Unknown plugins remain visible and are not assumed compatible. Only explicit rules in the current compatibility engine produce Blocked or Review findings; an empty report is not a universal compatibility guarantee.

The Cockpit build intentionally registers no operational plugin adapters in this view. A plugin is therefore not presented as configurable merely because a portable adapter contract exists elsewhere in the repository.

## Validation

The plugin parser and compatibility evaluator already have host regression tests. The Cockpit VPK workflow now rebuilds when these shared sources change. A successful cross-build validates compilation and packaging, not physical-console behavior; the new page still requires a real PS Vita test.
