/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

#include "ZMachineImpl.hpp"

namespace com::saxbophone::zench {
    ZMachine::ZMachine(
        InputFile& game_file,
        FileSystem& fs,
        Screen& screen,
        Keyboard& keyboard
    ) : _impl(new ZMachineImpl(game_file, fs, screen, keyboard)) {}

    ZMachine::~ZMachine() = default; // needed to allow pimpl idiom to work
    // see: https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/
}
