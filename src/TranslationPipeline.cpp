// TranslationPipeline.cpp

#include "TranslationPipeline.hpp"
#include "ConnectionManager.hpp" // proto_versions constants
#include <algorithm>
#include <stdexcept>

TranslationPipeline::TranslationPipeline(std::int32_t clientProtocol)
    : mClientProtocol(clientProtocol) {

    // Collect all translators whose fromVersion falls in [clientProtocol, serverProto).
    const std::int32_t serverProto = proto_versions::V1_26_10;

    if (clientProtocol == serverProto) {
        // Native client — no translation needed; mSteps stays empty.
        return;
    }

    if (clientProtocol > serverProto) {
        throw std::runtime_error("Client protocol newer than server — not supported");
    }

    // Gather the full registered list, then walk from clientProtocol up to serverProto.
    const auto& all = TranslatorRegistry::get().all();

    // Sort a working copy by fromVersion ascending.
    auto sorted = all;
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a->fromVersion() < b->fromVersion();
    });

    std::int32_t current = clientProtocol;
    while (current < serverProto) {
        // Find the translator whose fromVersion == current.
        auto it = std::find_if(sorted.begin(), sorted.end(), [current](const auto& t) {
            return t->fromVersion() == current;
        });

        if (it == sorted.end()) {
            throw std::runtime_error(
                "No translator found for protocol hop starting at " + std::to_string(current)
            );
        }

        mSteps.push_back(*it);
        current = (*it)->toVersion();
    }
}

std::unique_ptr<IPacket>
TranslationPipeline::translateServerbound(std::unique_ptr<IPacket> pkt) const {
    for (const auto& step : mSteps) {
        if (!pkt) return nullptr;
        pkt = step->translate(std::move(pkt), TranslateDirection::Serverbound);
    }
    return pkt;
}

std::unique_ptr<IPacket>
TranslationPipeline::translateClientbound(std::unique_ptr<IPacket> pkt) const {
    // Clientbound: apply steps in REVERSE order.
    for (auto it = mSteps.rbegin(); it != mSteps.rend(); ++it) {
        if (!pkt) return nullptr;
        pkt = (*it)->translate(std::move(pkt), TranslateDirection::Clientbound);
    }
    return pkt;
}
