/*
 * This is a sample source file corresponding to a private header file.
 *
 * <Copyright information goes here>
 */

#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

#include "ZMachineImpl.hpp"

namespace com::saxbophone::zench {
    ZMachine::ZMachineImpl::ZMachineImpl(
        InputFile& game_file,
        FileSystem& fs,
        Screen& screen,
        Keyboard& keyboard
    )
      : _filesystem(fs)
      , _screen(screen)
      , _keyboard(keyboard)
      {}
}
