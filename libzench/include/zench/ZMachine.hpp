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

#ifndef COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP
#define COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP

#include <cstddef>   // size_t

#include <bitset>    // bitset
#include <deque>     // deque
#include <istream>   // istream
#include <memory>    // unique_ptr

#include <zench/zench.hpp>

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        // NOTE: it is permitted for story_file to be closed for further reading after this constructor returns
        ZMachine(std::istream& story_file);
        ~ZMachine();

        bool is_running(); // returns true if a runnable machine has not quit

        void execute(); // executes one instruction
                                                            // v87654321
        static constexpr std::bitset<8> SUPPORTED_VERSIONS = {0b00000100};

    private:
        class ZMachineImpl;

        static constexpr std::size_t HEADER_SIZE = 64;
        static constexpr std::size_t STORY_FILE_MAX_SIZE = 128 * 1024; // Version 1-3: 128KiB

        std::unique_ptr<ZMachineImpl> _impl;
    };
}

#endif // include guard
