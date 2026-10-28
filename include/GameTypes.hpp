#pragma once
#include <cstdint>
#include <cmath>
#include <string>

// Базовые математические типы для работы с координатами CS2
struct Vec3 {
    float x = 0.f, y = 0.f, z = 0.f;

    Vec3 operator-(const Vec3& o) const { return { x - o.x, y - o.y, z - o.z }; }
    Vec3 operator+(const Vec3& o) const { return { x + o.x, y + o.y, z + o.z }; }
    Vec3 operator*(float s)        const { return { x * s,   y * s,   z * s   }; }

    float Length() const { return sqrtf(x * x + y * y + z * z); }
    float Length2D() const { return sqrtf(x * x + y * y); }
    float Dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    bool  IsZero() const { return x == 0.f && y == 0.f && z == 0.f; }
};

struct Vec2 {
    float x = 0.f, y = 0.f;
};

// Матрица проекции 4x4
struct ViewMatrix {
    float m[4][4];

    // Переводит 3D координату в 2D экранные (возвращает false если за камерой)
    bool WorldToScreen(const Vec3& pos, Vec2& screen, int screenW, int screenH) const {
        float w = m[3][0] * pos.x + m[3][1] * pos.y + m[3][2] * pos.z + m[3][3];
        if (w < 0.001f) return false;

        float x = m[0][0] * pos.x + m[0][1] * pos.y + m[0][2] * pos.z + m[0][3];
        float y = m[1][0] * pos.x + m[1][1] * pos.y + m[1][2] * pos.z + m[1][3];

        float invW = 1.f / w;
        screen.x = (screenW / 2.f) + (x * invW) * (screenW / 2.f);
        screen.y = (screenH / 2.f) - (y * invW) * (screenH / 2.f);
        return true;
    }
};

// Структура модели скелета (для доступа к костям)
struct BoneMatrix {
    float val[3][4];

    Vec3 GetPosition() const {
        return { val[0][3], val[1][3], val[2][3] };
    }
};

// Данные об одном игроке
struct PlayerData {
    bool        isValid     = false;
    bool        isDormant   = false;
    bool        isAlive     = false;
    int         team        = 0;
    int         health      = 0;
    int         armor       = 0;
    bool        isScoped    = false;
    bool        isDefusing  = false;
    float       flashAlpha  = 0.f;
    Vec3        origin;
    Vec3        head;
    std::string name;
    int         weaponId    = 0;
};

// Данные о бомбе
struct BombData {
    bool  isValid       = false;
    bool  isPlanted     = false;
    bool  isTicking     = false;
    bool  isBeingDefused = false;
    float blowTime      = 0.f;
    float timerLength   = 0.f;
    float defuseCountdown = 0.f;
    float defuseLength  = 0.f;
    int   site          = 0;   // 0 = A, 1 = B
};
