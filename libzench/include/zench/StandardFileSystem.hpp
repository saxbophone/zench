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

#ifndef COM_SAXBOPHONE_ZENCH_STANDARD_FILESYSTEM_HPP
#define COM_SAXBOPHONE_ZENCH_STANDARD_FILESYSTEM_HPP

#include <string>

#include <zench/Component.hpp>
#include <zench/FileSystem.hpp>

namespace com::saxbophone::zench {
    // abstracts out the method of prompting for filenames
    // i.e. one implementation could ask for a filename on the console, another
    // could spawn a file-picker dialog...
    class StandardFilePicker : public Component {
    public:
        virtual std::string get_filename() = 0;
    };
    // an implementation of FileSystem that just passes through to the stdlib
    class StandardFileSystem : public FileSystem {
    public:
        StandardFileSystem(StandardFilePicker& picker);
        constexpr const char* name() override {
            return "StandardFileSystem";
        }
        std::optional<InputFile> open_for_read() override;
        std::optional<InputFile> open_for_read(std::string filename) override;
        std::optional<OutputFile> open_for_write() override;
        std::optional<OutputFile> open_for_write(std::string filename) override;
    };
}

#endif // include guard
