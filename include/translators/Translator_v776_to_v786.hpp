#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Translator: 1.21.6x (proto 776) → 1.21.7x (proto 786)
// Source: GlacieTeam/ProtocolLib releases table (авторитетный источник)
//
// КАК ЗАПОЛНЯТЬ:
//   1. Найди changelog между 1.21.6x и 1.21.7x на minecraft.wiki или
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

class Translator_v776_to_v786 final : public IVersionTranslator {
public:
    [[nodiscard]] std::int32_t fromVersion() const noexcept override {
        return proto_versions::V1_21_60; // 776
    }
    [[nodiscard]] std::int32_t toVersion() const noexcept override {
        return proto_versions::V1_21_70; // 786
    }

    [[nodiscard]] std::unique_ptr<IPacket> translate(
        std::unique_ptr<IPacket> packet,
        TranslateDirection       direction
    ) override {
        using namespace sculk::protocol;

        switch (packet->getId()) {

        // ══════════════════════════════════════════════════════════════════════
        // TODO: Вставить реальные case-ы после изучения diff 1.21.6x→1.21.7x
        //
        // ШАБЛОН для поля, добавленного в 1.21.7x:
        //
        // case MinecraftPacketIds::StartGame: {
        //     auto* sg = static_cast<StartGamePacket*>(packet.get());
        //     if (direction == TranslateDirection::Clientbound) {
        //         // Клиент 1.21.6x не знает mNewField — обнуляем/убираем
        //         sg->mNewFieldAddedIn786 = {};
        //     } else {
        //         // Сервер 1.21.7x ожидает mNewField — проставляем дефолт
        //         // (уже нулевой, но явно)
        //     }
        //     return packet;
        // }
        //
        // ШАБЛОН для нового пакета, появившегося в 1.21.7x:
        //
        // case MinecraftPacketIds::SomeNewPacket: {
        //     if (direction == TranslateDirection::Clientbound) {
        //         return nullptr; // Клиент 1.21.6x не знает этот пакет — дроп
        //     }
        //     // Serverbound: такой пакет клиент 1.21.6x послать не может —
        //     // если дошёл, значит накосячили, дропаем тоже
        //     return nullptr;
        // }
        // ══════════════════════════════════════════════════════════════════════

        default:
            return packet; // Пакет не изменился в этом хопе
        }
    }
};
