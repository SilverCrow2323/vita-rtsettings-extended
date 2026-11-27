# SKGleba ineedsettings — reference audit

## Source inspected

Repository: [SKGleba/ineedsettings](https://github.com/SKGleba/ineedsettings)

The public README makes two concrete statements:

1. Install the supplied SUPRX under both `NPXS10015` and `NPXS10016`, **instead of** `henkaku.suprx`.
2. Build it by replacing `system_settings.xml` from the HENkaku project.

The README also points to a Mega collection of alternative builds. The repository material inspected for this audit does not expose the implementation source, hook setup, callback table, XML diff or build recipe. Therefore this is useful architectural evidence, but not enough to copy implementation details or certify firmware compatibility.

## What this tells Rt:Settings Extended

- The Settings integration path is tied to the Settings application title(s), not to a standalone LiveArea application.
- The plugin and the Settings XML are a coordinated pair. A SUPRX alone does not describe the complete integration.
- The instruction to replace HENkaku's SUPRX is a deployment choice made by that project. Rt must **not** adopt it as an installer default: users may rely on HENkaku Settings, and replacing another plugin would violate our non-destructive installation policy.
- Supporting `NPXS10016` is a separate compatibility decision. Rt's initial target is the primary Settings application `NPXS10015`; adding another title requires evidence that its module layout, lifecycle and XML resources are compatible.
- The reference reinforces the need to preserve the stock Settings XML and to manage the plugin/XML lifecycle together.

## Rt implementation decision

Rt will use its own user-mode SUPRX registered in the taiHEN `*NPXS10015` section after explicit installer approval. It will not replace `henkaku.suprx`, rewrite HENkaku resources, or install into `NPXS10016` by assumption.

The first native proof remains deliberately narrow:

1. detect the Settings core module lifecycle;
2. validate the exact root XML request and its bounded buffer;
3. insert one Rt-owned root entry without dropping Sony or third-party entries;
4. open a read-only Rt page;
5. close and reopen Settings repeatedly and verify hook/resource cleanup.

Only after that proof should the Tai Saver page expose operational callbacks.

## Required source-level follow-up

Before implementation relies on any ineedsettings-specific behavior, obtain and inspect the actual source or a reproducible source archive, including:

- SUPRX entry point and module metadata;
- exact module-load/unload hooks and NIDs;
- PAF XML loader hook and resource ownership;
- root XML changes and callback registrations;
- target title IDs and firmware assumptions;
- cleanup/unhook behavior.

Until those items are independently verified, no NID, offset, callback slot or XML schema detail is attributed to ineedsettings.

## Relationship to other references

- [YAMT user plugin](https://github.com/SKGleba/yamt-vita/tree/master/plugin/user) remains the source-level reference for a user-mode plugin loaded by Settings and a nested Devices page.
- [h-encore Settings plugin](https://github.com/TheOfficialFloW/h-encore/tree/master/plugin) remains the source-level reference for a root Settings XML and custom callback interception.
- [Rt native adapter design](NATIVE_SETTINGS_ADAPTER.md) defines the implementation and installer safety requirements.

## Status

Research note only. No ineedsettings code, binary, XML or NID has been copied into Rt. No native Settings integration or VPK is claimed by this document.
