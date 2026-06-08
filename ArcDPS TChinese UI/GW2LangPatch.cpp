#include "GW2LangPatch.h"
#include <Psapi.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <string>

namespace {
    // --- 內部狀態變數 ---
    constexpr const char* kValidateLanguageAnchor = "ValidateLanguage(language)";
    constexpr const char* kViewAdvanceTextAnchor = "ViewAdvanceText";
    constexpr uint32_t kChineseLanguageId = 5;

    bool g_initialized = false;
    bool g_chineseEnabled = false;
    bool g_hasPendingApply = false;
    bool g_pendingEnable = false;
    bool g_waitingForDeferredCall = false;

    uint32_t g_originalLanguage = 0;
    uint32_t* g_originalLanguagePtr = nullptr;

    using LanguageSetterFn = void(__fastcall*)(uint32_t languageId);
    LanguageSetterFn g_languageSetter = nullptr;

    uint8_t* g_viewAdvanceTextHookPoint = nullptr;
    uint8_t* g_viewAdvanceTextOriginalCallTarget = nullptr;

    // --- Deferred Hook 結構與變數 ---
    struct PendingCallBuffer {
        uintptr_t Function;
        uint64_t Arg0;
        uint64_t CompletedCount;
    };

    PendingCallBuffer g_pendingCall{};
    void* g_callerCodeCave = nullptr;
    std::array<uint8_t, 5> g_callerHookBackup{};
    bool g_callerHookInstalled = false;

    struct CodeBuffer {
        std::array<uint8_t, 512> Bytes{};
        size_t Size = 0;
    };

    // --- 記憶體輔助與除錯輸出 ---
    void DebugLog(const char* message) {
        OutputDebugStringA((std::string("[GW2LangPatch] ") + message + "\n").c_str());
    }

    void AppendU8(CodeBuffer& code, uint8_t value) {
        if (code.Size < code.Bytes.size()) code.Bytes[code.Size++] = value;
    }

    void AppendU32(CodeBuffer& code, uint32_t value) {
        for (int i = 0; i < 4; ++i) AppendU8(code, static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }

    void AppendU64(CodeBuffer& code, uint64_t value) {
        for (int i = 0; i < 8; ++i) AppendU8(code, static_cast<uint8_t>((value >> (i * 8)) & 0xFF));
    }

    bool WriteProtectedMemory(void* target, const void* source, size_t size) {
        DWORD oldProtect = 0;
        if (!VirtualProtect(target, size, PAGE_EXECUTE_READWRITE, &oldProtect)) return false;
        std::memcpy(target, source, size);
        FlushInstructionCache(GetCurrentProcess(), target, size);
        DWORD restoredProtect = 0;
        return VirtualProtect(target, size, oldProtect, &restoredProtect) != 0;
    }

    bool BuildRel32Jump(uint8_t* source, uint8_t* target, std::array<uint8_t, 5>* out) {
        intptr_t delta = target - (source + 5);
        if (delta < INT32_MIN || delta > INT32_MAX) return false;
        (*out)[0] = 0xE9;
        int32_t rel = static_cast<int32_t>(delta);
        std::memcpy(out->data() + 1, &rel, sizeof(rel));
        return true;
    }

    bool AppendRel32Jump(CodeBuffer& code, uint8_t* codeBase, uint8_t* target) {
        uint8_t* source = codeBase + code.Size;
        intptr_t delta = target - (source + 5);
        if (delta < INT32_MIN || delta > INT32_MAX) return false;
        AppendU8(code, 0xE9);
        AppendU32(code, static_cast<uint32_t>(static_cast<int32_t>(delta)));
        return true;
    }

    void* AllocNearMemory(uint8_t* target, size_t size) {
        SYSTEM_INFO sysInfo{};
        GetSystemInfo(&sysInfo);
        uintptr_t granularity = sysInfo.dwAllocationGranularity;
        uintptr_t targetAddress = reinterpret_cast<uintptr_t>(target);
        uintptr_t minAddress = targetAddress > 0x7FFF0000ULL ? targetAddress - 0x7FFF0000ULL : granularity;
        uintptr_t maxAddress = targetAddress + 0x7FFF0000ULL;

        minAddress &= ~(granularity - 1);
        maxAddress &= ~(granularity - 1);

        for (uintptr_t address = minAddress; address < maxAddress; address += granularity) {
            void* result = VirtualAlloc(reinterpret_cast<void*>(address), size, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
            if (result) return result;
        }
        return nullptr;
    }

    bool GetMainModuleRange(uint8_t** base, size_t* size) {
        HMODULE module = GetModuleHandleW(nullptr);
        if (!module) return false;
        MODULEINFO info{};
        if (!GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info))) return false;
        *base = static_cast<uint8_t*>(info.lpBaseOfDll);
        *size = static_cast<size_t>(info.SizeOfImage);
        return *base != nullptr && *size > 0;
    }

