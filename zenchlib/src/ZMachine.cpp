/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zench/ZMachine.hpp>

namespace com::saxbophone::zench {
    ZMachine::ZMachine(std::istream& story_file) {
        // check stream can be read from
        if (not story_file.good()) { return; }
        // check file version
        char file_version = story_file.peek();
        if (not ZMachine::SUPPORTED_VERSIONS.contains(file_version)) {
            // TODO: log error, file version, and supported versions
            return;
        }
        // load story file
        if (not this->_load_header(story_file)) {
            return; // failed to load header
        }
    }

    ZMachine::operator bool() {
        return false;
    }

    bool ZMachine::is_running() {
        return true;
    }

    void ZMachine::execute() {
        return;
    }

    const std::unordered_set<char> ZMachine::SUPPORTED_VERSIONS = {0x03,};

    bool ZMachine::_load_header(std::istream& story_file) {
        this->_memory.resize(64); // pre-allocate enough for header
        for (auto& byte : this->_memory) {
            auto next = story_file.get();
            if (not story_file) { // handle EOF/failbit
                return false;
            }
            byte = next;
        }
        return true;
    }
}
