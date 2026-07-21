---
name: gw2-arcdps-tchineseui
description: Repository-specific guide for the GW2-ArcDPS-TChineseUI fork. Use when Codex diagnoses extension load or Chinese-application failures; changes memory resolvers, MinHook/AsmJit lifecycle, settings, or UI behavior; builds arcdps_tchineseui.dll with Windows MSBuild/vcpkg; creates candidate artifacts or fork releases; maintains GitHub Actions; packages resource JSON; or syncs/rebases upstream.
---

# GW2 ArcDPS TChinese UI

## Overview

Use this skill to preserve this fork's release and maintenance conventions while working in the repository.

## First Steps

1. Inspect local state with `git status --short --branch` before changing files, tags, or releases.
2. Confirm the current branch and target branch. The maintained feature/release branch is usually `codex/persist-tchinese-ui-settings`; `master` tracks the upstream base.
3. Read `references/runtime-diagnostics.md` for load failures, missing Chinese behavior, memory signatures, hooks, settings application, or unload/reload work.
4. Read `references/release-build.md` for release, candidate artifact, CI, vcpkg, MSBuild, tag, or upstream-sync work.

## Repo Rules

- Treat this as a fork. Do not publish plain upstream-style tags such as `v1.1.0` for fork-only changes.
- Use fork tags in the same style as the user's other forks: `v<upstream-version>-fork.N`, for example `v1.0.0-fork.4`.
- Do not rewrite or force-push existing release tags unless the user explicitly asks. If a tag-triggered release fails, fix forward and create the next `-fork.N` tag.
- Keep release notes player-facing: describe TChinese UI behavior, settings persistence, install steps, and known release assets rather than CI-only details.
- Keep Windows build changes conservative. This repo builds `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj` as `Release|x64` and publishes `arcdps_tchineseui.dll`.
- Never replace the installed DLL while Guild Wars 2 is running. Resolve the loaded path from `arcdps.log`, preserve the previous DLL and INI, and keep rollback possible.
- Treat arcdps's `not_charsel_or_loading` and `hide_if_combat_or_ooc` callback values as visibility hints. Do not gate saved language or conversion setup on them; doing so can miss the game's `ViewAdvanceText` call and leave the UI untranslated.
- Keep required language/`ViewAdvanceText` resolver failures fatal and traditional-conversion failures degradable with a visible reason and precise stage log.
- Detach hooks before freeing their code: disable/remove MinHook, restore the custom caller bytes, free the code cave, release AsmJit, then uninitialize owned MinHook state. Apply the same cleanup to partial initialization failures.
- Build and upload an untagged candidate before publishing a stable fork release. Do not tag a runtime repair until clean startup, settings application, and same-process unload/reload pass in GW2.

## Important Files

- `.github/workflows/build-and-release.yml`: tag/manual release workflow.
- `.github/workflows/sync-upstream.yml`: scheduled upstream sync PR workflow.
- `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj`: MSBuild/vcpkg project settings.
- `ArcDPS TChinese UI/Resource.rc`: UTF-16 resource script; preserve encoding.
- `ArcDPS TChinese UI/resources/*.json`: embedded simplification/traditional conversion resources.
- `.gitmodules`: required `external/arcdps-extension` submodule.

## Validation

After changing this skill, validate its folder with the current `skill-creator` validator:

```powershell
$validator = Join-Path $env:USERPROFILE ".codex\skills\.system\skill-creator\scripts\quick_validate.py"
python $validator .agents\skills\gw2-arcdps-tchineseui
```
