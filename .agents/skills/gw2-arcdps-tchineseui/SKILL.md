---
name: gw2-arcdps-tchineseui
description: Repository-specific guide for the GW2-ArcDPS-TChineseUI fork. Use when Codex works in this repo on ArcDPS TChinese UI settings, Windows MSBuild/vcpkg builds, GitHub Actions release automation, fork tag naming, GitHub Releases, upstream sync/rebase, resource JSON packaging, or maintenance of the arcdps_tchineseui.dll release workflow.
---

# GW2 ArcDPS TChinese UI

## Overview

Use this skill to preserve this fork's release and maintenance conventions while working in the repository.

## First Steps

1. Inspect local state with `git status --short --branch` before changing files, tags, or releases.
2. Confirm the current branch and target branch. The maintained feature/release branch is usually `codex/persist-tchinese-ui-settings`; `master` tracks the upstream base.
3. For release, CI, vcpkg, MSBuild, tag, or upstream-sync work, read `references/release-build.md`.

## Repo Rules

- Treat this as a fork. Do not publish plain upstream-style tags such as `v1.1.0` for fork-only changes.
- Use fork tags in the same style as the user's other forks: `v<upstream-version>-fork.N`, for example `v1.0.0-fork.4`.
- Do not rewrite or force-push existing release tags unless the user explicitly asks. If a tag-triggered release fails, fix forward and create the next `-fork.N` tag.
- Keep release notes player-facing: describe TChinese UI behavior, settings persistence, install steps, and known release assets rather than CI-only details.
- Keep Windows build changes conservative. This repo builds `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj` as `Release|x64` and publishes `arcdps_tchineseui.dll`.

## Important Files

- `.github/workflows/build-and-release.yml`: tag/manual release workflow.
- `.github/workflows/sync-upstream.yml`: scheduled upstream sync PR workflow.
- `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj`: MSBuild/vcpkg project settings.
- `ArcDPS TChinese UI/Resource.rc`: UTF-16 resource script; preserve encoding.
- `ArcDPS TChinese UI/resources/*.json`: embedded simplification/traditional conversion resources.
- `.gitmodules`: required `external/arcdps-extension` submodule.

## Validation

After changing this skill, run:

```bash
python3 /Users/jakeuj/.codex/skills/.system/skill-creator/scripts/quick_validate.py .agents/skills/gw2-arcdps-tchineseui
```
