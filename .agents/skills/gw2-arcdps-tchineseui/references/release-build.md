# Release And Build Reference

## Current Conventions

- Repository: `jakeuj/GW2-ArcDPS-TChineseUI`
- Upstream remote: `m21248074/GW2-ArcDPS-TChineseUI`
- Upstream/base tag currently used by this fork: `v1.0.0`
- Fork release tag format: `v<upstream-version>-fork.N`
- Known-good release checkpoint (2026-07-21): `v1.0.0-fork.5`
- Next stable fork tag while the upstream base remains `v1.0.0`: `v1.0.0-fork.6`
- Release assets:
  - `arcdps_tchineseui.dll`
  - `arcdps_tchineseui-<tag>.zip`

Do not use plain semver tags like `v1.1.0` for fork-only releases. Those are reserved for upstream-style/base versions and may conflict with the original author.

Verify the current GitHub state before treating the checkpoint above as latest. The `v1.0.0-fork.5` release was built by Actions run `29827492094`; its published DLL SHA-256 is `da58edcb5cb23cd843d14a7705b4e57e422490b6c220fa5ba5f5c698673b181c`.

## Release Workflow

The release workflow is `.github/workflows/build-and-release.yml`.

Triggers:
- Push a stable fork tag matching `v*-fork.*`
- `workflow_dispatch` with optional `new_version` and boolean `publish_release`

Build behavior:
- Runs on `windows-latest`
- Checks out submodules recursively
- Uses Visual Studio/MSBuild
- Installs vcpkg manifest dependencies for `x64-windows-static`
- Builds `ArcDPS TChinese UI/ArcDPS TChinese UI.vcxproj`
- Verifies the DLL is x64, exports `get_init_addr` and `get_release_addr`, has no unexpected dynamic MSVC runtime, and prints SHA-256
- Always uploads the DLL and ZIP as a workflow artifact
- With `publish_release: false` (default), names the build `candidate-<run>-<sha>` and creates no tag or release
- With `publish_release: true`, creates the requested or next stable fork tag and publishes both assets to GitHub Releases

Filter stable tags with the exact regex `^v\d+\.\d+\.\d+-fork\.\d+$` before selecting the latest version. Test tags such as `v1.0.0-fork.999-test20260708` must not affect stable numbering.

Recommended candidate flow:

```powershell
gh workflow run build-and-release.yml --repo jakeuj/GW2-ArcDPS-TChineseUI --ref codex/persist-tchinese-ui-settings -f publish_release=false
gh run list --repo jakeuj/GW2-ArcDPS-TChineseUI --workflow build-and-release.yml --limit 5
gh run watch <run-id> --repo jakeuj/GW2-ArcDPS-TChineseUI --exit-status
gh run download <run-id> --repo jakeuj/GW2-ArcDPS-TChineseUI
```

Do not publish after build-only checks. First run the GW2 startup, saved-settings, unload/reload, character-select, map, combat, and shutdown acceptance in `runtime-diagnostics.md`.

Recommended release commands:

```powershell
git fetch origin --tags
git status --short --branch
git tag --list "v*-fork.*" --sort=-v:refname | Where-Object { $_ -match '^v\d+\.\d+\.\d+-fork\.\d+$' } | Select-Object -First 5
gh workflow run build-and-release.yml --repo jakeuj/GW2-ArcDPS-TChineseUI --ref codex/persist-tchinese-ui-settings -f publish_release=true -f new_version=v1.0.0-fork.N
gh run list --repo jakeuj/GW2-ArcDPS-TChineseUI --workflow build-and-release.yml --limit 5
gh run watch <run-id> --repo jakeuj/GW2-ArcDPS-TChineseUI --exit-status
gh release view v1.0.0-fork.N --repo jakeuj/GW2-ArcDPS-TChineseUI
```

If a tag-triggered release fails, do not move that tag by default. Commit a fix and publish the next fork tag.

## Release Notes Template

Use player-facing Traditional Chinese notes. Avoid saying "added Traditional Chinese support" unless the change truly did that; this repo already exists to provide TChinese UI.

Example:

```markdown
## 更新內容
- 以玩家可觀察到的行為說明本版修正，不要直接貼上 commit 標題。
- 若本版變更初始化或 hook，說明必要功能失敗與簡轉繁降級載入的差異。
- 若本版變更設定，說明保存項目以及重新啟動後的套用行為。

## 安裝方式
- 下載下方的 `arcdps_tchineseui.dll` 或 ZIP 檔。
- 完全關閉 Guild Wars 2 後，備份舊 DLL，再將新 DLL 放到 `arcdps.log` 顯示的實際載入位置；常見位置是遊戲根目錄或 `bin64`，請勿兩邊都保留一份。

## 更新後確認
- 正常套用中文與簡轉繁時，`arcdps.log` 應出現 `[trad/enable] enabled`、`[language/queue] language-id=5` 與 `[language/apply] completed`。

## 回復舊版
- 完全關閉 Guild Wars 2，放回先前備份或上一個可正常使用的 DLL；保留 INI，除非該版本明確不相容。

> arcdps 與其擴充為第三方工具，不受 ArenaNet 支援，使用風險請自行負擔。

**Full Changelog**: https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/compare/<previous-stable-tag>...<current-tag>
```

The GitHub Models release-notes call may fail and fall back to commit-based notes. If that happens, edit the release manually:

```bash
gh release edit v1.0.0-fork.N --repo jakeuj/GW2-ArcDPS-TChineseUI --notes-file /path/to/notes.md
```

After publishing, download both assets from the Release rather than trusting the workspace copy. Recheck the DLL SHA-256, x64 header, exports, dependents, embedded build string, and live GW2 load before declaring the release complete.

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
- A local machine may override the toolset only for diagnostic compilation when v143 is unavailable. Do not treat that binary as the CI/release artifact; keep the project and release workflow on v143 unless a separately validated migration is intentional.

Before accepting an artifact, run `dumpbin /headers`, `/exports`, and `/dependents`; verify x64, both unmangled exports, static runtime behavior, and SHA-256. A non-GW2 smoke process may validate precise load-error lifetime and the release callback, but it cannot prove memory signatures, language application, conversion, or unload safety in the game.

`microsoft/setup-msbuild` may expose MSBuild without adding VC tools such as `dumpbin.exe` to `PATH`. In Actions, resolve `dumpbin.exe` through `%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe`, require the x64 VC tools component, and invoke the returned absolute path.

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
