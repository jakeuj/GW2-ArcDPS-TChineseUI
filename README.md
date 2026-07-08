# GW2 ArcDPS 繁體中文 UI

這是 Guild Wars 2 的 ArcDPS 外掛，主要功能是讓遊戲介面使用中文。外掛會先將遊戲畫面切換成簡體中文，再以此為基礎進一步轉換與修正為繁體中文。

此專案是 fork 維護版本。為了避免和原作者未來的版本號衝突，本 fork 的 Release 會使用 `v<上游版本>-fork.N` 格式，例如 `v1.0.0-fork.4`。

## 原作者資訊

原作者在巴哈姆特貼文中發布的版本為 `TChinese UI v1.0.0`，功能介紹為將遊戲畫面改成簡體中文，並以此為基礎進一步轉換成繁體中文。

- 原作者巴哈貼文：<https://forum.gamer.com.tw/C.php?bsn=16901&snA=29614&tnum=9>
- 原始使用方式：DLL 下載後放置在 Guild Wars 2 遊戲根目錄的 `bin64` 裡即可。

本 fork 延續原作者的繁體中文 UI 外掛方向，並補上設定保存、GitHub Pages 說明頁與自動化 Release 流程。

## 主要功能

- 提供 Guild Wars 2 ArcDPS 外掛形式的中文 UI 支援。
- 將遊戲畫面改成簡體中文，並進一步翻譯與修正為繁體中文。
- 可在 ArcDPS UI 中切換中文 UI 與繁體轉換模式。
- 記住使用者設定，重啟遊戲後自動套用上次狀態。
- 設定檔儲存在 `addons/arcdps/arcdps_tchineseui.ini`。
- 內建簡體轉繁體詞庫與額外詞彙修正。
- 透過 GitHub Actions 自動建置並發布 `arcdps_tchineseui.dll`。

## 專案網頁與效果影片

專案網頁已整理外掛介紹、下載入口、安裝方式、遊戲內啟用方式與效果預覽：

<https://gw2.jakeuj.com/>

[![TChinese UI 效果預覽](docs/assets/tchinese-ui-demo.gif)](docs/assets/tchinese-ui-demo.mp4)

README 內嵌預覽使用 GIF；高清影片可看：

<docs/assets/tchinese-ui-demo.mp4>

## 下載

請到 Releases 頁面下載最新版：

https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/releases

Release 會提供：

- `arcdps_tchineseui.dll`
- `arcdps_tchineseui-<version>.zip`

## 安裝方式

1. 先安裝 ArcDPS：<https://www.deltaconnected.com/arcdps/>
2. 從 Releases 下載 `arcdps_tchineseui.dll`。
3. 將 `arcdps_tchineseui.dll` 放到 Guild Wars 2 遊戲根目錄的 `bin64` 資料夾。
4. 啟動遊戲。
5. 在遊戲中按 `Shift + Alt + T` 叫出 ArcDPS 視窗。
6. 在 ArcDPS 裡啟用擴展 `TChinese UI`，再依需要調整中文 UI 與繁體轉換模式。

安裝後若沒有看到繁體中文效果，請先確認：

- `arcdps.dll` 與 `arcdps_tchineseui.dll` 都在遊戲根目錄的 `bin64` 資料夾。
- 遊戲中已用 `Shift + Alt + T` 開啟 ArcDPS 視窗。
- ArcDPS 的擴展列表中已啟用 `TChinese UI`。

## 設定檔

外掛會將設定寫入：

```text
addons/arcdps/arcdps_tchineseui.ini
```

目前設定項目：

```ini
[TChineseUI]
chinese_enabled=1
trad_mode_enabled=1
```

## 從原始碼建置

需求：

- Windows
- Visual Studio 2022 與 MSBuild
- vcpkg
- Git submodules

Clone 專案：

```bash
git clone --recurse-submodules https://github.com/jakeuj/GW2-ArcDPS-TChineseUI.git
cd GW2-ArcDPS-TChineseUI
```

建置 `Release|x64`：

```bat
msbuild "ArcDPS TChinese UI\ArcDPS TChinese UI.vcxproj" ^
  /p:Configuration=Release ^
  /p:Platform=x64 ^
  /m ^
  /nologo ^
  /verbosity:minimal
```

輸出位置：

```text
ArcDPS TChinese UI\x64\Release\arcdps_tchineseui.dll
```

## 維護筆記

- CI/CD workflow：`.github/workflows/build-and-release.yml`
- upstream 同步 workflow：`.github/workflows/sync-upstream.yml`
- fork release tag 使用 `v<上游版本>-fork.N`，例如 `v1.0.0-fork.4`
- fork 自己的修改不要使用 `v1.1.0` 這類可能和原作者衝突的版本號
- 如果某個 tag 觸發 Release 失敗，修正後發布下一個 `-fork.N` tag，不要覆寫既有 tag

## 授權

本專案保留 upstream 的 MIT License 與原作者授權資訊。
