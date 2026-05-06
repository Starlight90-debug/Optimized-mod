#pragma once

// IVersionTranslator.hpp
// Abstract interface for a single-step version translator.
// Each concrete implementation translates ONE hop, e.g.:
//   Serverbound:  1.21.130 -> 1.26.0
//   Clientbound:  1.26.0   -> 1.21.130
//
// The pipeline chains multiple translators to bridge any gap.

#include <memory>
#include <sculk/protocol/codec/packet/IPacket.hpp>

using namespace sculk::protocol; // abi_v975 is inline, so this works

// Direction of translation relative to the server.
enum class TranslateDirection : std::uint8_t {
    Serverbound, // client  ->  server  (translate UP)
    Clientbound, // server  ->  client  (translate DOWN)
};

class IVersionTranslator {
public:
    IVersionTranslator()                                       = default;
    IVersionTranslator(const IVersionTranslator&)              = delete;
    IVersionTranslator& operator=(const IVersionTranslator&)   = delete;
    virtual ~IVersionTranslator() noexcept                     = default;

    // Protocol version this translator accepts as *input*.
    // For Serverbound: the older client protocol.
    // For Clientbound: the newer server protocol.
    [[nodiscard]] virtual std::int32_t fromVersion() const noexcept = 0;

    // Protocol version this translator produces as *output*.
    [[nodiscard]] virtual std::int32_t toVersion() const noexcept   = 0;

    // Translate the packet.
    // Returns nullptr if the packet should be silently dropped (no equivalent).
    // Returns the original `packet` (possibly mutated) if no structural change is needed.
    // Returns a newly allocated packet if the structure changes.
    //
    // Ownership: the caller owns both input and output.
    [[nodiscard]] virtual std::unique_ptr<IPacket> translate(
        std::unique_ptr<IPacket> packet,
        TranslateDirection       direction
    ) = 0;
};
