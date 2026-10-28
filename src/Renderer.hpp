#pragma once
#include "../include/GameTypes.hpp"
#include <windows.h>
#include <vector>
#include <string>

// Конфиг — включаем/выключаем фичи
struct Config {
    // ESP включен?
    bool bESP           = true;
    bool bBombTimer     = true;
    bool bOnlyEnemy     = true;    // показывать только врагов

    // Цвета (RGBA 0-255)
    COLORREF colEnemy       = RGB(255, 60, 60);
    COLORREF colTeam        = RGB(60, 200, 255);
    COLORREF colBomb        = RGB(255, 220, 0);
    COLORREF colText        = RGB(255, 255, 255);

    // Размер шрифта ESP
    int fontSize            = 12;
};

// Рендер поверх игры через GDI (простой вариант без DirectX)
class Renderer {
private:
    HWND    m_hwnd = nullptr;
    HDC     m_hdc  = nullptr;
    Config  m_cfg;

    HFONT CreateESPFont(int size) {
        return CreateFontA(
            size, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH, "Tahoma"
        );
    }

    void DrawRect(int x, int y, int w, int h, COLORREF c, int thick = 1) {
        HPEN pen = CreatePen(PS_SOLID, thick, c);
        HPEN oldPen = (HPEN)SelectObject(m_hdc, pen);
        HBRUSH oldBr = (HBRUSH)SelectObject(m_hdc, GetStockObject(NULL_BRUSH));
        Rectangle(m_hdc, x, y, x + w, y + h);
        SelectObject(m_hdc, oldPen);
        SelectObject(m_hdc, oldBr);
        DeleteObject(pen);
    }

    void DrawText2D(int x, int y, const std::string& text, COLORREF c) {
        SetTextColor(m_hdc, c);
        SetBkMode(m_hdc, TRANSPARENT);
        TextOutA(m_hdc, x, y, text.c_str(), (int)text.size());
    }

    void DrawHealthBar(int x, int y, int h, int hp) {
        int barH = (int)(h * (hp / 100.f));
        int by   = y + h - barH;

        // Фон
        RECT bg = { x - 6, y, x - 3, y + h };
        COLORREF bgC = RGB(50, 50, 50);
        HBRUSH bgBr  = CreateSolidBrush(bgC);
        FillRect(m_hdc, &bg, bgBr);
        DeleteObject(bgBr);

        // Шкала HP
        COLORREF hpC = hp > 60 ? RGB(80, 220, 80) : hp > 30 ? RGB(240, 200, 0) : RGB(220, 50, 50);
        RECT bar = { x - 6, by, x - 3, y + h };
        HBRUSH hpBr = CreateSolidBrush(hpC);
        FillRect(m_hdc, &bar, hpBr);
        DeleteObject(hpBr);
    }

public:
    bool Init(HWND hwnd, Config cfg) {
        m_hwnd = hwnd;
        m_cfg  = cfg;
        return hwnd != nullptr;
    }

    void SetConfig(Config cfg) { m_cfg = cfg; }

    void BeginFrame() {
        m_hdc = GetDC(m_hwnd);
    }

    void EndFrame() {
        if (m_hdc) { ReleaseDC(m_hwnd, m_hdc); m_hdc = nullptr; }
    }

    // Рисуем ESP по списку игроков
    void DrawPlayers(
        const std::vector<PlayerData>& players,
        int localTeam,
        int screenW, int screenH,
        const ViewMatrix& vm)
    {
        if (!m_cfg.bESP || !m_hdc) return;

        HFONT font    = CreateESPFont(m_cfg.fontSize);
        HFONT oldFont = (HFONT)SelectObject(m_hdc, font);

        for (const auto& p : players) {
            if (!p.isValid || !p.isAlive || p.isDormant) continue;
            if (m_cfg.bOnlyEnemy && p.team == localTeam) continue;

            COLORREF col = (p.team == localTeam) ? m_cfg.colTeam : m_cfg.colEnemy;

            Vec2 feet, head;
            if (!vm.WorldToScreen(p.origin, feet, screenW, screenH)) continue;
            if (!vm.WorldToScreen(p.head,   head,  screenW, screenH)) continue;

            int boxH = (int)(feet.y - head.y);
            int boxW = (int)(boxH * 0.5f);
            int bx   = (int)(head.x - boxW / 2);
            int by   = (int)(head.y);

            // Боксы
            DrawRect(bx, by, boxW, boxH, col, 1);

            // Полоска HP
            DrawHealthBar(bx, by, boxH, p.health);

            // Имя + HP текст
            std::string info = p.name + " [" + std::to_string(p.health) + "]";
            if (p.isScoped)   info += " [SCOPE]";
            if (p.isDefusing) info += " [DEFUSE]";
            DrawText2D(bx, by - 14, info, m_cfg.colText);
        }

        SelectObject(m_hdc, oldFont);
        DeleteObject(font);
    }

    // Рисуем таймер бомбы
    void DrawBomb(const BombData& b) {
        if (!m_cfg.bBombTimer || !m_hdc || !b.isValid || !b.isPlanted) return;

        HFONT font    = CreateESPFont(16);
        HFONT oldFont = (HFONT)SelectObject(m_hdc, font);

        std::string site = (b.site == 0) ? "A" : "B";
        std::string line1 = "[C4] Site: " + site;
        std::string line2, line3;

        DWORD now = GetTickCount();
        float remainTime = b.blowTime - (now / 1000.f);
        if (remainTime < 0) remainTime = 0.f;

        line2 = "Blow: " + std::to_string((int)remainTime) + "s";

        if (b.isBeingDefused) {
            float defRemain = b.defuseCountdown - (now / 1000.f);
            if (defRemain < 0) defRemain = 0.f;
            line3 = "Defuse: " + std::to_string((int)defRemain) + "s";
        }

        DrawText2D(10, 50, line1, m_cfg.colBomb);
        DrawText2D(10, 68, line2, m_cfg.colBomb);
        if (!line3.empty())
            DrawText2D(10, 86, line3, RGB(100, 255, 100));

        SelectObject(m_hdc, oldFont);
        DeleteObject(font);
    }
};
