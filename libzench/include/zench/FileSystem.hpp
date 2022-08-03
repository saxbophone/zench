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

#ifndef COM_SAXBOPHONE_ZENCH_FILESYSTEM_HPP
#define COM_SAXBOPHONE_ZENCH_FILESYSTEM_HPP

#include <memory> // unique_ptr
#include <string> // string

#include <zench/Component.hpp>

namespace com::saxbophone::zench {
    /**
     * @brief Abstracted interface for filesystem access
     * @details Allows ZMachine to be agnostic of how files are handled
     */
    class FileSystem : public Component {
    public:
        class InputFile : public Component {};
        class OutputFile : public Component {};
        // all the following methods return nullptr when a file could not be retrieved.
        // tries to open a file for reading from. Where the file comes from, is the responsibility of the FileSystem
        // object to determine (such as prompting the user, or opening up a file-picker).
        virtual std::unique_ptr<InputFile> open_for_read() = 0;
        // tries to open a file with the given name, for reading from.
        // Filenames have the following format (approximately the MS-DOS 8.3 rule):
        // one to eight alphanumeric characters, a full stop and zero to three alphanumeric characters for extension.
        // NOTE: pedantically, we use UTF-16 characters as the spec doesn't specify they have to be ASCII. They probably
        // are in practice, though. We use null-terminated strings, hence the 13-character maximum.
        // TODO: what about the optional "prompt?" flag?
        virtual std::unique_ptr<InputFile> open_for_read(std::string filename) = 0;
        // tries to open a file for writing to. Where the file comes from, is the responsibility of the FileSystem
        // object to determine (such as prompting the user, or opening up a file-picker).
        virtual std::unique_ptr<OutputFile> open_for_write() = 0;
        // tries to open a file with the given name, for writing to.
        // TODO: what about the optional "prompt?" flag?
        virtual std::unique_ptr<OutputFile> open_for_write(std::string filename) = 0;
    };
}

#endif // include guard
