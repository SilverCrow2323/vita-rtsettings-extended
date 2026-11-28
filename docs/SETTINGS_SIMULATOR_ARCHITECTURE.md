# PS Vita Settings Simulator

## Purpose
Interactive browser-only representation of the PS Vita Settings experience with an additional Rt:Settings Extended layer. It is a design and navigation simulator, not a firmware emulator or remote-control interface.

## Four implementation boundaries
1. Presentation: 960 × 544 reference viewport, responsive device frame, Vita-inspired wallpaper, status strip, title row, selection list, footer and modal overlays.
2. Navigation: data-driven page registry, stack-based navigation, selected-row state, keyboard, pointer and touch input.
3. Settings model: Sony categories remain separate from Rt categories. Rt pages represent the intended hierarchy and can be compared with the native SUPRX implementation.
4. Device adapters: intentionally absent. Public GitHub Pages must never write to a console or expose private configuration.

## Native/web parity
Each setting should be classified as implemented-native, implemented-simulator, planned, inventory-only or blocked-by-compatibility. The simulator may preview planned features but must label them simulated. Native Settings must expose only implemented native pages.

## Interaction
- Up/Down moves selection; Enter activates; Escape/Backspace and Back pop the stack.
- Pointer/touch selects and activates rows.
- Toggles/selectors update in-memory demo state only.
- Destructive-looking actions use confirmation dialogs.
- Reload restores initial demo state.

## Safety
- No access to ur0, ux0, taiHEN config, plugins, Bluetooth, Wi-Fi or system APIs.
- No plugin installation, activation, removal or config editing.
- Unknown plugin combinations remain review-required; known exclusive groups are documented in data/settings-simulator.json.
- Hardware-specific options are gated by a selected profile. The initial Vita 1000 OLED / 3.65 Ensō profile is illustrative, not detected.
- Sony and Rt remain separate navigation layers; Rt appears as an added Settings category.

## Asset policy
The simulator currently references the wallpaper URL supplied in the project conversation. For reproducible Pages builds, replace the external reference with a checked-in image under site/assets after the original is uploaded to the repository. Do not silently substitute another wallpaper.

## GitHub Pages
Publish the site/ directory as the Pages artifact from the default branch after integration. A feature-branch preview is not assumed to be live.
