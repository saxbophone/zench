/**
 * @file
 * This is a sample public header file.
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

#include <memory>   // unique_ptr

#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        ZMachine(
            InputFile& game_file,
            FileSystem& fs,
            Screen& screen,
            Keyboard& keyboard
        );
        ~ZMachine();
    private:
        class ZMachineImpl;
        // pimpl pointer
        std::unique_ptr<ZMachineImpl> _impl;
    };
}

#endif // include guard
