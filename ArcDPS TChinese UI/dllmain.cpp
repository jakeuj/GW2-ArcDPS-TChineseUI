#include <imgui/imgui.h>

#include <arcdps_structs.h>

/* proto/globals */
arcdps_exports arc_exports{};
extern "C" __declspec(dllexport) void* get_init_addr(char* arcversionstr, void* imguicontext, void* id3dptr, HANDLE arcdll, void* mallocfn, void* freefn, uint32_t imguiversion);
extern "C" __declspec(dllexport) void* get_release_addr(uint32_t reason);
arcdps_exports* init_module();
void release_module();

// get exports
e3_func_ptr arc_log_file;
e3_func_ptr arc_log;

arcdps_exports* init_module() {
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.sig = 0x54434849;
    arc_exports.imguivers = IMGUI_VERSION_NUM;
	arc_exports.out_name = "TChinese UI";
	arc_exports.out_build = "0.0.1";

	return &arc_exports;
}

void release_module() {
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
    arc_log_file = (arc_log_func_ptr)GetProcAddress(arc_dll, "e3");
    arc_log = (arc_log_func_ptr)GetProcAddress(arc_dll, "e8");

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
    return TRUE;
}