#pragma once
#include <windows.h>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstdint>

// -----------------------------------------------
// Все нужные смещения CS2 в одном месте
// -----------------------------------------------
struct GameOffsets {
    // client.dll — глобальные указатели
    uintptr_t dwEntityList                          = 0;
    uintptr_t dwViewMatrix                          = 0;
    uintptr_t dwViewAngles                          = 0;
    uintptr_t dwLocalPlayerController               = 0;
    uintptr_t dwLocalPlayerPawn                     = 0;
    uintptr_t dwPlantedC4                           = 0;
    uintptr_t dwGlobalVars                          = 0;

    // C_BaseEntity
    uint32_t m_iHealth                              = 0;
    uint32_t m_iTeamNum                             = 0;
    uint32_t m_lifeState                            = 0;
    uint32_t m_pGameSceneNode                       = 0;
    uint32_t m_vecAbsOrigin                         = 0;

    // CGameSceneNode / CSkeletonInstance
    uint32_t m_bDormant                             = 0;
    uint32_t m_modelState                           = 0;

    // CCSPlayerController
    uint32_t m_hPlayerPawn                          = 0;
    uint32_t m_bPawnIsAlive                         = 0;
    uint32_t m_iPawnHealth                          = 0;
    uint32_t m_iPawnArmor                           = 0;
    uint32_t m_iszPlayerName                        = 0;

    // C_CSPlayerPawn
    uint32_t m_bIsScoped                            = 0;
    uint32_t m_bIsDefusing                          = 0;
    uint32_t m_ArmorValue                           = 0;
    uint32_t m_flFlashDuration                      = 0;
    uint32_t m_vecViewOffset                        = 0;
    uint32_t m_pWeaponServices                      = 0;

    // CPlayer_WeaponServices
    uint32_t m_hActiveWeapon                        = 0;

    // C_EconItemView
    uint32_t m_iItemDefinitionIndex                 = 0;

    // C_PlantedC4
    uint32_t m_flC4Blow                             = 0;
    uint32_t m_flTimerLength                        = 0;
    uint32_t m_bBombTicking                         = 0;
    uint32_t m_bBeingDefused                        = 0;
    uint32_t m_flDefuseCountDown                    = 0;
    uint32_t m_flDefuseLength                       = 0;
    uint32_t m_nBombSite                            = 0;
};

// -----------------------------------------------
// Менеджер: читает JSON из локальной папки
// и автоматически перезагружает при изменении
// -----------------------------------------------
class OffsetWatcher {
private:
    std::wstring    m_folder;
    std::thread     m_thread;
    std::atomic<bool> m_running{ false };
    std::mutex      m_mutex;
    GameOffsets     m_offsets;

