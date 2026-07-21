#include "TabUI.h"
#include "GW2LangPatch.h"

#include <imgui/imgui.h>

#define IMGUI_UTF8(str) (const char*)u8##str

TabUI tabui;

void TabUI::Draw() {
    bool isChinese = GW2LangPatch::IsChineseEnabled();
    bool isTradModeRequested = GW2LangPatch::IsTradModeRequested();
    bool isTradModeAvailable = GW2LangPatch::IsTradModeAvailable();
    
    if (ImGui::Checkbox(IMGUI_UTF8("開啟內建簡體中文"), &isChinese)) {
        GW2LangPatch::QueueLanguageToggle(isChinese);
    }

    ImGui::SameLine();

    if (!isTradModeAvailable) {
        ImGui::BeginDisabled();
    }

    if (ImGui::Checkbox(IMGUI_UTF8("簡體轉繁體模式"), &isTradModeRequested)) {
        GW2LangPatch::SetTradModeRequested(isTradModeRequested);
    }

    if (!isTradModeAvailable) {
        ImGui::EndDisabled();
        ImGui::TextDisabled(IMGUI_UTF8("簡體轉繁體目前不可用：%s"), GW2LangPatch::GetTradModeStatus());
    }
}
