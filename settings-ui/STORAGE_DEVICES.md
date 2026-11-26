# Storage & Devices snapshot model

The first Storage & Devices milestone adds a bounded, renderer-neutral snapshot model for platform-reported storage observations.

## Device information

Each observation can report a stable device ID, user-facing label, device kind (SD2Vita, Sony Memory Card, internal, USB or unknown), mount state, mount point, total/free bytes and read-only status. The snapshot computes free-space percentage and aggregate mounted/unavailable/error counts.

Duplicate stable IDs are rejected. Two mounted devices claiming the same mount point are both flagged and surfaced in the snapshot; the model does not guess which mapping is correct.

## Safety boundary

This module does not probe the Vita, mount or unmount devices, change YAMT configuration, format storage or write files. The future Vita adapter must provide observations from verified platform APIs and distinguish SD2Vita from the Sony Memory Card using reliable device metadata—not merely assume a device type from ux0:/uma0:.

Missing devices should be represented by an empty/unavailable observation only when the adapter has reliable evidence. A failed query should be reported as ERROR, not silently converted to an empty device list.

## API and validation

See storage_devices.h. The builder validates bounded strings, device kinds/states, mount-point syntax, free space not exceeding total capacity and unique IDs. It is portable C99 and host-testable.

## Pending integration

Vita mount/device enumeration, YAMT mapping detection, live refresh, storage health checks, UI rendering and physical validation remain pending.
