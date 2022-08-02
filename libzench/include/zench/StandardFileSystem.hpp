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

#ifndef COM_SAXBOPHONE_ZENCH_STANDARD_FILESYSTEM_HPP
#define COM_SAXBOPHONE_ZENCH_STANDARD_FILESYSTEM_HPP

#include <string>

#include <zench/FileSystem.hpp>

namespace com::saxbophone::zench {
    // abstracts out the method of prompting for filenames
    // i.e. one implementation could ask for a filename on the console, another
    // could spawn a file-picker dialog...
    class StandardFilePicker {
    public:
        virtual std::string get_filename() = 0;
    };
    // an implementation of FileSystem that just passes through to the stdlib
    class StandardFileSystem : public FileSystem {
    public:
        StandardFileSystem(StandardFilePicker& picker);
        std::optional<InputFile> open_for_read() override;
        std::optional<InputFile> open_for_read(std::string filename) override;
        std::optional<OutputFile> open_for_write() override;
        std::optional<OutputFile> open_for_write(std::string filename) override;
    };
}

#endif // include guard
