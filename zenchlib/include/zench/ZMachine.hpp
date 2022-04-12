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
 *
 */

#ifndef COM_SAXBOPHONE_ZENCH_PUBLIC_HPP
#define COM_SAXBOPHONE_ZENCH_PUBLIC_HPP

#include <cstdint>
#include <istream>

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        using Byte = std::uint8_t;
        using Word = std::uint16_t;

        ZMachine(std::istream& story_file);

        explicit operator bool(); // returns true if the ZMachine is runnable

        bool is_running();

        void execute();
    };
}

#endif // include guard
