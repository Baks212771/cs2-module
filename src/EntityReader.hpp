#pragma once
#include "../include/Memory.hpp"
#include "../include/GameTypes.hpp"
#include "../include/OffsetWatcher.hpp"
#include <vector>
#include <cstdint>

// Читает данные всех игроков и бомбы из памяти CS2
class EntityReader {
private:
    uintptr_t m_clientBase;

public:
    explicit EntityReader(uintptr_t clientBase) : m_clientBase(clientBase) {}

    // --- Вспомогательный метод: получить entity pawn по контроллеру ---
    uintptr_t GetPawn(uintptr_t controller, const GameOffsets& o) {
        uintptr_t pawnHandle = Memory::Read<uint32_t>(controller + o.m_hPlayerPawn);
        if (!pawnHandle) return 0;

        uintptr_t entityList = Memory::Read<uintptr_t>(m_clientBase + o.dwEntityList);
        if (!entityList) return 0;

        uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 0x8 * ((pawnHandle & 0x7FFF) >> 9) + 16);
        if (!listEntry) return 0;

        return Memory::Read<uintptr_t>(listEntry + 120 * (pawnHandle & 0x1FF));
    }

    // --- Читаем всех игроков ---
    std::vector<PlayerData> ReadPlayers(const GameOffsets& o) {
        std::vector<PlayerData> players;

        uintptr_t entityList = Memory::Read<uintptr_t>(m_clientBase + o.dwEntityList);
        if (!entityList) return players;

        uintptr_t listEntry = Memory::Read<uintptr_t>(entityList + 16);
        if (!listEntry) return players;

        for (int i = 0; i < 64; i++) {
            uintptr_t controller = Memory::Read<uintptr_t>(listEntry + 120 * i);
            if (!controller) continue;

            // Проверяем что это игрок
            uintptr_t pawn = GetPawn(controller, o);
            if (!pawn) continue;

            PlayerData p{};

            // Жив ли
            p.isAlive = Memory::Read<bool>(controller + o.m_bPawnIsAlive);
            if (!p.isAlive) { players.push_back(p); continue; }

            // Базовые данные
            p.team      = Memory::Read<int>(pawn + o.m_iTeamNum);
            p.health    = Memory::Read<int>(pawn + o.m_iHealth);
            p.armor     = Memory::Read<int>(pawn + o.m_ArmorValue);
            p.isScoped  = Memory::Read<bool>(pawn + o.m_bIsScoped);
            p.isDefusing= Memory::Read<bool>(pawn + o.m_bIsDefusing);
            p.flashAlpha= Memory::Read<float>(pawn + o.m_flFlashDuration);

            // Позиция (через GameSceneNode)
            uintptr_t sceneNode = Memory::Read<uintptr_t>(pawn + o.m_pGameSceneNode);
            if (sceneNode) {
                p.origin = Memory::Read<Vec3>(sceneNode + o.m_vecAbsOrigin);
                p.isDormant = Memory::Read<bool>(sceneNode + o.m_bDormant);
            }

            // Head position (origin + viewOffset)
            Vec3 viewOffset = Memory::Read<Vec3>(pawn + o.m_vecViewOffset);
            p.head = { p.origin.x + viewOffset.x, p.origin.y + viewOffset.y, p.origin.z + viewOffset.z };

            // Имя
            p.name = Memory::ReadString(controller + o.m_iszPlayerName);

            // Оружие
            uintptr_t wepSvc = Memory::Read<uintptr_t>(pawn + o.m_pWeaponServices);
            if (wepSvc) {
                uintptr_t activeWep = Memory::Read<uintptr_t>(wepSvc + o.m_hActiveWeapon);
                if (activeWep) {
                    p.weaponId = Memory::Read<uint16_t>(activeWep + o.m_iItemDefinitionIndex);
                }
            }

            p.isValid = true;
            players.push_back(p);
        }

        return players;
    }

    // --- Читаем данные бомбы ---
    BombData ReadBomb(const GameOffsets& o) {
        BombData b{};

        uintptr_t plantedC4 = Memory::Read<uintptr_t>(m_clientBase + o.dwPlantedC4);
        if (!plantedC4) return b;

        // Разыменование указателя
        plantedC4 = Memory::Read<uintptr_t>(plantedC4);
        if (!plantedC4) return b;

        b.isPlanted      = true;
        b.isTicking      = Memory::Read<bool>(plantedC4 + o.m_bBombTicking);
        b.isBeingDefused = Memory::Read<bool>(plantedC4 + o.m_bBeingDefused);
        b.blowTime       = Memory::Read<float>(plantedC4 + o.m_flC4Blow);
        b.timerLength    = Memory::Read<float>(plantedC4 + o.m_flTimerLength);
        b.defuseCountdown= Memory::Read<float>(plantedC4 + o.m_flDefuseCountDown);
        b.defuseLength   = Memory::Read<float>(plantedC4 + o.m_flDefuseLength);
        b.site           = Memory::Read<int>(plantedC4 + o.m_nBombSite);
        b.isValid        = true;

        return b;
    }

    // --- View Matrix ---
    ViewMatrix ReadViewMatrix(const GameOffsets& o) {
        return Memory::Read<ViewMatrix>(m_clientBase + o.dwViewMatrix);
    }

    // --- Позиция локального игрока ---
    Vec3 ReadLocalOrigin(const GameOffsets& o) {
        uintptr_t pawn = Memory::Read<uintptr_t>(m_clientBase + o.dwLocalPlayerPawn);
        if (!pawn) return {};
        uintptr_t node = Memory::Read<uintptr_t>(pawn + o.m_pGameSceneNode);
        if (!node) return {};
        return Memory::Read<Vec3>(node + o.m_vecAbsOrigin);
    }

    int ReadLocalTeam(const GameOffsets& o) {
        uintptr_t pawn = Memory::Read<uintptr_t>(m_clientBase + o.dwLocalPlayerPawn);
        if (!pawn) return 0;
        return Memory::Read<int>(pawn + o.m_iTeamNum);
    }
};
