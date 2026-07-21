#include "TabUI.h"
#include "GW2LangPatch.h" 

#include <stdexcept>
#include <string>
#include <imgui/imgui.h>
#include <arcdps_structs.h>

/* proto/globals */
arcdps_exports arc_exports{};
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dptr, HANDLE arcdll, void* mallocfn, void* freefn, uint32_t imguiversion);
extern "C" __declspec(dllexport) void* get_release_addr(uint32_t reason);
arcdps_exports* init_module();
void release_module();
void options_tab();
void imgui_callback(uint32_t not_charsel_or_loading, uint32_t hide_if_combat_or_ooc);

HMODULE g_hSelfModule;
HMODULE arc_dll;
std::string g_initErrorMessage;

// get exports
e3_func_ptr arc_log_file;
e3_func_ptr arc_log;

void ArcLog(const char* message) {
    if (arc_log_file) arc_log_file(const_cast<char*>(message));
}

arcdps_exports* init_module() {
    arc_exports = {};
    bool loading_successful = false;
    std::string initializationError;

    try {
        if (!GW2LangPatch::Initialize(initializationError)) {
            throw std::runtime_error(initializationError.empty()
                ? "GW2LangPatch initialization failed without a diagnostic message."
                : initializationError);
        }
        loading_successful = true;
    }
    catch (const std::exception& e)
    {
        g_initErrorMessage = "Error starting up: ";
        g_initErrorMessage.append(e.what());
        ArcLog(g_initErrorMessage.c_str());
    }
    catch (...)
    {
        g_initErrorMessage = "Error starting up: Unknown exception during GW2LangPatch initialization.";
        ArcLog(g_initErrorMessage.c_str());
    }

    arc_exports.imguivers = IMGUI_VERSION_NUM;
    arc_exports.out_name = "TChinese UI";
    arc_exports.out_build = "1.0.0-fork.5";

    if (loading_successful)
    {
        arc_exports.size = sizeof(arcdps_exports);
        arc_exports.sig = 0x54434849;
        arc_exports.options_end = options_tab;
        arc_exports.imgui = imgui_callback;
    }
    else
    {
        arc_exports.sig = 0;
        arc_exports.size = reinterpret_cast<uintptr_t>(g_initErrorMessage.c_str());
    }

    return &arc_exports;
}

void release_module() {
    GW2LangPatch::Shutdown();
}

void options_tab()
{
    tabui.Draw();
}

void imgui_callback(uint32_t not_charsel_or_loading, uint32_t hide_if_combat_or_ooc) {
    GW2LangPatch::Update();
}

extern "C" __declspec(dllexport) void* get_init_addr(
    char* arcversionstr,
    void* imguicontext,
    void* id3dptr,
    HANDLE arcdll,
    void* mallocfn,
    void* freefn,
    uint32_t imguiversion)
{
    // set all arcdps stuff
    arc_dll = (HMODULE)arcdll;
    arc_log_file = (e3_func_ptr)GetProcAddress(arc_dll, "e3");
    arc_log = (e3_func_ptr)GetProcAddress(arc_dll, "e8");

    // set imgui context && allocation for arcdps dll space
    ImGui::SetCurrentContext(static_cast<ImGuiContext*>(imguicontext));
    ImGui::SetAllocatorFunctions((void* (*)(size_t, void*))mallocfn, (void (*)(void*, void*))freefn);

    return (void*)&init_module;
}

extern "C" __declspec(dllexport) void* get_release_addr(uint32_t reason) {
    return (void*)&release_module;
}

/* dll main -- winapi */
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        g_hSelfModule = hModule;
    }
    return TRUE;
}
