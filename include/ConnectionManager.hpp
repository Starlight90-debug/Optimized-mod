#pragma once

// ConnectionManager.hpp
// ─────────────────────────────────────────────────────────────────────────────
// ИСТОЧНИК ИСТИНЫ: GlacieTeam/ProtocolLib releases (авторы оригинала Glacie)
// https://github.com/GlacieTeam/ProtocolLib/releases
//
// ПОЛНАЯ ТАБЛИЦА (верифицировано по changelog):
//   1.21.4x  ≤ 748   — ProtocolLib НЕ поддерживает, мы тоже не будем
//   1.21.5x    766
//   1.21.6x    776
//   1.21.7x    786
//   1.21.8x    800
//   1.21.9x    818, 819  (два хотфикса)
//   1.21.10x   827
//   1.21.11x   844
//   1.21.12x   859, 860  (два хотфикса)
//   1.21.13x   898
//   1.26.x     924       (первый 1.26 релиз)
//   1.26.1x    944       (сервер — LeviLamina 1.26.10)
//
// ВАЖНО: «1.26.10» в терминах LeviLamina = «1.26.1x» в таблице GlacieTeam
//         Его протокол = 944, а НЕ 830, как мы ошибочно писали раньше.
//
// Версии с двумя номерами (818/819, 859/860) — это одна игровая версия
// с патч-релизом. Оба номера должны быть приняты как одна точка в цепочке.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdint>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace proto_versions {

// Минимальная поддерживаемая версия (ProtocolLib не поддерживает ≤748)
inline constexpr std::int32_t V1_21_50  = 766;   // 1.21.5x
inline constexpr std::int32_t V1_21_60  = 776;   // 1.21.6x
inline constexpr std::int32_t V1_21_70  = 786;   // 1.21.7x
inline constexpr std::int32_t V1_21_80  = 800;   // 1.21.8x
inline constexpr std::int32_t V1_21_90  = 818;   // 1.21.9x (также 819 — тот же хоп)
inline constexpr std::int32_t V1_21_90b = 819;   // 1.21.9x hotfix
inline constexpr std::int32_t V1_21_100 = 827;   // 1.21.10x
inline constexpr std::int32_t V1_21_110 = 844;   // 1.21.11x
inline constexpr std::int32_t V1_21_120 = 859;   // 1.21.12x (также 860)
inline constexpr std::int32_t V1_21_120b= 860;   // 1.21.12x hotfix
inline constexpr std::int32_t V1_21_130 = 898;   // 1.21.13x
inline constexpr std::int32_t V1_26_0   = 924;   // 1.26.x
inline constexpr std::int32_t V1_26_10  = 944;   // 1.26.1x — СЕРВЕР (LeviLamina 1.26.10)

// Минимальный и максимальный принимаемый протокол
inline constexpr std::int32_t PROTO_MIN = V1_21_50;  // 766
inline constexpr std::int32_t PROTO_MAX = V1_26_10;  // 944

// Нормализация: 819→818, 860→859. Хотфиксы ведут себя как предыдущий номер.
inline constexpr std::int32_t normalize(std::int32_t p) noexcept {
    if (p == V1_21_90b)  return V1_21_90;
    if (p == V1_21_120b) return V1_21_120;
    return p;
}

} // namespace proto_versions

class ConnectionManager {
public:
    static ConnectionManager& get() noexcept {
        static ConnectionManager instance;
        return instance;
    }

    // proto — raw mClientNetworkVersion из RequestNetworkSettingsPacket
    void registerClient(const std::string& key, std::int32_t proto) {
        std::unique_lock lock(mMutex);
        mMap[key] = proto_versions::normalize(proto);
    }

    [[nodiscard]] std::optional<std::int32_t> getProtocol(const std::string& key) const {
        std::shared_lock lock(mMutex);
        if (auto it = mMap.find(key); it != mMap.end()) return it->second;
        return std::nullopt;
    }

    void removeClient(const std::string& key) {
        std::unique_lock lock(mMutex);
        mMap.erase(key);
    }

    [[nodiscard]] static bool isSupported(std::int32_t proto) noexcept {
        const auto n = proto_versions::normalize(proto);
        return n >= proto_versions::PROTO_MIN && n <= proto_versions::PROTO_MAX;
    }

private:
    ConnectionManager() = default;
    ConnectionManager(const ConnectionManager&) = delete;
    ConnectionManager& operator=(const ConnectionManager&) = delete;

    mutable std::shared_mutex mMutex;
    std::unordered_map<std::string, std::int32_t> mMap;
};
