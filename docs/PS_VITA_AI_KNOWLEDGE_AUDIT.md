# PS Vita AI Knowledge Skills — technical audit and adoption plan

## Why this reference is in the project

The project now uses [Syscallh00k/PS-Vita-Ai-knowledge-Skills](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills) as a curated research index for VitaSDK, taiHEN, native I/O, VPK packaging, diagnostics and porting workflows.

Repository inspected: `main`, tree `84a1ed88a9705706d30be20bf8a10851cfaf3653` (27 November 2026).

This is a useful collection of engineering notes and task playbooks, not an authoritative SDK specification. Its README says the material was gathered from real projects and assembled using DeepSeek. Individual claims therefore need source-level verification against VitaSDK headers, taiHEN headers, pinned upstream implementations and the target firmware. We must not turn a plausible note into an API guarantee.

## First-pass findings relevant to Rt:Settings Extended

### 1. Native Sony Settings integration

Relevant reading:

- [Skill: taiHEN plugin](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/skills/skill-taihen-plugin.md)
- [18 — taiHEN plugin development](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/18-taihen-plugin-development.md)
- [17 — NID and import system](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/17-nid-and-import-system.md)

Adopted engineering rules:

- Prefer a user-mode `.suprx` for Settings UI integration. Do not introduce a kernel plugin unless a demonstrated kernel-only capability requires it.
- Track every hook handle and release it on the corresponding module unload and on plugin stop.
- Hook installation must be non-blocking, checked, reversible and fail-closed.
- Do not patch an address merely because a sample contains an offset. Verify the target module, firmware, import library/function NIDs or a build-specific signature before installing a hook.
- Log hook installation failures and leave the original Settings flow untouched.
- Treat PAF object ownership, callback lifetime and thread affinity as explicit contracts.

Important limitation: the sample hook skeleton in the knowledge repository contains a placeholder offset and is not suitable for copy/paste into Rt. The notes explain the general taiHEN pattern; they do not prove that a particular Settings hook is safe on our firmware.

Our existing [Native Sony Settings adapter design](NATIVE_SETTINGS_ADAPTER.md) identifies the YAMT and h-encore implementations as concrete research targets. Their pinned source is the evidence for the proposed `SceSettings` / PAF lifecycle, XML-loader and callback techniques. The exact Rt hook implementation still needs a source-level audit, module identity checks, signature/NID validation and repeated open/close testing on firmware 3.65.

**Decision:** retain the dedicated Rt category + embedded Rt-owned page strategy. Never replace Sony's complete root XML with a reduced copy. Do not claim native integration until the real SUPRX is loaded by the Settings title and the page is exercised on hardware.

### 2. VitaSDK build and VPK packaging

Relevant reading:

- [Skill: build VPK](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/skills/skill-build-vpk.md)
- [Skill: validate VPK](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/skills/skill-vpk-validate.md)
- [03 — Build and packaging](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/03-build-and-packaging.md)
- [52 — Building existing projects](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/52-building-existing-projects.md)
- [39 — Tooling scripts](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/39-tooling-scripts.md)

Adopted pipeline requirements:

1. Pin the VitaSDK container/toolchain version rather than relying indefinitely on `latest`.
2. Build the actual SUPRX module and installer executable as distinct targets; the existing `librtsettings.a` is only a static library.
3. Produce the SELF with the SDK toolchain and package a VPK with a unique nine-character title ID, generated `param.sfo`, root-level `eboot.bin`, and validated `sce_sys` assets.
4. Keep debug symbols/unstripped ELF as CI artifacts for diagnosis.
5. Inspect the final VPK archive, validate metadata and image formats, and hash the exact artifact that is published.
6. Keep installer payloads, plugin payloads and persistent user data separate. Never put mutable configuration or archives under `app0:`.

**Important product distinction:** a VPK installs a homebrew bubble. It does not by itself register a taiHEN plugin inside Sony Settings. The installer must stage and verify the SUPRX, inspect the existing taiHEN configuration, present the exact proposed `*NPXS10015` entry and obtain explicit confirmation before changing it. It must preserve unrelated lines and offer rollback. This is an installer responsibility, not an automatic side effect of VPK packaging.

