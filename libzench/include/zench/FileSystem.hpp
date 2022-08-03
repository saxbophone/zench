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
        /**
         * @brief Top-level abstract base class for abstract file objects
         * @note This interface is not intended to be inherited from directly
         * for implementing InputFile/OutputFile.
         * @details A File is a handle to a file, which can be in the open or
         * closed state at any given time. Implementors should write their
         * constructors to produce objects which start off in the open state if
         * possible, and which can be closed explicitly with a call to .close().
         * @note Implementors of this interface, or interfaces derived from it,
         * must make sure that their destructors call .close() on the File
         * object if it is still open, approximately equivalent to this code:
         * @code{.cpp}
         * ~File() {
         *     if (this->is_open()) {
         *         this->close();
         *     }
         * }
         * @endcode
         */
        class File : public Component {
        public:
            /**
             * @returns whether this File object is currently open or closed
             */
            virtual bool is_open() = 0;
            /**
             * @brief Closes this File object. The object remembers which file
             * it is a handle to even after being closed.
             * @pre `File.is_open() == true`
             * @post `File.is_open() == false`
             */
            virtual void close() = 0;
            /**
             * @brief Attempts to re-open a closed File.
             * @returns whether the File was successfully re-opened or not
             * @pre `File.is_open() == false`
             * @note Implementors are not required to support re-opening of files
             * and if they do not support it, should always return `false`.
             * If however an implementor does support this feature, then they
             * must return success/failure as appropriate, and leave the File
             * object in a valid state according to whether it was opened
             * successfully, or couldn't be opened (in which case, the File
             * remains closed).
             */
            virtual bool open() = 0;
        };
        /**
         * @brief An abstract file from which bytes can be read
         * @see File for important remarks about making sure that destructors
         * of classes implementing this one ensuring that the file is closed
         * before it is destroyed.
         */
        class InputFile : public File {
        public:
            virtual char read() = 0; // EOF?
        };
        /**
         * @brief An abstract file to which bytes can be written
         * @see File for important remarks about making sure that destructors
         * of classes implementing this one ensuring that the file is closed
         * before it is destroyed.
         */
        class OutputFile : public File {};
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
