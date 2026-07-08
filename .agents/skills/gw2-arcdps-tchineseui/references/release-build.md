# Release And Build Reference

## Current Conventions

- Repository: `jakeuj/GW2-ArcDPS-TChineseUI`
- Upstream remote: `m21248074/GW2-ArcDPS-TChineseUI`
- Upstream/base tag currently used by this fork: `v1.0.0`
- Fork release tag format: `v<upstream-version>-fork.N`
- Latest successful release created during setup: `v1.0.0-fork.4`
- Release assets:
  - `arcdps_tchineseui.dll`
  - `arcdps_tchineseui-<tag>.zip`

Do not use plain semver tags like `v1.1.0` for fork-only releases. Those are reserved for upstream-style/base versions and may conflict with the original author.

## Release Workflow

The release workflow is `.github/workflows/build-and-release.yml`.

Triggers:
- Push tag matching `v*`
- `workflow_dispatch` with optional `new_version`

Build behavior:
- Runs on `windows-latest`
- Checks out submodules recursively
- Uses Visual Studio/MSBuild
- Installs vcpkg manifest dependencies for `x64-windows-static`
- Builds `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj`
- Verifies `ArcDPS TChinese UI/x64/Release/arcdps_tchineseui.dll`
- Uploads the DLL and ZIP to a GitHub Release

Recommended release commands:

```bash
git fetch origin --tags
git status --short --branch
git tag --list 'v*-fork.*' --sort=-v:refname | head
git tag -a v1.0.0-fork.N -m "Release v1.0.0-fork.N"
git push origin v1.0.0-fork.N
gh run list --repo jakeuj/GW2-ArcDPS-TChineseUI --branch v1.0.0-fork.N --limit 5
gh run watch <run-id> --repo jakeuj/GW2-ArcDPS-TChineseUI --exit-status
gh release view v1.0.0-fork.N --repo jakeuj/GW2-ArcDPS-TChineseUI
```

If a tag-triggered release fails, do not move that tag by default. Commit a fix and publish the next fork tag.

## Release Notes Template

Use player-facing Traditional Chinese notes. Avoid saying "added Traditional Chinese support" unless the change truly did that; this repo already exists to provide TChinese UI.

Example:

```markdown
## 更新內容
- 記住 TChinese UI 的啟用狀態，重啟遊戲後會自動套用上次設定。
- 新增 `addons/arcdps/arcdps_tchineseui.ini` 設定檔，保存 `chinese_enabled` 與 `trad_mode_enabled`。
- 將繁簡轉換詞庫改為 repo 內建資源，修正 `Resource.rc` 原本指向作者本機絕對路徑的問題。
- 內建 `jianfan.json` 與 `add.json` 詞庫規則。

## 安裝方式
- 下載下方的 `arcdps_tchineseui.dll` 或 ZIP 檔。
- 將 DLL 放到 Guild Wars 2 目錄，與 `arcdps.dll` 放在同一層。

**Full Changelog**: https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/compare/v1.0.0...v1.0.0-fork.N
```

The GitHub Models release-notes call may fail and fall back to commit-based notes. If that happens, edit the release manually:

```bash
gh release edit v1.0.0-fork.N --repo jakeuj/GW2-ArcDPS-TChineseUI --notes-file /path/to/notes.md
```

## Build Pitfalls

The CI build intentionally avoids relying on vcpkg's MSBuild manifest integration during the compile step because it previously produced `--triplet ""` in GitHub Actions.

Current working pattern:
- `Configure vcpkg` installs dependencies explicitly with `vcpkg install --triplet x64-windows-static`.
- The MSBuild step passes `/p:VcpkgEnabled=false` and `/p:VcpkgManifestInstall=false`.
- `Release|x64` in the `.vcxproj` includes explicit paths:
  - `..\vcpkg_installed\x64-windows-static\include`
  - `..\vcpkg_installed\x64-windows-static\lib`
  - `..\vcpkg_installed\x64-windows-static\lib\manual-link`

Do not remove those explicit paths unless a CI run proves vcpkg/MSBuild integration works without the empty-triplet failure.

Other project details:
- `Release|x64` outputs `arcdps_tchineseui.dll`.
- `Release|x64` uses static runtime/library settings and does not use precompiled headers.
- `Resource.rc` is UTF-16 LE. Use care when patching and do not accidentally convert encoding.
- `Resource.rc` should reference repo-relative files under `ArcDPS TChinese UI/resources/`, not an author's local absolute path.
- The `external/arcdps-extension` submodule is required for includes.

## Upstream Sync

The sync workflow is `.github/workflows/sync-upstream.yml`.

It runs on a schedule or manually, rebases `origin/master` onto `upstream/master`, pushes `automation/sync-upstream`, and opens or updates a PR only when there are changes.

Manual local sync pattern:

```bash
git fetch upstream
git switch codex/persist-tchinese-ui-settings
git rebase upstream/master
git push --force-with-lease origin codex/persist-tchinese-ui-settings
```

Use the scheduled PR workflow for safer routine syncs. Use direct branch rebases only when the user explicitly wants that branch updated.
