/*
 * This is a sample source file corresponding to a private header file.
 *
 * <Copyright information goes here>
 */

#include <cstddef>
#include <cstdint>

#include <array>
#include <map>
#include <utility>
#include <vector>

#include "ZStringDecoder.hpp"

namespace {
    using namespace com::saxbophone::zench;

    using ZChar = Byte;

    enum class Alphabet : int { A0 = 0, A1 = 1, A2 = 2, };

    std::map<Alphabet, std::string> ALPHABET_TABLE = {
        {Alphabet::A0, "abcdefghijklmnopqrstuvwxyz"},
        {Alphabet::A1, "ABCDEFGHIJKLMNOPQRSTUVWXYZ"},
        {Alphabet::A2, " \n0123456789.,!?_#'\"/\\-:()"},
    };

    // unpacks a word of two bytes into three Z-Chars
    std::array<ZChar, 3> decompose_pair(std::pair<const Byte, const Byte> pair) {
        std::array<ZChar, 3> triple;
        std::uint16_t code = ((std::uint16_t)pair.first << 8) | pair.second;
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

    std::string fetch_abbreviation(ZChar z, ZChar x) {
        std::string output;
        std::size_t abbreviation = (32 * ((std::size_t)z - 1)) + x;
        // TODO: actually fetch the abbreviation here
        output += "@{";
        output += std::to_string(abbreviation); // XXX: just prints abbrev number
        output += "}";
        return output;
    }

    std::string fetch_escape(ZChar top, ZChar bottom) {
        std::string output;
        std::uint16_t code = ((std::uint16_t)top << 5) | bottom;
        // XXX: just output the escape code as an escape sequence for now
        output += "\\z";
        output += std::to_string(code);
        return output;
    }

    std::string zscii_decode(const std::vector<ZChar>& z_chars) {
        std::string output;
        Alphabet current_alphabet = Alphabet::A0;
        for (std::size_t i = 0; i < z_chars.size(); i++) {
            auto z = z_chars[i];
            switch (z) {
            case 1: case 2: case 3: {
                // Abbreviation (currently unhandled)
                output += fetch_abbreviation(z, z_chars[i + 1]);
                i++; // skip next char (would indicate which abbrev. to use)
                break;
            }
            case 4:
                current_alphabet = Alphabet::A1; // shift for next character only
                break;
            case 5:
                current_alphabet = Alphabet::A2; // shift for next character only
                break;
            default:
                // print corresponding ZSCII character from the current alphabet
                if (z == 0) {
                    output += " "; // Z-char 0 is a space
                } else if (current_alphabet == Alphabet::A2 and z == 6) {
                    // ZSCII escape code --not implemented right now
                    output += fetch_escape(z_chars[i + 1], z_chars[i + 2]);
                    i += 2; // skip next two chars
                } else {
                    output += ALPHABET_TABLE[current_alphabet][z - 6];
                }
                // reset alphabet in case it was shifted previously
                current_alphabet = Alphabet::A0;
                break;
            }
        }
        return output;
    }
}

namespace com::saxbophone::zench {
    // ctor only needs to take a few details about version, any custom decoder tables...
    ZStringDecoder::ZStringDecoder(
        ZVersion version,
        std::span<const Byte> abbreviations_table,
        std::optional<std::span<Byte, 78>> alphabet_table,
        std::optional<std::span<char16_t>> unicode_translation_table
    ) {
        // TODO: validate version number here! We only support V3 right now!
    }

    std::string ZStringDecoder::decode(std::span<const Byte> z_string) const {
        return _decode(z_string, false);
    }

    std::string ZStringDecoder::_decode(
        std::span<const Byte> z_string,
        bool abbreviations_allowed
    ) const {
        auto z_chars = decompose(z_string); // non fully decoded z-chars
        return zscii_decode(z_chars);
    }
}
