# Runtime Diagnostics And Hook Lifecycle

Use this reference for extension load failures, successful loads with no Chinese UI, memory-resolver changes, or unload/reload bugs.

## Evidence First

1. Confirm the exact loaded DLL path and arcdps build from `addons/arcdps/arcdps.log`; do not assume the root or `bin64` copy is active.
2. Check whether `Gw2-64.exe` is running before replacing any DLL. Never overwrite or delete the installed extension while the game is running.
3. Record the installed DLL SHA-256, size, INI values, and relevant stage logs. Preserve unrelated extensions and configuration.
4. Compare against the current official arcdps API and ImGui version at <https://www.deltaconnected.com/arcdps/api/README.txt>; do not freeze mutable ABI facts in the skill.

## Stage Interpretation

Required initialization must fail with a specific `[init/<stage>]` message:

- `module-range`: could not inspect the main executable image.
- `language-*`: failed to resolve or validate the language pointer/setter.
- `view-advance-*`: failed to resolve or validate the deferred caller location/target.

Traditional conversion is optional. A `[trad/<stage>] unavailable` message must leave the extension loaded, preserve `trad_mode_enabled`, disable its checkbox, and show the reason. Relevant stages include dictionary loading, parser anchor/reference, converter signature, MinHook initialization, AsmJit generation, hook creation, and hook enable/disable.

A healthy saved-on startup should produce this sequence, allowing unrelated extension lines between entries:

```text
[init/module-range] ready
[init/language-setter] ready
[init/view-advance] ready
[trad/ready]
[trad/enable] enabled
[init/complete] ready
[caller/install] deferred language caller hook installed
[language/queue] language-id=5
[language/apply] completed
```

Interpret missing entries narrowly:

- No `caller/install`: the first ImGui callback did not queue the saved language or caller-hook installation failed.
- `caller/install` without `language/queue`: required language state is inconsistent.
- `language/queue` without `language/apply`: the queued call never reached the game's `ViewAdvanceText` path; inspect timing and the patched call site.
- `trad/ready` without `trad/enable` while the INI requests it: preference application did not run or `MH_EnableHook` failed.

## Timing Invariant

Queue the saved language on the first available ImGui callback, including character select/loading, and apply the saved traditional-conversion preference immediately after its hook is created. Do not wait for `not_charsel_or_loading != 0`.

The official API defines `not_charsel_or_loading` and `hide_if_combat_or_ooc` as values extensions may use to match arcdps window visibility. They are not readiness guarantees. A tested regression that gated setup on the first value delayed activation by more than two minutes and missed the `ViewAdvanceText` call, so the extension loaded without translating the UI.

## Ownership And Teardown

Track these states independently: MinHook usable/owned, text hook created/enabled, caller hook installed, code cave allocated, AsmJit runtime allocated, user-requested traditional mode, and effective traditional mode.

On shutdown or partial initialization failure:

1. Clear queued language work.
2. Disable and remove the text-converter hook.
3. Restore and verify the original caller bytes.
4. Free the caller code cave only after restoration succeeds.
5. Release AsmJit only after its detour is no longer referenced.
6. Uninitialize owned MinHook state and reset all pointers/flags.

Check every `MH_*` and `JitRuntime::add` result. Retain executable memory rather than freeing a detour that may still be referenced, and emit a precise cleanup-stage error.

## Runtime Acceptance

- Start from a fully closed GW2 process with the intended INI combinations.
- Confirm Chinese and traditional conversion appear and persist after restart.
- Unload/reload the extension at least three times in one process.
- Exercise character select, loading, map change, combat, and clean shutdown.
- If any test fails, keep the build as an untagged candidate and collect the latest relevant stage log.

Keep the existing in-place converter buffer-capacity concern as a separate hardening task; do not mix it into an unrelated signature or lifecycle fix without dedicated design and regression tests.
