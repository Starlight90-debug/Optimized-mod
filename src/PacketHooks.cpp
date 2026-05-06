// PacketHooks.cpp
// Intercepts PacketReceivedEvent (Serverbound) and PacketSentEvent (Clientbound)
// using the LeviLamina 1.26.10 event API.
//
// The key design points:
//   - RequestNetworkSettings (ID 193) carries mClientNetworkVersion — used to
//     populate ConnectionManager.
//   - Every other packet is passed through the TranslationPipeline.
//   - For Clientbound packets we cancel the original event and re-inject the
//     translated packet via NetworkHandler::sendNetworkPacket.

#include "ConnectionManager.hpp"
#include "TranslationPipeline.hpp"

// LeviLamina headers (paths relative to SDK install)
#include <ll/api/event/EventBus.h>
#include <ll/api/event/PacketEvent.h>          // PacketReceivedEvent, PacketSentEvent
#include <ll/api/plugin/NativePlugin.h>
#include <mc/network/NetworkHandler.h>
#include <mc/network/NetworkIdentifier.h>

// SculkCatalystMC/Protocol
#include <sculk/protocol/codec/MinecraftPacketIds.hpp>
#include <sculk/protocol/codec/MinecraftPackets.hpp>
#include <sculk/protocol/codec/packet/RequestNetworkSettingsPacket.hpp>
#include <sculk/protocol/codec/utility/deps/BinaryStream.hpp>
#include <sculk/protocol/codec/utility/deps/ReadOnlyBinaryStream.hpp>

#include <memory>
#include <string>
#include <unordered_map>

using namespace sculk::protocol; // inline abi_v975

// ── Helpers ──────────────────────────────────────────────────────────────────

// Produces a stable string key for a NetworkIdentifier.
static std::string netId(const NetworkIdentifier& id) {
    // NetworkIdentifier provides getAddress() returning a string like "ip:port".
    return id.getAddress();
}

// Deserialise raw packet bytes into a sculk IPacket.
// `raw` is the payload bytes AFTER the length prefix is already stripped
// (LeviLamina gives us the inner payload via event.getPacket()).
static std::unique_ptr<IPacket> deserialise(const std::string_view raw) {
    ReadOnlyBinaryStream stream{raw};
    return MinecraftPackets::readAndCreatePacketFromStream(stream);
}

// Serialise an IPacket back to bytes (with header).
static std::vector<std::byte> serialise(const IPacket& pkt) {
    std::vector<std::byte> buf;
    buf.reserve(256);
    BinaryStream stream{buf};
    pkt.writeWithHeader(stream);
    return buf;
}

// Pipeline cache: clientProtocol -> TranslationPipeline
static std::unordered_map<std::int32_t, TranslationPipeline> gPipelineCache;

static TranslationPipeline& getPipeline(std::int32_t proto) {
    auto it = gPipelineCache.find(proto);
    if (it == gPipelineCache.end()) {
        it = gPipelineCache.emplace(proto, TranslationPipeline{proto}).first;
    }
    return it->second;
}

// ── Hook registration ─────────────────────────────────────────────────────────

void registerPacketHooks(ll::plugin::NativePlugin& plugin) {
    auto& bus = ll::event::EventBus::getInstance();

    // ── SERVERBOUND ──────────────────────────────────────────────────────────
    bus.emplaceListener<ll::event::PacketReceivedEvent>(
        [](ll::event::PacketReceivedEvent& ev) {
            const auto& netIdentifier = ev.getNetworkIdentifier();
            const std::string key     = netId(netIdentifier);

            // 1. Deserialise
            auto sculkPkt = deserialise(ev.getPacket().payload());
            if (!sculkPkt) return;

            const auto id = sculkPkt->getId();

            // 2. Special case: RequestNetworkSettings — register client version.
            if (id == MinecraftPacketIds::RequestNetworkSettings) {
                auto* rnsPkt = static_cast<RequestNetworkSettingsPacket*>(sculkPkt.get());
                const std::int32_t proto = rnsPkt->mClientNetworkVersion;

                if (!ConnectionManager::isSupported(proto)) {
                    // Unsupported version — let it through; server will handle disconnect.
                    return;
                }
                ConnectionManager::get().registerClient(key, proto);
                // No translation needed for this packet itself.
                return;
            }

            // 3. Look up the client's protocol.
            const auto protoOpt = ConnectionManager::get().getProtocol(key);
            if (!protoOpt) return; // Unknown client — pass through unmodified.

            const std::int32_t clientProto = *protoOpt;
            if (clientProto == proto_versions::V1_26_10) return; // Native, no translation.

            // 4. Translate UP to server version.
            auto& pipeline  = getPipeline(clientProto);
            auto translated = pipeline.translateServerbound(std::move(sculkPkt));

            if (!translated) {
                // Packet has no equivalent — drop it silently.
                ev.cancel();
                return;
            }

            // 5. Re-inject.
            // Currently LL's PacketReceivedEvent does not support packet replacement
            // directly; we cancel and manually feed the translated payload back.
            // TODO: replace with official replacement API when available in 1.26.10.
            ev.cancel();

            // Serialise and send back into the server receive pipeline.
            // NetworkHandler::handleDataPacket processes the raw bytes.
            auto bytes = serialise(*translated);
            // ev.getNetworkHandler().handleDataPacket(netIdentifier, bytes);
            // ^ Uncomment and adjust to the actual LL 1.26.10 API signature.
        },
        ll::event::EventPriority::Low
    );

    // ── CLIENTBOUND ──────────────────────────────────────────────────────────
    bus.emplaceListener<ll::event::PacketSentEvent>(
        [](ll::event::PacketSentEvent& ev) {
            const auto& netIdentifier = ev.getNetworkIdentifier();
            const std::string key     = netId(netIdentifier);

            const auto protoOpt = ConnectionManager::get().getProtocol(key);
            if (!protoOpt) return;

            const std::int32_t clientProto = *protoOpt;
            if (clientProto == proto_versions::V1_26_10) return; // Native, no work.

            // 1. Deserialise the outgoing packet.
            auto sculkPkt = deserialise(ev.getPacket().payload());
            if (!sculkPkt) return;

            // 2. Translate DOWN to client version.
            auto& pipeline  = getPipeline(clientProto);
            auto translated = pipeline.translateClientbound(std::move(sculkPkt));

            if (!translated) {
                ev.cancel(); // Drop — no equivalent in older version.
                return;
            }

            // 3. Cancel original and send translated packet.
            ev.cancel();

            auto bytes = serialise(*translated);
            // NetworkHandler sends raw packet bytes to a specific client.
            // Adjust to the exact LL 1.26.10 NetworkHandler API:
            // ev.getNetworkHandler().sendNetworkPacket(netIdentifier, bytes);
        },
        ll::event::EventPriority::Low
    );
}

// ── Cleanup on disconnect ─────────────────────────────────────────────────────

void registerDisconnectHook() {
    auto& bus = ll::event::EventBus::getInstance();
    bus.emplaceListener<ll::event::NetworkDisconnectEvent>(
        [](ll::event::NetworkDisconnectEvent& ev) {
            ConnectionManager::get().removeClient(netId(ev.getNetworkIdentifier()));
        }
    );
}
