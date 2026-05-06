#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Translator: 1.21.8x (proto 800) → 1.21.9x (proto 818)
// Source: GlacieTeam/ProtocolLib releases table (авторитетный источник)
//
// КАК ЗАПОЛНЯТЬ:
//   1. Найди changelog между 1.21.8x и 1.21.9x на minecraft.wiki или
//      в Mojang/bedrock-protocol-docs.
//   2. Для каждого изменённого пакета добавь case ниже.
//   3. Clientbound (DOWN): убери/замени поля, которых нет в более старом клиенте.
//   4. Serverbound (UP):   добавь поля с дефолтом, которые ожидает новый сервер.
//   5. Если пакет не существует в старой версии — return nullptr (Clientbound)
//      или ev.cancel() без re-inject (Serverbound).
// ─────────────────────────────────────────────────────────────────────────────
#include "IVersionTranslator.hpp"
#include "ConnectionManager.hpp"
#include <sculk/protocol/codec/MinecraftPacketIds.hpp>

class Translator_v800_to_v818 final : public IVersionTranslator {
public:
    [[nodiscard]] std::int32_t fromVersion() const noexcept override {
        return proto_versions::V1_21_80; // 800
    }
    [[nodiscard]] std::int32_t toVersion() const noexcept override {
        return proto_versions::V1_21_90; // 818
    }

    [[nodiscard]] std::unique_ptr<IPacket> translate(
        std::unique_ptr<IPacket> packet,
        TranslateDirection       direction
    ) override {
        using namespace sculk::protocol;

        switch (packet->getId()) {

        // ══════════════════════════════════════════════════════════════════════
        // TODO: Вставить реальные case-ы после изучения diff 1.21.8x→1.21.9x
        //
        // ШАБЛОН для поля, добавленного в 1.21.9x:
        //
        // case MinecraftPacketIds::StartGame: {
        //     auto* sg = static_cast<StartGamePacket*>(packet.get());
        //     if (direction == TranslateDirection::Clientbound) {
        //         // Клиент 1.21.8x не знает mNewField — обнуляем/убираем
        //         sg->mNewFieldAddedIn818 = {};
        //     } else {
        //         // Сервер 1.21.9x ожидает mNewField — проставляем дефолт
        //         // (уже нулевой, но явно)
        //     }
        //     return packet;
        // }
        //
        // ШАБЛОН для нового пакета, появившегося в 1.21.9x:
        //
        // case MinecraftPacketIds::SomeNewPacket: {
        //     if (direction == TranslateDirection::Clientbound) {
        //         return nullptr; // Клиент 1.21.8x не знает этот пакет — дроп
        //     }
        //     // Serverbound: такой пакет клиент 1.21.8x послать не может —
        //     // если дошёл, значит накосячили, дропаем тоже
        //     return nullptr;
        // }
        // ══════════════════════════════════════════════════════════════════════

        default:
            return packet; // Пакет не изменился в этом хопе
        }
    }
};
