#include <arcdps_structs.h>

static arcdps_exports arc_exports = { 0 };

arcdps_exports* init_module() {
	arc_exports.size = sizeof(arcdps_exports);
	arc_exports.sig = 0x54434849;
	arc_exports.imguivers = 19270;
	arc_exports.out_name = "GW2-ArcDPS-TChineseUI";
	arc_exports.out_build = "1.0.0";

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
    return (void*)&init_module;
}

extern "C" __declspec(dllexport) void* get_release_addr(uint32_t reason) {
    return (void*)&release_module;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}