/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <vector>

#include <zench/ZStringDecoder.hpp>

namespace {
    using namespace com::saxbophone::zench;

    using ZChar = Byte;

    // unpacks bytes of z-string into sequence of 5-bit Z-Chars
    std::vector<const ZChar> decompose(std::span<const Byte> z_string) {
        return {};
    }
}

namespace com::saxbophone::zench {
    // ctor only needs to take a few details about version, any custom decoder tables...
    ZStringDecoder::ZStringDecoder(
        ZVersion version,
        std::span<const Byte> abbreviations_table,
        std::optional<std::span<Byte, 78>> alphabet_table,
        std::optional<std::span<char16_t>> unicode_translation_table
    ) {}

    std::string ZStringDecoder::decode(std::span<const Byte> z_string) const {
        return _decode(z_string, false);
    }

    std::string ZStringDecoder::_decode(
        std::span<const Byte> z_string,
        bool abbreviations_allowed
    ) const {
        return "<NOT DECODED>";
    }
}