The knowledge repository's `check_vpk.py` is explicitly described as an upstream script that is not included in that repository. Do not claim to run it until we vendor or fetch the script under a reviewed licence and pin its revision. We can implement equivalent independent checks in our own CI in the meantime.

### 3. Vita filesystem and Tai Saver

Relevant reading:

- [06 — Filesystem and I/O](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/06-filesystem-and-io.md)
- [01 — Hardware and memory](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/01-vita-hardware-and-memory.md)

Adopted rules:

- Treat `app0:` as read-only; store Tai Saver archives and settings under the Rt-owned data directory.
- Use device-prefixed paths and create directories explicitly.
- Check every open/read/write/close/sync result; do not treat a short read or write as success.
- Keep backup, verification and restore work off the Settings UI thread.
- Bound file counts, manifest sizes, path lengths, copy buffers and progress reporting.
- Re-verify the source and destination around copy/restore operations; retain a verified Pre-Restore snapshot before replacing live files.

Audit item for the next implementation pass: `source/tai-saver/tai_saver.c` currently uses a POSIX-style filesystem layer for most directory, file and traversal operations, while only selected locking/sync paths are Vita-conditional. A successful static-library cross-build proves compilation, not that every POSIX operation has the required Vita semantics or that locking/rename/directory traversal behave correctly on the target. We must validate each used call against the installed VitaSDK headers and console behavior, then add a dedicated Vita I/O backend where the semantics differ or need stronger guarantees.

Do not infer crash consistency from a successful `rename` call. Atomic replacement, lock behavior and directory durability need explicit evidence; otherwise the operation must be described as best-effort or rejected.

### 4. Firmware, 32-bit ABI and kernel dependencies

Relevant reading:

- [36 — 32-bit portability audit](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/36-32bit-portability-audit.md)
- [37 — kubridge and executable memory](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/37-kubridge-and-executable-memory.md)

Adopted rules:

- Audit pointer/size assumptions for ARMv7 and 32-bit `size_t`/pointer widths.
- Avoid unaligned struct access and implicit on-disk enum layouts.
- Keep kernel-only APIs out of the user-mode Settings adapter.
- Declare any optional plugin dependency and minimum version; probe it at runtime and fail gracefully.
- Do not add kubridge merely because it appears in a plugin-development guide. The first Rt Settings slice should not need executable-memory patching or kernel writes if the import-hook design is sufficient.

The knowledge notes include empirical findings from specific ports. Compiler flags, enum behavior and firmware offset tables are not blanket rules for every translation unit. Keep this project's C99 ABI and public interfaces stable; do not adopt the repository's C++ house style wholesale.

### 5. Diagnostics and validation

Relevant reading:

- [14 — Debugging and logging](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/14-debugging-and-logging.md)
- [15 — Crash and error diagnostics](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/15-crash-and-error-diagnostics.md)
- [25 — Testing with Vita3K](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills/blob/main/docs/25-testing-with-vita3k.md)

Adopted test layers:

- Host regression + ASan/UBSan for portable logic.
- VitaSDK cross-build for target compilation and linking.
- Static inspection of SUPRX imports, exports and package contents.
- Vita3K for early application/package smoke tests where the feature is supported.
- Physical PS Vita testing for taiHEN load order, PAF hooks, Settings lifecycle, storage semantics and recovery behavior.

An emulator pass is not proof of correct taiHEN/PAF behavior on a physical 3.65 Ensō console.

## Claims we will not copy blindly

| Knowledge note | Rt policy |
|---|---|
| A generic hook skeleton with offset `0x0` | Placeholder only; never a usable target |
| “Use NID hooks for shipping” | Prefer stable imports when verified; NID alone does not guarantee ABI or lifecycle safety |
| “VPK packaging is straightforward” | Validate the actual SELF, SFO, archive paths, assets and installer transaction |
| POSIX-like I/O examples | Cross-build is insufficient; verify Vita runtime semantics |
| `UNSAFE` / executable-memory advice | Not needed unless our own implementation proves it needs writable code pages |
| kubridge version/firmware table | Optional dependency only; verify upstream release and exact API used |
| universal compiler flag recipes | Apply only to the translation unit and ABI they were measured for |

