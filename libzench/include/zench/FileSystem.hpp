/**
 * @file
 * This is a sample public header file.
 *
 * @author Your Name <your.email.address@goes.here>
 * @date Creation/Edit Date
 *
 * @copyright Copyright information goes here
 *
 * @copyright
 * Copyright information can span multiple paragraphs if needed, such as if you
 * use a well-known software license for which license header text (to be
 * placed in locations like these) are provided by the license custodians.
 */

#ifndef COM_SAXBOPHONE_ZENCH_FILESYSTEM_HPP
#define COM_SAXBOPHONE_ZENCH_FILESYSTEM_HPP

#include <optional> // optional
#include <string>   // string

namespace com::saxbophone::zench {
    class InputFile {};
    class OutputFile {};
    // abstracted interface for filesystem, allows ZMachine to be agnostic of how files are handled
    class FileSystem {
    public:
        // all the following methods return std::nullopt when a file could not be retrieved.
        // tries to open a file for reading from. Where the file comes from, is the responsibility of the FileSystem
        // object to determine (such as prompting the user, or opening up a file-picker).
        virtual std::optional<InputFile> open_for_read() = 0;
        // tries to open a file with the given name, for reading from.
        // Filenames have the following format (approximately the MS-DOS 8.3 rule):
        // one to eight alphanumeric characters, a full stop and zero to three alphanumeric characters for extension.
        // NOTE: pedantically, we use UTF-16 characters as the spec doesn't specify they have to be ASCII. They probably
        // are in practice, though. We use null-terminated strings, hence the 13-character maximum.
        // TODO: what about the optional "prompt?" flag?
        virtual std::optional<InputFile> open_for_read(std::string filename) = 0;
        // tries to open a file for writing to. Where the file comes from, is the responsibility of the FileSystem
        // object to determine (such as prompting the user, or opening up a file-picker).
        virtual std::optional<OutputFile> open_for_write() = 0;
        // tries to open a file with the given name, for writing to.
        // TODO: what about the optional "prompt?" flag?
        virtual std::optional<OutputFile> open_for_write(std::string filename) = 0;
    };
}

#endif // include guard
