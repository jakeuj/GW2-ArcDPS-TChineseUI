#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

namespace GW2LangPatch {
    bool Initialize(std::string& errorMessage); // for init_module
    void Shutdown(); // release_module

    // for ui
    bool IsChineseEnabled();
    void QueueLanguageToggle(bool enable);
    bool IsTradModeRequested();
    bool IsTradModeAvailable();
    const char* GetTradModeStatus();
    void SetTradModeRequested(bool enable);

    void Update(); // for imgui_callback
}
