# Rt:Vita Cockpit — Component Model

## Component states

- **Installed:** a valid installed-registry record and owned payload are present.
- **Disabled:** installed but intentionally not activated.
- **Update available:** trusted release index advertises a newer compatible version.
- **Missing:** selected by the user but not installed.
- **Blocked:** firmware, dependency, conflict, signature or storage gate failed.
- **Review required:** compatibility or provenance is incomplete.
- **Orphan candidate:** owned files exist without a matching registry record; never auto-delete.

## Selection resolution

1. Start with the user's explicit selections.
2. Add required dependencies recursively.
3. Reject dependency cycles and missing dependency manifests.
4. Check conflicts against both selected and already-installed active components.
5. Validate firmware and framework ranges.
6. Calculate required storage, backup space and reboot impact.
7. Present the exact resolved plan.
8. Require explicit confirmation before writing.

Select All means all compatible modules included in the currently available package. It does not imply downloading or installing every plugin in an online catalogue.

## Help overlay contract

A help record is keyed by stable component ID and contains:
- title and purpose;
- concise summary and detailed explanation;
- firmware and hardware notes;
- dependencies and conflicts;
- files and configuration affected;
- required storage and reboot impact;
- known limitations and recovery procedure;
- upstream source, author and license.

If no help record exists, do not draw a dead '?' control. Unknown details are explicitly labelled unknown.

## Registry record

Each component record must contain:
- schema version;
- component ID and installed version;
- source package/release identity;
- manifest digest and verification status;
- enabled/disabled state;
- exact owned files and their digests;
- data paths that are preserved on update/removal;
- install/update timestamp;
- transaction ID and last successful verification.

The registry is committed only after payload verification and successful transaction completion. A failed or partial transaction must not create an Installed state.

## Trust and rollback

- Manifest and payload digests are checked before commit.
- Signed release metadata is required for online updates.
- Backups are retained until post-commit verification completes.
- Rollback restores only paths owned by the transaction.
- Unknown files and unrelated taiHEN entries are never removed.
- A rollback failure leaves the journal and recovery data intact and reports a degraded state.
