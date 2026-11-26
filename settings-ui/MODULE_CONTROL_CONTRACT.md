# Module Control Contract

Plugin and service modules expose controls only through explicit descriptors and backend callbacks.

- Supported kinds: toggle, enumerated selector, stepped slider and bounded numeric input.
- Selectors must declare a finite list of unique values and user-facing labels.
- Numeric controls must declare minimum, maximum and positive step; writes outside the declared range or step are rejected.
- Unknown plugin settings are not inferred and cannot be changed through this API.
- Read-only controls cannot be written.
- Backend failures and invalid backend-reported values are surfaced to the caller.

The portable layer validates descriptors and values; the adapter owns the real plugin/service API, version checks, persistence and side effects. No generic taiHEN plugin configuration editing is attempted.