    static std::string ReadFile(const std::wstring& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f.is_open()) return "";
        std::ostringstream ss;
        ss << f.rdbuf();
        return ss.str();
    }

    // Ищем "key": <число> в JSON без тяжёлых зависимостей
    static uintptr_t ParseKey(const std::string& json, const std::string& key) {
        std::string pat = "\"" + key + "\":";
        size_t p = json.find(pat);
        if (p == std::string::npos) return 0;
        p += pat.length();
        while (p < json.size() && (json[p] == ' ' || json[p] == '\t' || json[p] == '\n' || json[p] == '\r')) p++;
        if (p >= json.size()) return 0;
        try { return std::stoull(&json[p], nullptr, 0); }
        catch (...) { return 0; }
    }

    void Reload() {
        auto off = ReadFile(m_folder + L"\\offsets.json");
        auto cli = ReadFile(m_folder + L"\\client_dll.json");

        GameOffsets tmp{};

        // --- offsets.json ---
        if (!off.empty()) {
            tmp.dwEntityList                = ParseKey(off, "dwEntityList");
            tmp.dwViewMatrix                = ParseKey(off, "dwViewMatrix");
            tmp.dwViewAngles                = ParseKey(off, "dwViewAngles");
            tmp.dwLocalPlayerController     = ParseKey(off, "dwLocalPlayerController");
            tmp.dwLocalPlayerPawn           = ParseKey(off, "dwLocalPlayerPawn");
            tmp.dwPlantedC4                 = ParseKey(off, "dwPlantedC4");
            tmp.dwGlobalVars                = ParseKey(off, "dwGlobalVars");
        }

        // --- client_dll.json ---
        if (!cli.empty()) {
            tmp.m_iHealth                   = (uint32_t)ParseKey(cli, "m_iHealth");
            tmp.m_iTeamNum                  = (uint32_t)ParseKey(cli, "m_iTeamNum");
            tmp.m_lifeState                 = (uint32_t)ParseKey(cli, "m_lifeState");
            tmp.m_pGameSceneNode            = (uint32_t)ParseKey(cli, "m_pGameSceneNode");
            tmp.m_vecAbsOrigin              = (uint32_t)ParseKey(cli, "m_vecAbsOrigin");
            tmp.m_bDormant                  = (uint32_t)ParseKey(cli, "m_bDormant");
            tmp.m_modelState                = (uint32_t)ParseKey(cli, "m_modelState");
            tmp.m_hPlayerPawn               = (uint32_t)ParseKey(cli, "m_hPlayerPawn");
            tmp.m_bPawnIsAlive              = (uint32_t)ParseKey(cli, "m_bPawnIsAlive");
            tmp.m_iPawnHealth               = (uint32_t)ParseKey(cli, "m_iPawnHealth");
            tmp.m_iPawnArmor                = (uint32_t)ParseKey(cli, "m_iPawnArmor");
            tmp.m_iszPlayerName             = (uint32_t)ParseKey(cli, "m_iszPlayerName");
            tmp.m_bIsScoped                 = (uint32_t)ParseKey(cli, "m_bIsScoped");
            tmp.m_bIsDefusing               = (uint32_t)ParseKey(cli, "m_bIsDefusing");
            tmp.m_ArmorValue                = (uint32_t)ParseKey(cli, "m_ArmorValue");
            tmp.m_flFlashDuration           = (uint32_t)ParseKey(cli, "m_flFlashDuration");
            tmp.m_vecViewOffset             = (uint32_t)ParseKey(cli, "m_vecViewOffset");
            tmp.m_pWeaponServices           = (uint32_t)ParseKey(cli, "m_pWeaponServices");
            tmp.m_hActiveWeapon             = (uint32_t)ParseKey(cli, "m_hActiveWeapon");
            tmp.m_iItemDefinitionIndex      = (uint32_t)ParseKey(cli, "m_iItemDefinitionIndex");
            tmp.m_flC4Blow                  = (uint32_t)ParseKey(cli, "m_flC4Blow");
            tmp.m_flTimerLength             = (uint32_t)ParseKey(cli, "m_flTimerLength");
            tmp.m_bBombTicking              = (uint32_t)ParseKey(cli, "m_bBombTicking");
            tmp.m_bBeingDefused             = (uint32_t)ParseKey(cli, "m_bBeingDefused");
            tmp.m_flDefuseCountDown         = (uint32_t)ParseKey(cli, "m_flDefuseCountDown");
            tmp.m_flDefuseLength            = (uint32_t)ParseKey(cli, "m_flDefuseLength");
            tmp.m_nBombSite                 = (uint32_t)ParseKey(cli, "m_nBombSite");
        }

        std::lock_guard<std::mutex> lock(m_mutex);
        m_offsets = tmp;
    }

    void WatchLoop() {
        HANDLE h = FindFirstChangeNotificationW(
            m_folder.c_str(), FALSE,
            FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_SIZE
        );
        if (h == INVALID_HANDLE_VALUE) return;

        while (m_running) {
            if (WaitForSingleObject(h, 1000) == WAIT_OBJECT_0) {
                Sleep(150); // ждём пока файл полностью записан
                Reload();
                FindNextChangeNotification(h);
            }
        }
        FindCloseChangeNotification(h);
    }

public:
    explicit OffsetWatcher(const std::wstring& folder) : m_folder(folder) {}
    ~OffsetWatcher() { Stop(); }

    void Start() {
        Reload();                  // сразу грузим при запуске
        m_running = true;
        m_thread = std::thread(&OffsetWatcher::WatchLoop, this);
    }

    void Stop() {
        if (m_running.exchange(false) && m_thread.joinable())
            m_thread.join();
    }

    GameOffsets Get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_offsets;
    }

    bool IsValid() {
        auto o = Get();
        return o.dwEntityList != 0 && o.dwViewMatrix != 0;
    }
};