    bool IsReadableProtect(DWORD protect) {
        if ((protect & PAGE_GUARD) || (protect & PAGE_NOACCESS)) return false;
        switch (protect & 0xFF) {
        case PAGE_READONLY: case PAGE_READWRITE: case PAGE_WRITECOPY:
        case PAGE_EXECUTE_READ: case PAGE_EXECUTE_READWRITE: case PAGE_EXECUTE_WRITECOPY: return true;
        default: return false;
        }
    }

    bool IsExecutableProtect(DWORD protect) {
        if ((protect & PAGE_GUARD) || (protect & PAGE_NOACCESS)) return false;
        switch (protect & 0xFF) {
        case PAGE_EXECUTE: case PAGE_EXECUTE_READ: case PAGE_EXECUTE_READWRITE: case PAGE_EXECUTE_WRITECOPY: return true;
        default: return false;
        }
    }

    bool IsMemoryRangeUsable(void* ptr, size_t size, bool executable) {
        if (!ptr || size == 0) return false;
        uintptr_t current = reinterpret_cast<uintptr_t>(ptr);
        uintptr_t end = current + size;
        if (end < current) return false;

        while (current < end) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery(reinterpret_cast<void*>(current), &mbi, sizeof(mbi))) return false;
            if (mbi.State != MEM_COMMIT) return false;
            bool usable = executable ? IsExecutableProtect(mbi.Protect) : IsReadableProtect(mbi.Protect);
            if (!usable) return false;
            uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
            if (regionEnd <= current) return false;
            current = regionEnd;
        }
        return true;
    }

    bool IsReadableAddress(void* ptr, size_t size) { return IsMemoryRangeUsable(ptr, size, false); }
    bool IsExecutableAddress(void* ptr) {
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery(ptr, &mbi, sizeof(mbi))) return false;
        return mbi.State == MEM_COMMIT && IsExecutableProtect(mbi.Protect);
    }
    bool IsInRange(uint8_t* ptr, uint8_t* base, size_t size) {
        uintptr_t start = reinterpret_cast<uintptr_t>(base);
        return reinterpret_cast<uintptr_t>(ptr) >= start && reinterpret_cast<uintptr_t>(ptr) < start + size;
    }

    uint8_t* FindBytes(uint8_t* base, size_t size, const uint8_t* pattern, size_t patternSize) {
        for (size_t i = 0; i <= size - patternSize; ++i) {
            if (std::memcmp(base + i, pattern, patternSize) == 0) return base + i;
        }
        return nullptr;
    }

    uint8_t* FindBytesInMemory(uint8_t* base, size_t size, const uint8_t* pattern, size_t patternSize, bool executable) {
        uintptr_t moduleStart = reinterpret_cast<uintptr_t>(base);
        uintptr_t moduleEnd = moduleStart + size;
        uintptr_t current = moduleStart;
        while (current < moduleEnd) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery(reinterpret_cast<void*>(current), &mbi, sizeof(mbi))) break;
            uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            uintptr_t regionEnd = regionStart + mbi.RegionSize;
            if (mbi.State == MEM_COMMIT && (executable ? IsExecutableProtect(mbi.Protect) : IsReadableProtect(mbi.Protect))) {
                uintptr_t scanStart = regionStart > moduleStart ? regionStart : moduleStart;
                uintptr_t scanEnd = regionEnd < moduleEnd ? regionEnd : moduleEnd;
                if (scanEnd > scanStart && scanEnd - scanStart >= patternSize) {
                    uint8_t* found = FindBytes(reinterpret_cast<uint8_t*>(scanStart), static_cast<size_t>(scanEnd - scanStart), pattern, patternSize);
                    if (found) return found;
                }
            }
            current = regionEnd;
        }
        return nullptr;
    }

    uint8_t* FindAsciiLiteral(uint8_t* base, size_t size, const char* text) {
        return FindBytesInMemory(base, size, reinterpret_cast<const uint8_t*>(text), std::strlen(text), false);
    }

    uint8_t* FollowRel32(uint8_t* displacementAddress) {
        if (!IsReadableAddress(displacementAddress, sizeof(int32_t))) return nullptr;
        int32_t displacement = 0;
        std::memcpy(&displacement, displacementAddress, sizeof(displacement));
        return displacementAddress + sizeof(displacement) + displacement;
    }

    uint8_t* FindLeaRipRefTo(uint8_t* base, size_t size, uint8_t* target) {
        uintptr_t moduleStart = reinterpret_cast<uintptr_t>(base);
        uintptr_t moduleEnd = moduleStart + size;
        uintptr_t current = moduleStart;
        while (current < moduleEnd) {
            MEMORY_BASIC_INFORMATION mbi{};
            if (!VirtualQuery(reinterpret_cast<void*>(current), &mbi, sizeof(mbi))) break;
            uintptr_t regionStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
            uintptr_t regionEnd = regionStart + mbi.RegionSize;
            if (mbi.State == MEM_COMMIT && IsExecutableProtect(mbi.Protect)) {
                uintptr_t scanStart = regionStart > moduleStart ? regionStart : moduleStart;
                uintptr_t scanEnd = regionEnd < moduleEnd ? regionEnd : moduleEnd;
                if (scanEnd > scanStart && scanEnd - scanStart >= 7) {
                    for (uintptr_t cursor = scanStart; cursor <= scanEnd - 7; ++cursor) {
                        uint8_t* instr = reinterpret_cast<uint8_t*>(cursor);
                        if (instr[0] == 0x48 && instr[1] == 0x8D && instr[2] == 0x0D) {
                            if (FollowRel32(instr + 3) == target) return instr + 3;
                        }
                    }
                }
            }
            current = regionEnd;
        }
        return nullptr;
    }

    // --- 核心 Hook 邏輯 ---
    bool InstallDeferredCallerHook() {
        if (!g_languageSetter || !g_viewAdvanceTextHookPoint || !g_viewAdvanceTextOriginalCallTarget) return false;
        if (g_callerHookInstalled) return true;

        std::memcpy(g_callerHookBackup.data(), g_viewAdvanceTextHookPoint, g_callerHookBackup.size());
        uint8_t* jumpBack = g_viewAdvanceTextHookPoint + g_callerHookBackup.size();
        g_callerCodeCave = AllocNearMemory(g_viewAdvanceTextHookPoint, 512);
        if (!g_callerCodeCave) return false;

        CodeBuffer code{};
        // 保存現場
        AppendU8(code, 0x9C); // pushfq
        AppendU8(code, 0x50); // push rax
        AppendU8(code, 0x53); // push rbx
        AppendU8(code, 0x51); // push rcx
        AppendU8(code, 0x52); // push rdx
        AppendU8(code, 0x41); AppendU8(code, 0x50); // push r8
        AppendU8(code, 0x41); AppendU8(code, 0x51); // push r9
        AppendU8(code, 0x41); AppendU8(code, 0x52); // push r10
        AppendU8(code, 0x41); AppendU8(code, 0x53); // push r11
        AppendU8(code, 0x48); AppendU8(code, 0x83); AppendU8(code, 0xEC); AppendU8(code, 0x28); // sub rsp, 0x28

        // 檢查佇列並呼叫
        AppendU8(code, 0x48); AppendU8(code, 0xBB); AppendU64(code, reinterpret_cast<uint64_t>(&g_pendingCall));
        AppendU8(code, 0x48); AppendU8(code, 0x8B); AppendU8(code, 0x03); // mov rax, [rbx]
        AppendU8(code, 0x48); AppendU8(code, 0x85); AppendU8(code, 0xC0); // test rax, rax

        size_t jeOffset = code.Size;
        AppendU8(code, 0x0F); AppendU8(code, 0x84); AppendU32(code, 0); // je skip

        AppendU8(code, 0x8B); AppendU8(code, 0x4B); AppendU8(code, 0x08); // mov ecx, [rbx+8]
        AppendU8(code, 0xFF); AppendU8(code, 0xD0); // call rax
        AppendU8(code, 0x48); AppendU8(code, 0xFF); AppendU8(code, 0x43); AppendU8(code, 0x10); // inc qword [rbx+0x10]
        AppendU8(code, 0x48); AppendU8(code, 0xC7); AppendU8(code, 0x03); AppendU32(code, 0); // mov qword [rbx], 0

        size_t skipOffset = code.Size;
        int32_t jeRel = static_cast<int32_t>(skipOffset - (jeOffset + 6));
        std::memcpy(code.Bytes.data() + jeOffset + 2, &jeRel, sizeof(jeRel));

        // 還原現場
        AppendU8(code, 0x48); AppendU8(code, 0x83); AppendU8(code, 0xC4); AppendU8(code, 0x28); // add rsp, 0x28
        AppendU8(code, 0x41); AppendU8(code, 0x5B); // pop r11
        AppendU8(code, 0x41); AppendU8(code, 0x5A); // pop r10
        AppendU8(code, 0x41); AppendU8(code, 0x59); // pop r9
        AppendU8(code, 0x41); AppendU8(code, 0x58); // pop r8
        AppendU8(code, 0x5A); // pop rdx
        AppendU8(code, 0x59); // pop rcx
        AppendU8(code, 0x5B); // pop rbx
        AppendU8(code, 0x58); // pop rax
        AppendU8(code, 0x9D); // popfq

        // 執行原跳轉
        AppendU8(code, 0x48); AppendU8(code, 0xB8); AppendU64(code, reinterpret_cast<uint64_t>(g_viewAdvanceTextOriginalCallTarget));
        AppendU8(code, 0xFF); AppendU8(code, 0xD0); // call rax
        if (!AppendRel32Jump(code, static_cast<uint8_t*>(g_callerCodeCave), jumpBack)) {
            VirtualFree(g_callerCodeCave, 0, MEM_RELEASE);
            g_callerCodeCave = nullptr;
            return false;
        }

        std::memcpy(g_callerCodeCave, code.Bytes.data(), code.Size);
        FlushInstructionCache(GetCurrentProcess(), g_callerCodeCave, code.Size);

        std::array<uint8_t, 5> hookJump{};
        if (!BuildRel32Jump(g_viewAdvanceTextHookPoint, static_cast<uint8_t*>(g_callerCodeCave), &hookJump)) return false;
        if (!WriteProtectedMemory(g_viewAdvanceTextHookPoint, hookJump.data(), hookJump.size())) return false;

        g_callerHookInstalled = true;
        return true;
    }

    void SetLanguageInternal(uint32_t languageId) {
        if (!g_callerHookInstalled && !InstallDeferredCallerHook()) {
            DebugLog("Failed to install deferred caller hook.");
            return;
        }
        g_pendingCall.Arg0 = languageId;
        g_pendingCall.Function = reinterpret_cast<uintptr_t>(g_languageSetter);
        g_waitingForDeferredCall = true;
    }
} // anonymous namespace

