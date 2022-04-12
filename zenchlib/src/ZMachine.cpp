/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zench/ZMachine.hpp>

namespace com::saxbophone::zench {
    ZMachine::ZMachine(std::istream& story_file) {}

    ZMachine::operator bool() {
        return false;
    }

    bool ZMachine::is_running() {
        return true;
    }

    void ZMachine::execute() {
        return;
    }
}
