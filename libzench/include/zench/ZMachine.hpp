/**
 * @file
 * This is a sample public compilation unit.
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

#ifndef COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP
#define COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP

#include <memory> // unique_ptr

namespace com::saxbophone::zench {
    // forward-declarations, for now. All of these should be polymorphic to allow client programs to customise them.
    // TODO: work out where the boundary between common code in the ZMachine itself vs inside these parent classes lies.
    class FileSystem; // abstracted interface for filesystem, allows ZMachine to be agnostic of how files are handled
    class InputFile;
    class OutputFile;
    class Screen;
    class Keyboard;

    class ZMachine {
    public:
        // NOTE: although the ZMachine *could* elect to keep a preserved copy of the game file's original contents, we
        // don't need to keep the file handle itself hanging around. Keeping it limited in this way allows great
        // flexibility, such as loading a game file from the network, for instance.
        // In practice, that might not be that useful, as one would not be able to play the game again without having
        // access to that same URL. On the other hand, this permits the possibility of transparent, savefile-compatible
        // updates (i.e. a game distributor could make the latest version of their game available at a URL, which could
        // transparently offer new bug release versions of the game as it is updated, and an interpreter would be none
        // the wiser...)
        ZMachine(InputFile& game_file);
    private:
        FileSystem& _filesystem;
        // output streams:
        Screen& _screen;
        std::unique_ptr<OutputFile> _transcript;
        std::unique_ptr<OutputFile> _commands_script;
        // input streams:
        Keyboard& _keyboard;
        std::unique_ptr<InputFile> _file_with_commands;
        // TODO: add pimpl pointer
    };
}

#endif // include guard
