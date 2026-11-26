# Service Manager Contract

A portable service catalogue for Connectivity and other system-service pages.

- Service adapters declare only supported operations (start, stop, auto-start).
- Refresh reads actual backend state; query failures are represented as Error instead of assumed Stopped.
- Services sharing a non-empty conflict group cannot be started together. The manager refreshes all services immediately before a start request to avoid relying on stale UI state.
- Unsupported operations are rejected; read-only services can be inventoried without control.
- Auto-start is a persistent preference delegated to the service backend, not a promise that hardware or a cable is present at boot.

The core does not implement FTP, USB or streaming itself. Platform/plugin adapters must provide the real lifecycle and conflict-group declarations.