namespace GW2LangPatch {

    bool Initialize() {
        uint8_t* base = nullptr;
        size_t size = 0;
        if (!GetMainModuleRange(&base, &size)) return false;

        // 1. 解析 Language Setter
        uint8_t* anchor = FindAsciiLiteral(base, size, kValidateLanguageAnchor);
        if (!anchor) return false;
        uint8_t* parentBlock = FindLeaRipRefTo(base, size, anchor);
        if (!parentBlock) return false;

        uint8_t* originLangPtrAddress = parentBlock + 0x0B;
        uint8_t* setterTargetAddress = parentBlock + 0x24;
        uint8_t* originLangPtr = FollowRel32(originLangPtrAddress);
        uint8_t* setterTarget = FollowRel32(setterTargetAddress);

        if (!originLangPtr || !setterTarget || !IsExecutableAddress(setterTarget)) return false;

        g_originalLanguagePtr = reinterpret_cast<uint32_t*>(originLangPtr);
        g_originalLanguage = *g_originalLanguagePtr;
        g_languageSetter = reinterpret_cast<LanguageSetterFn>(setterTarget);

        // 2. 解析 Caller Hook Diagnostics
        uint8_t* hookAnchor = FindAsciiLiteral(base, size, kViewAdvanceTextAnchor);
        if (!hookAnchor) return false;
        uint8_t* hookRef = FindLeaRipRefTo(base, size, hookAnchor);
        if (!hookRef) return false;

        uint8_t* hookPoint = hookRef - 0x08;
        if (hookPoint[0] != 0xE8) return false; // 必須是 Call 指令

        g_viewAdvanceTextOriginalCallTarget = FollowRel32(hookPoint + 1);
        g_viewAdvanceTextHookPoint = hookPoint;

        g_initialized = true;
        DebugLog("Initialization complete.");
        return true;
    }

