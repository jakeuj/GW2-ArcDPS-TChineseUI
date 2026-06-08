#include "TabUI.h"
#include "GW2LangPatch.h"

#include <imgui/imgui.h>

#define IMGUI_UTF8(str) (const char*)u8##str

TabUI tabui;

void TabUI::Draw() {
    bool isChinese = GW2LangPatch::IsChineseEnabled();
    
    if (ImGui::Checkbox(IMGUI_UTF8("開啟內建簡體中文"), &isChinese)) {
        GW2LangPatch::QueueLanguageToggle(isChinese);
    }

    ImGui::SameLine();

    if (ImGui::Button(IMGUI_UTF8("簡體轉繁體模式"))) {
        // TODO: 簡轉繁模式邏輯
    }
}