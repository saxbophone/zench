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

#include <cstdint>

#include <variant>
#include <vector>

#include <zench/Component.hpp>

namespace com::saxbophone::zench {
    /**
     * @brief This is the lowest-level keyboard driver for use with Zench.
     * @details Implementors of a keyboard to interface with Zench should
     * inherit from this class and override all prototyped methods it specifies,
     * with appropriate behaviour.
     * @note This Keyboard interface is non-blocking from the point of view of
     * the Z-machine, i.e. you should send all applicable keyboard input from
     * your application into this interface at all times, without worrying about
     * whether the Z-machine is waiting for input or not --this buffering/blocking
     * behaviour is provided by Zench itself.
     * This is important as it allows Zench to echo input to the Screen as it is
     * being typed, without having to wait for the results of an executed READ
     * opcode. Zench will not start echoing keyboard input to the screen until
     * a READ opcode has started executing, but this is beyond scope of a Keyboard
     * interface to worry about in any case.
     */
    class Keyboard : public Component {
    public:
        // these are keycodes that may not be well supported by Unicode, but which Z-machine requires input for
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
            // mouse input codes (yes, some of the mouse input is handled via the "keyboard"!)
            // NOTE: passive polling of the mouse state is handled elsewhere, in the Mouse class/interface
            MenuClick, DoubleClick, SingleClick,
        };
        /*
         * Keyboard Events are either Unicode codepoints for the character that
         * was typed, or a SpecialKey value for those keys unlikely to be
         * capturable by text input.
         * NOTE: We use uint16_t for Unicode codepoints, because the Z-Machine
         * is restricted to only handling characters from the Basic Multilingual
         * Plane (first 65,536 codepoints). Even then, it's only allowed to
         * handle a specified subset of them at once (according to the Unicode
         * translation table), but it's not Keyboard's job to translate these.
         * WARN: Keyboard codepoints are literal Unicode codepoint values, these
         * are NOT UTF-16 characters! A valid method to produce these codepoints
         * is to take your text input as UTF-32, discard characters greater than
         * 0xFFFF and cast the remaining ones to uint16_t.
         */
        using Event = std::variant<std::uint16_t, SpecialKey>;
        /**
         * @returns whether this Keyboard driver supports mouse input or not
         */
        constexpr virtual bool supports_mouse() = 0;
        /**
         * @returns whether this Keyboard driver supports menus or not
         */
        constexpr virtual bool supports_menus() = 0;
        /**
         * @brief Get any keyboard input events that were triggered since this
         * method was last called
         * @warn This method must not block, returning an empty sequence if
         * there were not any input events, rather than wait for input events.
         * @returns a sequence of input events, in the order that they were
         * emitted.
         * @post The underlying container holding the input events is empty of
         * all the events returned by this method.
         */
        virtual std::vector<Event> get_input() = 0;
    };
}

#endif // include guard
