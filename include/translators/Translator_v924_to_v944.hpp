#pragma once
// ─────────────────────────────────────────────────────────────────────────────
// Translator: 1.26.x (proto 924) → 1.26.1x (server) (proto 944)
// Source: GlacieTeam/ProtocolLib releases table (авторитетный источник)
//
// КАК ЗАПОЛНЯТЬ:
//   1. Найди changelog между 1.26.x и 1.26.1x (server) на minecraft.wiki или
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

class Translator_v924_to_v944 final : public IVersionTranslator {
public:
    [[nodiscard]] std::int32_t fromVersion() const noexcept override {
        return proto_versions::V1_26_0; // 924
    }
    [[nodiscard]] std::int32_t toVersion() const noexcept override {
        return proto_versions::V1_26_10; // 944
    }

    [[nodiscard]] std::unique_ptr<IPacket> translate(
        std::unique_ptr<IPacket> packet,
        TranslateDirection       direction
    ) override {
        using namespace sculk::protocol;

        switch (packet->getId()) {

        // ══════════════════════════════════════════════════════════════════════
        // TODO: Вставить реальные case-ы после изучения diff 1.26.x→1.26.1x (server)
        //
        // ШАБЛОН для поля, добавленного в 1.26.1x (server):
        //
        // case MinecraftPacketIds::StartGame: {
        //     auto* sg = static_cast<StartGamePacket*>(packet.get());
        //     if (direction == TranslateDirection::Clientbound) {
        //         // Клиент 1.26.x не знает mNewField — обнуляем/убираем
        //         sg->mNewFieldAddedIn944 = {};
        //     } else {
        //         // Сервер 1.26.1x (server) ожидает mNewField — проставляем дефолт
        //         // (уже нулевой, но явно)
        //     }
        //     return packet;
        // }
        //
        // ШАБЛОН для нового пакета, появившегося в 1.26.1x (server):
        //
        // case MinecraftPacketIds::SomeNewPacket: {
        //     if (direction == TranslateDirection::Clientbound) {
        //         return nullptr; // Клиент 1.26.x не знает этот пакет — дроп
        //     }
        //     // Serverbound: такой пакет клиент 1.26.x послать не может —
        //     // если дошёл, значит накосячили, дропаем тоже
        //     return nullptr;
        // }
        // ══════════════════════════════════════════════════════════════════════

        default:
            return packet; // Пакет не изменился в этом хопе
        }
    }
};