    void Shutdown() {
        if (!g_initialized) return;

        if (g_callerHookInstalled && g_viewAdvanceTextHookPoint) {
            WriteProtectedMemory(g_viewAdvanceTextHookPoint, g_callerHookBackup.data(), g_callerHookBackup.size());
        }
        g_callerHookInstalled = false;
        g_pendingCall = {};

        if (g_callerCodeCave) {
            VirtualFree(g_callerCodeCave, 0, MEM_RELEASE);
            g_callerCodeCave = nullptr;
        }

        g_initialized = false;
    }

    bool IsChineseEnabled() {
        return g_chineseEnabled;
    }

    void QueueLanguageToggle(bool enable) {
        if (!g_initialized) return;
        g_pendingEnable = enable;
        g_hasPendingApply = true;
    }

    void Update() {
        if (!g_initialized) return;

        // 檢查延遲調用是否完成
        if (g_waitingForDeferredCall && g_pendingCall.Function == 0) {
            g_waitingForDeferredCall = false;
        }

        // 處理排隊的 UI 請求
        if (g_hasPendingApply) {
            bool enable = g_pendingEnable;
            g_hasPendingApply = false;

            uint32_t target = enable ? kChineseLanguageId : g_originalLanguage;
            SetLanguageInternal(target);
            g_chineseEnabled = enable;
        }
    }

} // namespace GW2LangPatch