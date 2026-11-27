# Rt:Vita Cockpit

## Product role

Rt:Vita Cockpit is the installation, configuration, update and maintenance front end for the Rt ecosystem on the PS Vita. It is the user's entry point for preparing the console and selecting which Rt components to deploy.

Cockpit is not a replacement for the daily Sony Settings experience. After installation, day-to-day controls remain in native Settings pages and existing Sony categories.

## Product split

- **Rt:Vita Cockpit:** standalone LiveArea utility for installation, component discovery, update checks, selection, repair, rollback and maintenance.
- **Rt:Components:** independently versioned modules with stable IDs, manifests, dependencies, conflicts, firmware ranges, payload hashes, owned paths and data-preservation rules.
- **Rt:Settings Extended:** native Settings integration and shared services.
- **Rt Manager:** always located at Settings → Rt:Settings Extended → Preferences & Settings Management → Rt Manager.
- **Native Settings adapter:** user-mode SUPRX in the Settings title. It is not implemented by Lua and must not replace Sony's root XML or HENkaku.

## Runtime and language decision

The Cockpit shell and privileged filesystem/configuration operations are implemented in C with VitaSDK. This avoids making a Lua runtime a prerequisite for installation or recovery.

Lua may be embedded later for non-critical presentation scripting or optional custom screens, provided the interpreter is bundled, sandboxed and versioned with Cockpit. It must not own taiHEN config transactions, SUPRX lifecycle, component trust verification or recovery.

Native Sony Settings integration remains C/SUPRX. Cockpit's boot animation is an application startup presentation and must not be confused with Ensō boot animation. Actual boot animation management is a separately selectable component adapter with format validation, backup, engine-conflict detection and reboot-aware rollback.

## Cockpit sections

1. **System Overview** — firmware, storage, free space, Rt framework state and diagnostics.
2. **Components** — installed, missing, disabled, update available, incompatible and unknown states.
3. **Install Manager** — install selected, select all compatible, deselect all, add later and dependency resolution.
4. **Update Center** — Cockpit, framework and per-component version/provenance checks.
5. **Settings Integration** — native Settings extension state and optional Sony-category adapters.
6. **Recovery & Maintenance** — repair, verified backups, rollback, disable and uninstall.
7. **Boot & Appearance** — Cockpit startup animation plus separate Ensō boot splash/animation/sound adapters.

## Selection and help behavior

Every component has a stable ID, display name, summary, detailed help content, firmware support, dependency/conflict list and current state. The user can enable all compatible components or select only individual entries.

Required dependencies are auto-selected and cannot be unchecked while a dependent module remains selected. Conflicting or unverified modules are blocked or marked review-required; unknown compatibility is never silently treated as supported.

A '?' affordance appears only when a help record exists. It opens an overlay containing purpose, installation effects, required dependencies, storage requirements, firmware status, reboot requirements, source/credits and recovery notes. Help overlays are informational and never perform actions.

## Component registry

The first registry contract uses one record per installed component:

`ur0:data/RtSettingsExtended/registry/modules/<component-id>.json`

The record stores component ID, installed version, package version, file ownership, installation timestamp, enabled state and manifest digest. A missing record means **not detected**, not necessarily absent: diagnostics may report orphan files separately and must not auto-delete them.

## Update security

Update checks compare installed component manifests with a trusted release index. A version match alone is not proof of authenticity. Before installation, Cockpit verifies the manifest signature/provenance, payload digest, firmware range, dependencies and conflicts. Network failure leaves the existing installation untouched. No silent auto-update.

## First installable milestone

The initial Cockpit VPK is a read-only component-audit shell. It detects registry records and provides per-component details and help overlays. It does not edit taiHEN, install SUPRX files, alter boot configuration or claim that the full installer engine is ready.

The first write-capable VPK milestone must add the transaction engine and a complete, independently installable first module. It must include preview, staging, digest verification, backups, rollback, registry commit and uninstall tests before it is called an installer.

## Roadmap

- **Milestone A — Cockpit shell:** VitaSDK application, branded startup screen, controller navigation, component audit, detail view and help overlay.
- **Milestone B — component model:** manifest validation, registry reader/writer, selected-module resolver, dependency/conflict checks and firmware gate.
- **Milestone C — first installable module:** native Settings bootstrap or a self-contained diagnostics module, including reversible install/update/disable/remove.
- **Milestone D — update center:** signed release index, version comparison, download staging and payload verification.
- **Milestone E — native Settings integration:** Rt root menu, nested pages and Rt Manager under Preferences & Settings Management.
- **Milestone F — optional adapters:** Network, Storage, Controllers, Display/Audio, Boot & Appearance and optional VitaDeploy integration.

## VitaDeploy integration

Cockpit may offer an optional **VitaDeploy compatibility entry** that exposes its supported tools or launches VitaDeploy using a verified application URI. It must not copy, replace or impersonate VitaDeploy's own pages. Any Settings entry is optional and appears only when its target is detected and its integration is validated.

## Status boundaries

The initial Cockpit shell is not the final installer. The component audit is read-only. Network update, component payload installation, taiHEN editing, SUPRX integration, actual boot animation switching and physical-console validation remain separate milestones.
