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

#ifndef COM_SAXBOPHONE_ZENCH_COMPONENT_HPP
#define COM_SAXBOPHONE_ZENCH_COMPONENT_HPP

namespace com::saxbophone::zench {
    /**
     * @brief A Component is one of the swappable interfaces of a Z-Machine
     * @details These include FileSystem, Keyboard, Screen, etc...
     */
    class Component {
    public:
        /**
         * @returns the name of this Zench component.
         * @details Use this to give your implementations of Screen, Keyboard,
         * etc. a unique, descriptive identifier. It will be used in logs and
         * error messages.
         */
        constexpr virtual const char* name() = 0;
    };
}

#endif // include guard
