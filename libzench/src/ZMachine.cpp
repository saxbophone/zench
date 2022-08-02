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

#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

#include "ZMachineImpl.hpp"

namespace com::saxbophone::zench {
    ZMachine::ZMachine(
        InputFile& story_file,
        FileSystem& fs,
        Screen& screen,
        Keyboard& keyboard
    ) : _impl(new ZMachineImpl(story_file, fs, screen, keyboard)) {}

    ZMachine::~ZMachine() = default; // needed to allow pimpl idiom to work
    // see: https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/
    // returns true if ZMachine instance is ready to execute an instruction
    bool ZMachine::is_ready() { return false; } // STUB
    // executes one instruction
    void ZMachine::execute() { return; } // STUB
}
