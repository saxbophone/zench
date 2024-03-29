/**
 * This is a sample private header file.
 *
 * <Copyright information goes here>
 */

#ifndef COM_SAXBOPHONE_ZENCH_Z_STRING_DECODER_HPP
#define COM_SAXBOPHONE_ZENCH_Z_STRING_DECODER_HPP

#include <optional>        // optional
#include <span>            // span
#include <string>          // string

#include <zench/zench.hpp> // base library definitions of core types

namespace com::saxbophone::zench {
    class ZStringDecoder {
    public:
        // ctor only needs to take a few details about version, any custom decoder tables...
        ZStringDecoder(
            ZVersion version,
            std::span<const Byte> abbreviations_table={},
            std::optional<std::span<Byte, 78>> alphabet_table=std::nullopt,
            std::optional<std::span<char16_t>> unicode_translation_table=std::nullopt
        );
        /*
         * NOTE: we return a UTF8-encoded string implicitly
         * the Unicode lookup table is actually encoded in UTF16 but we'll
         * convert that on an as-needs basis rather than work solely in UTF16
         * to accomodate it...
         */
        std::string decode(std::span<const Byte> z_string) const;
    private:
        std::string _decode(
            std::span<const Byte> z_string,
            bool abbreviations_allowed=true
        ) const;
    };
}

#endif // include guard
