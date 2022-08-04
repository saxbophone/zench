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

#ifndef COM_SAXBOPHONE_ZENCH_SCREEN_HPP
#define COM_SAXBOPHONE_ZENCH_SCREEN_HPP

#include <cstdint> // uint8_t

#include <utility> // pair

#include <zench/Component.hpp>

namespace com::saxbophone::zench {
    class Screen : public Component {
        // This is meant to be abstract
        /*
         * This is meant to be like a low-level device driver, so we
         * shouldn't make implementors worry about Z-machine concepts
         * such as upper/lower window or numbered windows (when v6)
         * as these vary across Z-machine versions anyway and will
         * needlessly complicate the driver beyond what it needs to do.
         * Let the Z-machine itself handle those concepts, including
         * remembering where lines are for scrolling behaviour, etc...
         *
         * things to add here:
         * - navigating the invisible cursor to a specific position
         * - retrieving the screen dimensions
         * - writing text starting at the cursor position, with styles
         * - erase text from the cursor position to a specified end pos
         * - erase an entire line
         * - erase the whole screen
         * - erase a single character
         * - move a range of text from cursor to end pos, by a delta
         * - get mouse position?
         * - set the visible cursor position
         * - change visible cursor style/blink settings/visibility
         * - get resize events?
         */
        virtual std::pair<std::uint8_t, std::uint8_t> get_dimensions() = 0;
        virtual bool supports_colour() = 0;
        virtual bool supports_truecolour() = 0;
    };
}

#endif // include guard
