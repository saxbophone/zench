/**
 * @file
 * This is a sample public compilation unit.
 *
 * @author Your Name <your.email.address@goes.here>
 * @date Creation/Edit Date
 *
 * @copyright Copyright information goes here
 *
 * @copyright
 * Copyright information can span multiple paragraphs if needed, such as if you
 * use a well-known software license for which license header text (to be
 * placed in locations like these) are provided by the license custodians.
 */

#ifndef COM_SAXBOPHONE_ZENCH_ZENCH_HPP
#define COM_SAXBOPHONE_ZENCH_ZENCH_HPP

#include <cstdint>   // fixed-width types

#include <exception> // exception
#include <string>    // string

namespace com::saxbophone::zench {
    using Byte = std::uint8_t;
    using Word = std::uint16_t;
    using SWord = std::int16_t; // signed Word
    // these addresses can address any byte in memory. Range depends on version, but never bigger than 19-bit.
    using Address = std::uint32_t;
    using ByteAddress = std::uint16_t; // address to a Byte anywhere in dynamic or static memory
    using WordAddress = std::uint16_t; // address/2 of a Word anywhere in the bottom 128KiB of all memory
    using PackedAddress = std::uint16_t; // packed address for routines and strings
    // Z-machine string stuff
    using ZChar = Byte;

    // base class for all of zench's exceptions
    class Exception : public std::exception {};
    class CantReadStoryFileException : public Exception {
        const char* what() const noexcept {
            return "Can't read story file";
        }
    };
    class UnsupportedVersionException : public Exception {
        // TODO: rewrite this out-of-header to report supported versions
        // and actual version given
        const char* what() const noexcept {
            return "Story file version not supported";
        }
    };
    class InvalidStoryFileException : public Exception {
        const char* what() const noexcept {
            return "Invalid story file";
        }
    };
    class UnimplementedInstructionException : public Exception {
        const char* what() const noexcept {
            return "Unimplemented instruction encountered";
        }
    };
    class WrongNumberOfInstructionOperandsException : public Exception {
        const char* what() const noexcept {
            return "Wrong number of operands given to instruction";
        }
    };

    extern const std::string VERSION;
    extern const std::string VERSION_DESCRIPTION;
}

#endif // include guard
