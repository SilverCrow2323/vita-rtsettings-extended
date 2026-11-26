# Boot Appearance Adapter Decisions

**Research date:** 26 November 2026  
**Target:** PCH-1000 OLED · firmware 3.65 · Ensō

This document separates the underlying engines. Rt presents one Settings experience and one asset library, but it must not pretend these plugins share a file format or install mechanism.

## Candidate matrix

| Engine | Scope | Source state | Rt disposition |
|---|---|---|---|
| CBS-Manager (CBSM) | Static splash, boot animation, console-info animation | Upstream source README verified; no tagged GitHub release returned | First combined boot-screen adapter candidate |
| vita-bootanim | Dedicated Enso boot animation using RCF | Community docs report v1.1 and testing on 3.65 | Retain as animation alternative if target tests or workflow justify it |
| Custom Boot Splash | Static splash image | Upstream README and Brewology notes reviewed | Legacy alternative; do not stack with CBSM |
| Vita-BootSound | WAV startup jingle | Latest tag 1.0 (2018); 3.65 audio caveat documented | Conditional only; no default enablement on 3.65 |
| Custom Warning | Ensō startup warning text | Upstream README documents UTF-16LE + BOM file | Portable UTF-16LE adapter implemented; connect to Vita filesystem/plugin detection and target-test |

## CBS-Manager

Upstream README describes a VPK that installs CBSM into both Ensō boot configuration and taiHEN config. After reboot, it adds **Settings → Theme & Background → Boot Screen**.

Capabilities:
- Static image: ur0:tai/boot_splash.img.
- Animation: ur0:tai/boot_animation.img.
- Console information animation: ur0:tai/lboot_animation.img.
- Asset conversion uses the upstream CBANIM tool and its own file format.

Safety requirements:
1. Inspect and snapshot vs0:tai/boot_config.txt, ur0:tai/boot_config.txt if present, and the active taiHEN config before any install/update/remove.
2. Detect Ensō version and the special 3.65-update365 boot_config caveat.
3. Stage all files and verify them before changing boot_config.
4. Keep a byte-for-byte pre-change copy and a transaction journal.
5. Apply config changes atomically; verify expected plugin lines and paths.
6. Mark the change as reboot-required. Never report success as "active" until a post-reboot check confirms it.
7. Do not promise to hide the original Sony boot logo; upstream explicitly says it remains visible.

## vita-bootanim

Community documentation describes v1.1, test on 3.65 and Ensō support from 3.60 to 3.73. It uses boot.rcf in ur0:tai/, has a 108 MiB maximum animation and supports skipping a faulty animation with L-trigger.

The Rt adapter must:
- Verify the upstream package and converter version.
- Inspect existing CBSM/CustomBootSplash/bootanimations hooks and refuse overlapping boot engines.
- Validate RCF output and the 108 MiB ceiling.
- Keep a last-known-good RCF and restore path.
- Surface dual-buffering and lower-priority advice only if the installed engine version exposes those options.

## Custom Warning

The upstream VitaTweaks README documents ux0:tai/custom_warning.txt encoded as Unicode Little Endian with a FF FE BOM. A BOM-only file produces no warning text.

The Rt adapter must:
- Detect the active taiHEN config root and actual plugin entry.
- Read and preserve the original file bytes (including BOM and line endings).
- Validate UTF-16LE, BOM, maximum length and embedded NUL/control behavior.
- Provide a preview based on the exact text to be written.
- Write through a staged temporary file, verify bytes, then atomically replace.
- Offer Restore Original and Empty Warning as distinct actions.
- Never edit a file when the plugin is absent, disabled or its active config root is ambiguous.

## Vita-BootSound

Upstream release 1.0 is dated 2018. Documented constraints: WAV, no more than 1 MiB, sample rate up to 48 kHz, maximum two channels. Upstream notes possible System Music audio-hook issues above firmware 3.60.

The Rt adapter may list the plugin and inspect the WAV file, but must show 3.65 compatibility as **Unverified** and keep activation disabled until physical testing. Audio sample rate/channel metadata is already represented in the shared asset model.

## Shared Settings presentation

All these features use the same user-facing flow:
- **Boot Animation** — description, engine/version, selected animation, compatibility and reboot state.
- **Boot Splash** — selected image, preview when supported, original-logo limitation and restore.
- **Boot Sound** — selected WAV, format/size/sample-rate/channel validation and preview when supported.
- **Startup Warning** — preview text, encoding status, Restore Original and Empty Warning.
- **Custom Assets Manager** — import, metadata, source, hash, active/backup state and safe cleanup.

The common asset core is portable and host-testable. Filesystem copy/hash, upstream converters, plugin installation, boot_config editing and Sony Settings integration remain separate Vita-side work.
