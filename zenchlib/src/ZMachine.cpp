/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <algorithm>
#include <iterator>

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
        if (not this->_load_remaining(story_file)) {
            return; // failed to load rest of the story
        }
        this->_setup_accessors();
        this->_pc = this->_load_word(0x06); // load initial program counter
        this->_call_stack.emplace_back(); // setup dummy stack frame
        this->_state_valid = true; // this VM is ready to go
    }

    ZMachine::operator bool() {
        return this->_state_valid;
    }

    bool ZMachine::is_running() {
        return true;
    }

    void ZMachine::execute() {
        return;
    }

    const std::unordered_set<char> ZMachine::SUPPORTED_VERSIONS = {0x03,};

    ZMachine::Word ZMachine::_load_word(WordAddress address) {
        return (this->_memory[address] << 8) + this->_memory[address + 1];
    }

    bool ZMachine::_load_header(std::istream& story_file) {
        this->_memory.resize(64); // pre-allocate enough for header
        for (auto& byte : this->_memory) {
            auto next = story_file.get();
            if (not story_file) { // handle EOF/failbit
                return false;
            }
            byte = next;
        }
        // work out the memory map
        this->_static_memory_begin = this->_load_word(0x0e);
        // validate size of dynamic memory (must be at least 64 bytes)
        if (this->_static_memory_begin < 64) { return false; }
        // skip _static_memory_end for now --we won't know it until we've read the rest of the file
        this->_high_memory_begin = this->_load_word(0x04);
        // bottom of high memory must not overlap top of dynamic memory
        if (this->_high_memory_begin < this->_static_memory_begin) {
            return false; // validate
        }
        return true;
    }

    bool ZMachine::_load_remaining(std::istream& story_file) {
        // pre-allocate to the first byte of high memory (we don't know how much else there is)
        this->_memory.reserve(this->_static_memory_begin);
        // read in the remainder of the memory in the storyfile
        for (auto it = std::istreambuf_iterator<char>(story_file); it != std::istreambuf_iterator<char>(); it++) {
            this->_memory.push_back((Byte)*it);
        }
        // verify length of storyfile
        if (this->_memory.size() > 128 * 1024) { // XXX: Version 1-3: 128KiB
            return false;
        }
        // we can now work out where the end of static memory is
        this->_static_memory_end = std::clamp((BigAddress)(this->_memory.size() - 1), BigAddress{0x0}, BigAddress{0x0ffff});
        return true;
    }

    void ZMachine::_setup_accessors() {
        this->_writeable_memory = std::span<Byte>{this->_memory}.subspan(0, this->_static_memory_begin);
        this->_readable_memory = std::span<Byte>{this->_memory}.subspan(0, this->_static_memory_end - 1);
    }
}
