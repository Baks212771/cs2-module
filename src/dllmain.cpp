#include <windows.h>
#include <iostream>
#include <string>
#include "../include/OffsetWatcher.hpp"
#include "../include/Memory.hpp"
#include "EntityReader.hpp"
#include "Renderer.hpp"

// Глобальные объекты
OffsetWatcher* g_pWatcher = nullptr;
std::atomic<bool> g_bRunning{ true };

DWORD WINAPI MainThread(LPVOID lpParam) {
    HMODULE hModule = static_cast<HMODULE>(lpParam);

    // Папка с оффсетами (на рабочем столе в проекте)
    std::wstring offsetsPath = LR"(C:\Users\baksb\OneDrive\Рабочий стол\cs222\output)";
    
    g_pWatcher = new OffsetWatcher(offsetsPath);
    g_pWatcher->Start();

    // Ждем инициализации client.dll игры
    uintptr_t clientBase = 0;
    DWORD pid = GetCurrentProcessId();

    while (g_bRunning) {
        clientBase = Memory::GetModuleBase(L"client.dll", pid);
        if (clientBase != 0) break;
        Sleep(500);
    }

    EntityReader reader(clientBase);
    Renderer renderer;
    Config config;

    HWND gameHwnd = FindWindowA("SDL_app", "Counter-Strike 2");
    if (!gameHwnd) {
        gameHwnd = GetForegroundWindow();
    }
    renderer.Init(gameHwnd, config);

    // Основной рабочий цикл
    while (g_bRunning) {
        // Выход по клавише END
        if (GetAsyncKeyState(VK_END) & 0x8000) {
            g_bRunning = false;
            break;
        }

        // Переключение ESP по F11
        if (GetAsyncKeyState(VK_F11) & 1) {
            config.bESP = !config.bESP;
            renderer.SetConfig(config);
        }

        if (g_pWatcher && g_pWatcher->IsValid()) {
            GameOffsets offsets = g_pWatcher->Get();

            RECT clientRect;
            GetClientRect(gameHwnd, &clientRect);
            int width = clientRect.right - clientRect.left;
            int height = clientRect.bottom - clientRect.top;

            if (width > 0 && height > 0) {
                ViewMatrix vm = reader.ReadViewMatrix(offsets);
                int localTeam = reader.ReadLocalTeam(offsets);
                auto players = reader.ReadPlayers(offsets);
                BombData bomb = reader.ReadBomb(offsets);

                renderer.BeginFrame();
                renderer.DrawPlayers(players, localTeam, width, height, vm);
                renderer.DrawBomb(bomb);
                renderer.EndFrame();
            }
        }

        Sleep(10); // ~100 FPS
    }

    if (g_pWatcher) {
        g_pWatcher->Stop();
        delete g_pWatcher;
        g_pWatcher = nullptr;
    }

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        if (hThread) {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}
