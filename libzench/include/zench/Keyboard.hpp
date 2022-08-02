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

#ifndef COM_SAXBOPHONE_ZENCH_KEYBOARD_HPP
#define COM_SAXBOPHONE_ZENCH_KEYBOARD_HPP

#include <variant>
#include <vector>

namespace com::saxbophone::zench {
    class Keyboard {
    public:
        // these are keycodes that may not be well supported by UTF-16, but which Z-machine requires input for
        enum class SpecialKey {
            Delete,
            Newline, // NOTE: maybe unused, but provided for implementors who don't catch all LF/CR in their text input
            Escape,
            // arrow/cursor keys
            Up, Down, Left, Right,
            // function keys
            F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
            // numpad keys --maybe unused? Or perhaps these are to allow reading numpad separately from the numeric keys
            N0, N1, N2, N3, N4, N5, N6, N7, N8, N9,
        };
        // NOTE: normal text input should be sent as sequences of UTF-16 codepoints. The Z-machine converts them to ZSCII.

        /**
         * @brief Get any keyboard input events that were triggered since this
         * method was last called
         * @warn This method must not block, returning an empty sequence if
         * there were not any input events, rather than wait for input events.
         * @returns a sequence of input events, in the order that they were
         * emitted.
         * @post The underlying container holding the input events is empty
         */
        virtual std::vector<std::variant<char16_t, SpecialKey>> get_input() = 0;
    };
}

#endif // include guard