## Implementation consequences and next gates

The reference changes our next work order:

1. **Tai Saver Vita I/O audit:** enumerate every filesystem call used by `tai_saver.c`; verify signatures, flags, error codes, lock semantics, short I/O handling, directory iteration and sync behavior against VitaSDK. Split the backend from portable snapshot logic if necessary.
2. **SUPRX skeleton:** create a minimal user plugin with correct exports, start/stop lifecycle, structured logging and no unverified patch. Confirm it loads in the Settings title before adding hooks.
3. **PAF adapter proof:** implement one guarded hook using a verified import/NID path from a pinned reference, inspect the actual Settings module identity and XML request, and prove clean hook release on unload.
4. **Rt page + callbacks:** embed a minimal Rt page and wire one harmless read-only status row first. Only then add Tai Saver operations and worker/progress reporting.
5. **Installer/VPK:** package the installer and SUPRX, add archive/SFO/SELF checks, transactional staging, explicit taiHEN config confirmation, rollback and data preservation.
6. **Hardware acceptance:** repeated Settings open/close, boot/reboot, backup/verify/restore, simulated low-space and interrupted-operation recovery on the target Vita 1000 / 3.65 Ensō.

The macro-section remains open until the native page, real Tai Saver backend, installer VPK and physical-device acceptance gates are met.

## Source repository

