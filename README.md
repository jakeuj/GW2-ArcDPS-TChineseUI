# GW2 ArcDPS 繁體中文 UI

這是 Guild Wars 2 的 ArcDPS 外掛，主要功能是讓遊戲介面使用中文。外掛會先將遊戲畫面切換成簡體中文，再以此為基礎進一步轉換與修正為繁體中文。

此專案是 fork 維護版本。為了避免和原作者未來的版本號衝突，本 fork 的 Release 會使用 `v<上游版本>-fork.N` 格式，例如目前的穩定版 [`v1.0.0-fork.5`](https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/releases/tag/v1.0.0-fork.5)。

## 原作者資訊

原作者在巴哈姆特貼文中發布的版本為 `TChinese UI v1.0.0`，功能介紹為將遊戲畫面改成簡體中文，並以此為基礎進一步轉換成繁體中文。

- 原作者巴哈貼文：<https://forum.gamer.com.tw/C.php?bsn=16901&snA=29614&tnum=9>
- 原始使用方式：DLL 下載後放置在 Guild Wars 2 遊戲根目錄的 `bin64` 裡即可。

本 fork 延續原作者的繁體中文 UI 外掛方向，並補上設定保存、精確初始化診斷、安全的 hook 卸載／重載、GitHub Pages 說明頁與自動化 Release 流程。

## 主要功能

- 提供 Guild Wars 2 ArcDPS 外掛形式的中文 UI 支援。
- 將遊戲畫面改成簡體中文，並進一步翻譯與修正為繁體中文。
- 可在 ArcDPS UI 中切換中文 UI 與繁體轉換模式。
- 記住使用者設定，重啟遊戲後自動套用上次狀態。
- 設定檔儲存在 `addons/arcdps/arcdps_tchineseui.ini`。
- 簡轉繁 hook 暫時不可用時會降級載入，保留簡體中文切換與原本偏好，並在設定頁及 `arcdps.log` 顯示原因。
- 在 `arcdps.log` 記錄初始化、語言套用與 hook 狀態，方便判斷失敗階段。
- 內建簡體轉繁體詞庫與額外詞彙修正。
- 透過 GitHub Actions 自動建置並發布 `arcdps_tchineseui.dll`。

## 專案網頁與效果影片

專案網頁已整理外掛介紹、下載入口、安裝方式、遊戲內啟用方式與效果預覽：

<https://gw2.jakeuj.com/>

<p align="center">
  <a href="https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/raw/refs/heads/codex/persist-tchinese-ui-settings/docs/assets/tchinese-ui-demo.mp4">
    <img
      src="https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/raw/refs/heads/codex/persist-tchinese-ui-settings/docs/assets/tchinese-ui-demo.gif"
      alt="TChinese UI 效果預覽"
      width="720">
  </a>
</p>

高清影片預覽：

影片來源：原作者巴哈貼文 <https://forum.gamer.com.tw/C.php?bsn=16901&snA=29614&tnum=9>

<table>
  <tr>
    <td align="center">
      <video
        src="https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/raw/refs/heads/codex/persist-tchinese-ui-settings/docs/assets/tchinese-ui-demo.mp4"
        controls
        style="max-width:100%;">
      </video>
    </td>
  </tr>
  <tr>
    <td align="center">
      <a href="https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/raw/refs/heads/codex/persist-tchinese-ui-settings/docs/assets/tchinese-ui-demo.mp4">下載 MP4</a>
    </td>
  </tr>
</table>

## 下載

目前穩定版是 [`v1.0.0-fork.5`](https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/releases/tag/v1.0.0-fork.5)。也可以到 Releases 頁面查看所有版本：

<https://github.com/jakeuj/GW2-ArcDPS-TChineseUI/releases>

Release 會提供：

- `arcdps_tchineseui.dll`
- `arcdps_tchineseui-<version>.zip`

## 安裝方式

1. 先安裝 ArcDPS：<https://www.deltaconnected.com/arcdps/>
2. 完全關閉 Guild Wars 2，並確認 `Gw2-64.exe` 已結束；不要在遊戲執行中覆寫 DLL。
3. 備份原本的 `arcdps_tchineseui.dll`，也建議備份 `addons/arcdps/arcdps_tchineseui.ini`。
4. 從 Releases 下載 `arcdps_tchineseui.dll`。
5. 將新 DLL 放到 ArcDPS 實際載入外掛的位置。常見位置是遊戲根目錄或 `bin64`；若不確定，請以 `addons/arcdps/arcdps.log` 記錄的完整路徑為準，不要在兩個位置各留一份。
6. 啟動遊戲，在遊戲中按 `Shift + Alt + T` 叫出 ArcDPS 視窗。
7. 在 ArcDPS 裡啟用擴展 `TChinese UI`，再依需要調整中文 UI 與繁體轉換模式。

> arcdps 與其擴充為第三方工具，不受 ArenaNet 支援，使用風險請自行負擔。

## 疑難排解

安裝後若沒有看到中文效果，請先確認：

- `arcdps.log` 顯示載入的是剛下載的 DLL，而不是另一個位置的舊副本。
- 遊戲中已用 `Shift + Alt + T` 開啟 ArcDPS 視窗。
- ArcDPS 的擴展列表中已啟用 `TChinese UI`。
- 設定頁中的中文 UI 已勾選；需要繁體中文時也要勾選簡轉繁。

正常載入並套用保存的中文與簡轉繁設定時，`arcdps.log` 會依序出現下列關鍵紀錄：

```text
[trad/enable] enabled
[init/complete] ready
[language/queue] language-id=5
[language/apply] completed
```

若出現 `[trad/<階段>] unavailable`，代表外掛已降級載入：簡體中文切換仍可使用，但簡轉繁選項會停用並顯示具體原因。INI 內的 `trad_mode_enabled` 偏好會保留，更新外掛或重新啟動遊戲時會再次嘗試。

若外掛無法載入，`fork.5` 起會在錯誤視窗與 `arcdps.log` 標示確切的 `[init/<階段>]`，不再把所有初始化問題都誤報為 `Memory signatures not found`。回報問題時，請附上版本、DLL 載入完整路徑，以及相關的 `[init/*]`、`[trad/*]`、`[caller/*]`、`[language/*]` 紀錄。

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

即使目前版本無法建立簡轉繁 hook，外掛也會保留 `trad_mode_enabled`，供下次啟動重新嘗試。

## 回復舊版

1. 完全關閉 Guild Wars 2，確認 `Gw2-64.exe` 已結束。
2. 將 `arcdps.log` 所指位置的 DLL 換回先前備份，或改用上一個可正常使用的 Release。
3. 通常可以保留現有 INI；只有舊版明確不相容時才需要另外備份後移除。

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
- fork release tag 使用 `v<上游版本>-fork.N`；目前穩定版是 `v1.0.0-fork.5`，若上游基準不變，下一版為 `v1.0.0-fork.6`
- fork 自己的修改不要使用 `v1.1.0` 這類可能和原作者衝突的版本號
- 如果某個 tag 觸發 Release 失敗，修正後發布下一個 `-fork.N` tag，不要覆寫既有 tag

## 授權

本專案保留 upstream 的 MIT License 與原作者授權資訊。
