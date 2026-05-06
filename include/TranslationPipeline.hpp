#pragma once

// TranslationPipeline.hpp
// Builds and caches the ordered chain of IVersionTranslator steps
// needed to convert between a specific client protocol and the server protocol.
//
// Serverbound path example for client 1.21.50 (proto 594):
//   [1.21.50 -> 1.21.130] -> [1.21.130 -> 1.26.0] -> [1.26.0 -> 1.26.10]
//
// Clientbound path is the *reverse* order with Direction::Clientbound.

#include "IVersionTranslator.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <vector>

class TranslationPipeline {
public:
    // Build the pipeline for the given clientProtocol.
    // Translators are registered globally via TranslatorRegistry (see below).
    // Throws std::runtime_error if no chain can be built.
    explicit TranslationPipeline(std::int32_t clientProtocol);

    // Translate a packet arriving from the client (Serverbound direction).
    // Returns nullptr if the packet should be dropped.
    [[nodiscard]] std::unique_ptr<IPacket> translateServerbound(std::unique_ptr<IPacket> pkt) const;

    // Translate a packet going to the client (Clientbound direction).
    // Returns nullptr if the packet should be dropped.
    [[nodiscard]] std::unique_ptr<IPacket> translateClientbound(std::unique_ptr<IPacket> pkt) const;

    [[nodiscard]] std::int32_t clientProtocol() const noexcept { return mClientProtocol; }

private:
    std::int32_t                                       mClientProtocol{};
    // Ordered low-to-high (Serverbound direction).
    std::vector<std::shared_ptr<IVersionTranslator>>   mSteps;
};

// ── TranslatorRegistry ───────────────────────────────────────────────────────
// A simple global registry so each version-hop module can self-register.
// Use TRANSLATOR_AUTO_REGISTER macro in each translator's .cpp to hook in.

class TranslatorRegistry {
public:
    static TranslatorRegistry& get() noexcept {
        static TranslatorRegistry instance;
        return instance;
    }

    void registerTranslator(std::shared_ptr<IVersionTranslator> t) {
        mTranslators.push_back(std::move(t));
    }

    // Returns all translators ordered from lowest fromVersion to highest.
    [[nodiscard]] const std::vector<std::shared_ptr<IVersionTranslator>>& all() const noexcept {
        return mTranslators;
    }

private:
    TranslatorRegistry()  = default;
    std::vector<std::shared_ptr<IVersionTranslator>> mTranslators;
};

// Convenience macro — place at the top of each translator .cpp file.
// Example:
//   TRANSLATOR_AUTO_REGISTER(Translator_1_21_130_to_1_26_0)
#define TRANSLATOR_AUTO_REGISTER(ClassName)                              \
    namespace {                                                          \
    struct _AutoReg_##ClassName {                                        \
        _AutoReg_##ClassName() {                                         \
            TranslatorRegistry::get().registerTranslator(                \
                std::make_shared<ClassName>()                            \
            );                                                           \
        }                                                                \
    } _autoReg_##ClassName##_instance;                                   \
    }
