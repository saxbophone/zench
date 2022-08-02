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
    ZMachine::ZMachineImpl::ZMachineImpl(
        InputFile& game_file,
        FileSystem& fs,
        Screen& screen,
        Keyboard& keyboard
    )
      : _filesystem(fs)
      , _screen(screen)
      , _keyboard(keyboard)
      {}
}
