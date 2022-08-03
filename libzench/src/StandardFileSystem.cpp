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

#include <zench/StandardFileSystem.hpp>

namespace com::saxbophone::zench {
    StandardFileSystem::StandardFileSystem(StandardFilePicker& picker) {}
    std::unique_ptr<FileSystem::InputFile> StandardFileSystem::open_for_read() { return {}; }
    std::unique_ptr<FileSystem::InputFile> StandardFileSystem::open_for_read(std::string filename) { return {}; }
    std::unique_ptr<FileSystem::OutputFile> StandardFileSystem::open_for_write() { return {}; }
    std::unique_ptr<FileSystem::OutputFile> StandardFileSystem::open_for_write(std::string filename) { return {}; }
}