[PS-Vita-Ai-knowledge-Skills](https://github.com/Syscallh00k/PS-Vita-Ai-knowledge-Skills) — external reference, not a dependency or a source of copied implementation code. Re-check its history before future major upgrades.


## Additional reference: sillysquiredev/vita-packages

Repository inspected: [sillysquiredev/vita-packages](https://github.com/sillysquiredev/vita-packages), default branch `main`, tree `689b86875b79fe5f39dd0ddebd3964fd19b45363`. Repository metadata describes it as custom PS Vita build scripts; the inspected tree contains package recipes for taiHEN, kubridge, SceLibcPosix, SDL variants, audio libraries and graphics libraries. The repository has no README at its root and no declared licence in its GitHub metadata. Treat it as a technical reference, not as code we can automatically vendor or redistribute.

### What is useful

- **Pinned upstream inputs:** several recipes pin release tarballs with SHA-256 values or Git commits. This is a good pattern for reproducible dependency builds.
- **VitaSDK toolchain integration:** the recipes consistently use `$VITASDK/share/vita.toolchain.cmake` or `--host=arm-vita-eabi`, and stage installation through `DESTDIR`. This is useful when we eventually build the SUPRX and installer as separate target artifacts.
- **Dependency metadata:** recipes declare package dependencies (for example, libvorbis → libogg and opusfile → libogg + opus). The idea of an explicit dependency graph is worth adopting for Rt's own installer manifest.
- **Target-specific patches:** the libxmp and mpg123 examples show how a Vita-specific temporary directory, CPU feature handling or signal behavior can be isolated in a patch rather than scattered through generic build flags.
- **SDK compatibility checks:** the kubridge recipe checks for a header member before applying a compatibility patch. This is a useful pattern in principle: detect the actual SDK shape rather than assuming every VitaSDK installation has the same header revision.
- **Separate build and package stages:** the recipes distinguish compiling from staging installed headers/libraries/modules. Rt should preserve the same separation between compilation, packaging and console installation.

### Important limitations found during inspection

- The repository's `build.sh` expects each package directory to contain a generated `PKGBUILD`, reads `pkgname=` and `depends=(...)` from it, and invokes `vita-makepkg`. The checked-in package descriptors are named `VITABUILD`; therefore the script is not a self-contained generic builder when run against the raw tree. It assumes the surrounding VitaSDK/VITABUILD packaging environment.
- Some recipes use `sha256sums=('SKIP')`, unpinned Git sources or release assets without a recorded digest. Those inputs are not reproducible or integrity-checked to the same standard as the pinned recipes.
- The dependency parser in `build.sh` is a small grep/sed parser for a narrow PKGBUILD shape, not a general package metadata parser. Rt should not reuse it as its installer dependency resolver.
- The `kubridge` package is a kernel-side dependency and is unrelated to our first user-mode Settings/Tai Saver slice. Do not add it simply because the package collection builds it.
- `SceLibcPosix` is a compatibility layer with a kubridge dependency. It is not a substitute for validating native Vita I/O semantics, and it would add an unnecessary dependency chain to Tai Saver.
- The `flac` recipe hardcodes NEON compiler flags; the `opus` recipe sets both `OPUS_MAY_HAVE_NEON=OFF` and `OPUS_PRESUME_NEON=ON`. These are package-specific choices to verify against their exact upstream versions, not global Rt compiler settings.
- The `pvr_psp2` recipe consumes prebuilt archives and SUPRX payloads as package inputs. This is a reminder that a package recipe can stage existing binaries; it does not prove that those binaries were built from the checked-out source in that run.
- GitHub metadata reports no declared licence. Before copying any recipe, patch or script into Rt, obtain and review the upstream licensing terms and preserve attribution where required.

### Decisions for Rt:Settings Extended

1. **Do not import the package collection wholesale.** Rt currently needs its own SUPRX, installer executable, Vita filesystem adapter and VPK validation. It does not need SDL, audio codecs, graphics libraries, kubridge or SceLibcPosix for Macro-section 01.
2. **Adopt reproducible-build conventions:** pin every external source to a release or commit, record SHA-256 for downloaded archives, fail closed when a required checksum is missing, and record the VitaSDK/container version used by CI.
3. **Keep target build outputs explicit:** build the Settings plugin and installer separately, stage their payloads into a clean package root, then validate the final VPK independently. A successful library build is not a distributable.
4. **Use dependency metadata only for actual runtime requirements.** The installer may resolve declared module dependencies, but it must not install optional kernel plugins or compatibility layers without a demonstrated feature requirement and explicit compatibility checks.
5. **Use conditional SDK compatibility patches sparingly.** Prefer upstream-supported APIs; when a patch is unavoidable, pin its source and patch hashes, test both the expected and unsupported SDK cases, and document the header/version predicate.
6. **Keep source builds separate from binary packaging.** Any prebuilt SUPRX or library staged into a package must be identified as a prebuilt input, with source, version and hash recorded.

### Inspected files

- [build.sh](https://github.com/sillysquiredev/vita-packages/blob/main/build.sh)
- [bulk.sh](https://github.com/sillysquiredev/vita-packages/blob/main/bulk.sh)
- [taiHEN package recipe](https://github.com/sillysquiredev/vita-packages/blob/main/taihen/VITABUILD)
- [kubridge recipe and SDK compatibility patch](https://github.com/sillysquiredev/vita-packages/tree/main/kubridge)
- [SceLibcPosix recipe](https://github.com/sillysquiredev/vita-packages/blob/main/SceLibcPosix/VITABUILD)
- [libxmp recipe and Vita patch](https://github.com/sillysquiredev/vita-packages/tree/main/libxmp)
- [mpg123 recipe and Vita patch](https://github.com/sillysquiredev/vita-packages/tree/main/mpg123)
- [SDL2 VitaGL recipe](https://github.com/sillysquiredev/vita-packages/blob/main/sdl2_vitagl/VITABUILD)
- [SDL3 PVR recipe](https://github.com/sillysquiredev/vita-packages/blob/main/sdl3_pvr_psp2/VITABUILD)

**Rt conclusion:** this repository is valuable as a collection of Vita-targeted dependency build examples and reproducibility patterns. It does not supply a ready-made Settings plugin or VPK installer for Rt, and it does not change the Macro-section 01 gate: Tai Saver's native Vita I/O, the Settings SUPRX/page integration, and the installable VPK still need to be implemented and validated in this project.
