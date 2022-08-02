/**
 * @file
 * @brief This file forms part of libzench
 * @details libzench is a software library that implements a portable and
 * extensible Z-machine interpreter, designed to be embedded within other
 * programs.
 *
 * @author Joshua Saxby <joshua.a.saxby@gmail.com>
 * @date April 2022
 *
 * @copyright Copyright Joshua Saxby <joshua.a.saxby@gmail.com> 2022
 *
 * @copyright
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
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
