/**
 * @file
 * This is a sample private header.
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

#ifndef COM_SAXBOPHONE_ZENCH_ZMACHINE_IMPL_HPP
#define COM_SAXBOPHONE_ZENCH_ZMACHINE_IMPL_HPP

#include <memory>   // unique_ptr

#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

namespace com::saxbophone::zench {
    class ZMachine::ZMachineImpl {
    public:
        ZMachineImpl(
            InputFile& game_file,
            FileSystem& fs,
            Screen& screen,
            Keyboard& keyboard
        );
    private:
        FileSystem& _filesystem;
        // output streams:
        Screen& _screen;
        std::unique_ptr<OutputFile> _transcript;
        std::unique_ptr<OutputFile> _commands_script;
        // input streams:
        Keyboard& _keyboard;
        std::unique_ptr<InputFile> _file_with_commands;
    };
}

#endif // include guard
