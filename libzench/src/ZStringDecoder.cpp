/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <cstddef>
#include <cstdint>

#include <array>
#include <utility>
#include <vector>

#include <zench/ZStringDecoder.hpp>

namespace {
    using namespace com::saxbophone::zench;

    using ZChar = Byte;

    // unpacks a word of two bytes into three Z-Chars
    std::array<ZChar, 3> decompose_pair(std::pair<const Byte, const Byte> pair) {
        std::array<ZChar, 3> triple;
        std::uint16_t code = ((std::uint16_t)pair.first << 8) + pair.second;
        for (std::size_t i = 3; i --> 0;) {
            triple[i] = (Byte)(code & 0b11111);
            code >>= 5;
        }
        return triple;
    }

    // unpacks bytes of z-string into sequence of 5-bit Z-Chars
    std::vector<ZChar> decompose(std::span<const Byte> z_string) {
        // only decompose every complete pair of bytes (IIRC, the standard says we're allowed to omit partials)
        // TODO: confirm we're allowed to discard partials
        std::vector<ZChar> zchars;
        for (std::size_t i = 0; i < z_string.size(); i += 2) {
            auto decomposition = decompose_pair({z_string[i], z_string[i + 1]});
            zchars.insert(zchars.end(), decomposition.begin(), decomposition.end());
        }
        return zchars;
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
        // return "<NOT DECODED>";
        std::string output;
        for (auto c : decompose(z_string)) {
            output += std::to_string(c);
            output += "•";
        }
        return output;
    }
}
