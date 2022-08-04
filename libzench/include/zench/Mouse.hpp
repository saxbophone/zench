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

#ifndef COM_SAXBOPHONE_ZENCH_MOUSE_HPP
#define COM_SAXBOPHONE_ZENCH_MOUSE_HPP

#include <zench/Component.hpp>

namespace com::saxbophone::zench {
    class Mouse : public Component {
        // This is meant to be abstract
        /*
         * WARN: Mouse click events are actually sent through the Keyboard!
         * Just one of many Z-machine quirks.
         * We keep to that convention here because otherwise, if we handle
         * click events through this Mouse interface, we would lose the ordering
         * between the keyboard key events and the mouse click events.
         *
         * This class is for polling the mouse state.
         *
         * things to add here:
         * - button states?
         * - mouse position?
         */
    };
}

#endif // include guard
