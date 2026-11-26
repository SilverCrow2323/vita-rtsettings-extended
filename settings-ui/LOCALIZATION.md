# Localization architecture

## Policy
All user-visible text must be resolved through stable translation keys. UI modules must not embed user-facing labels, descriptions, warnings, confirmation prompts, status strings or error messages directly in renderer code. Protocol names, plugin identifiers, file paths and technical values remain unchanged where translation would be misleading.

## Runtime contract
`include/i18n.h` and `source/i18n/i18n.c` provide a bounded C99 registry:
- stable lowercase dotted keys, locale tags and module ownership;
- duplicate key+locale rejection;
- exact locale lookup, then primary-language lookup (for example `it-IT` → `it`);
- configured fallback locale, then its primary language;
- unresolved keys return the key itself, making missing coverage detectable rather than silently displaying blank text;
- module removal removes only that module's translations.

The registry is renderer-neutral and does not yet read JSON files or detect the Vita system language. Those are integration tasks for the UI/platform layer.

## Locale strategy
Initial release targets:
- `en-US`: mandatory fallback and source language;
- `it-IT`: first complete additional translation.

The catalogue format uses BCP-47-style tags. Future packs can add `fr-FR`, `de-DE`, `es-ES`, `pt-BR`, `ja-JP` and others without changing UI code.

## Required coverage
Every module must provide translations for:
- menu/category/setting names and descriptions;
- option values and contextual help;
- confirmations, warnings and success/failure messages;
- diagnostic states, compatibility explanations and recovery instructions;
- accessibility labels and input hints.

A module is not considered localization-complete until all its registered UI keys exist in the source locale and the supported shipped locales. Translation coverage checks and extraction tooling are planned for the build pipeline.

## Vita UI integration
The future language page belongs under Preferences → Language & Region. Selection precedence:
1. explicit user-selected locale;
2. detected Vita system locale when a platform adapter is available;
3. `en-US` fallback.

Changing language refreshes visible labels without restarting the console. Module-owned translation packs are loaded/unloaded with their module lifecycle. If a translation is missing, fallback occurs per string, never per screen.
