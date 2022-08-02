/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zench/StandardFileSystem.hpp>

namespace com::saxbophone::zench {
    StandardFileSystem::StandardFileSystem(StandardFilePicker& picker) {}
    std::optional<InputFile> StandardFileSystem::open_for_read() { return {}; }
    std::optional<InputFile> StandardFileSystem::open_for_read(std::string filename) { return {}; }
    std::optional<OutputFile> StandardFileSystem::open_for_write() { return {}; }
    std::optional<OutputFile> StandardFileSystem::open_for_write(std::string filename) { return {}; }
}
