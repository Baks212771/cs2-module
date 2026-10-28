#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <cstdint>
#include <string>

// Простые обёртки для чтения памяти процесса
namespace Memory {

    inline HANDLE g_hProcess = nullptr;

    inline bool Attach(const wchar_t* processName) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return false;

        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);

        bool found = false;
        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, processName) == 0) {
                    g_hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pe.th32ProcessID);
                    found = (g_hProcess != nullptr);
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
        return found;
    }

    inline void Detach() {
        if (g_hProcess) { CloseHandle(g_hProcess); g_hProcess = nullptr; }
    }

    template <typename T>
    inline T Read(uintptr_t address) {
        T value{};
        ReadProcessMemory(g_hProcess, (LPCVOID)address, &value, sizeof(T), nullptr);
        return value;
    }

    inline std::string ReadString(uintptr_t address, size_t maxLen = 64) {
        std::string buf(maxLen, '\0');
        ReadProcessMemory(g_hProcess, (LPCVOID)address, &buf[0], maxLen, nullptr);
        buf.resize(buf.find('\0'));
        return buf;
    }

    inline uintptr_t GetModuleBase(const wchar_t* moduleName, DWORD pid) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
        if (snap == INVALID_HANDLE_VALUE) return 0;

        MODULEENTRY32W me{};
        me.dwSize = sizeof(me);
        uintptr_t base = 0;

        if (Module32FirstW(snap, &me)) {
            do {
                if (_wcsicmp(me.szModule, moduleName) == 0) {
                    base = (uintptr_t)me.modBaseAddr;
                    break;
                }
            } while (Module32NextW(snap, &me));
        }
        CloseHandle(snap);
        return base;
    }

    inline DWORD GetProcessId(const wchar_t* name) {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap == INVALID_HANDLE_VALUE) return 0;

        PROCESSENTRY32W pe{};
        pe.dwSize = sizeof(pe);
        DWORD pid = 0;

        if (Process32FirstW(snap, &pe)) {
            do {
                if (_wcsicmp(pe.szExeFile, name) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32NextW(snap, &pe));
        }
        CloseHandle(snap);
        return pid;
    }
}
