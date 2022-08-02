/*
 * This file forms part of libzench
 * libzench is a software library that implements a portable and extensible
 * Z-machine interpreter, designed to be embedded within other programs.
 *
 * Created by Joshua Saxby <joshua.a.saxby@gmail.com>, May 2022
 *
 * Copyright Joshua Saxby <joshua.a.saxby@gmail.com> 2022
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
            InputFile& story_file,
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
