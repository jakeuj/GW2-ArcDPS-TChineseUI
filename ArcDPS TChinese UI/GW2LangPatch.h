#pragma once
#include <Windows.h>
#include <cstdint>

namespace GW2LangPatch {
    bool Initialize(); // for init_module
    void Shutdown(); // release_module

    // for ui
    bool IsChineseEnabled();
    void QueueLanguageToggle(bool enable);

    void Update(); // for imgui_callback
}