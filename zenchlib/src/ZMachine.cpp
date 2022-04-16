/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <algorithm>
#include <iostream> // XXX: debug
#include <iterator>

#include <zench/ZMachine.hpp>

#include "Instruction.hpp"

namespace com::saxbophone::zench {
    ZMachine::ZMachine(std::istream& story_file) {
        // check stream can be read from
        if (not story_file.good()) {
            throw CantReadStoryFileException();
        }
        // check file version
        Byte file_version = story_file.peek();
        if (0 < file_version and file_version < 9) {
            if (not SUPPORTED_VERSIONS.test(file_version - 1)) {
                // TODO: log error, file version, and supported versions
                throw UnsupportedVersionException();
            }
        } else {
            // invalid version byte (not a well-formed Quetzal file)
            throw InvalidStoryFileException();
        }
        // load story file
        this->_load_header(story_file);
        this->_load_remaining(story_file);
        this->_setup_accessors();
        this->_pc = this->_load_word(0x06); // load initial program counter
        this->_call_stack.emplace_back(); // setup dummy stack frame
        this->_state_valid = true; // this VM is ready to go
    }

    bool ZMachine::is_running() {
        return true;
    }

    void ZMachine::execute() {
        // XXX: debug
        this->_print_next_instruction();
    }

    Word ZMachine::_load_word(ByteAddress address) {
        return (this->_memory[address] << 8) + this->_memory[address + 1];
    }

    void ZMachine::_load_header(std::istream& story_file) {
        this->_memory.resize(ZMachine::HEADER_SIZE); // pre-allocate enough for header
        for (auto& byte : this->_memory) {
            auto next = story_file.get();
            if (not story_file) { // handle EOF/failbit
                throw InvalidStoryFileException(); // header ended prematurely
            }
            byte = next;
        }
        // work out the memory map
        this->_static_memory_begin = this->_load_word(0x0e);
        // validate size of dynamic memory (must be at least 64 bytes)
        if (this->_static_memory_begin < ZMachine::HEADER_SIZE) {
            throw InvalidStoryFileException();
        }
        // skip _static_memory_end for now --we won't know it until we've read the rest of the file
        this->_high_memory_begin = this->_load_word(0x04);
        // bottom of high memory must not overlap top of dynamic memory
        if (this->_high_memory_begin < this->_static_memory_begin) {
            throw InvalidStoryFileException();
        }
    }

    void ZMachine::_load_remaining(std::istream& story_file) {
        // pre-allocate to the maximum allowed storyfile size
        this->_memory.reserve(ZMachine::STORY_FILE_MAX_SIZE);
        // read in the remainder of the memory in the storyfile
        for (auto it = std::istreambuf_iterator<char>(story_file); it != std::istreambuf_iterator<char>(); it++) {
            if (this->_memory.size() == ZMachine::STORY_FILE_MAX_SIZE) {
                throw InvalidStoryFileException(); // storyfile too large
            }
            this->_memory.push_back((Byte)*it);
        }
        // re-allocate memory down to exact size --we're not going to resize it again
        this->_memory.shrink_to_fit();
        // we can now work out where the end of static memory is
        this->_static_memory_end = std::clamp((Address)(this->_memory.size() - 1), Address{0x0}, Address{0x0ffff});
    }

    void ZMachine::_setup_accessors() {
        this->_writeable_memory = std::span<Byte>{this->_memory}.subspan(0, this->_static_memory_begin);
        this->_readable_memory = std::span<Byte>{this->_memory}.subspan(0, this->_static_memory_end - 1);
    }

    void ZMachine::_print_next_instruction() {
        std::cout << std::hex << this->_pc << " ";
        std::span<const Byte> memory_view{this->_memory}; // read only accessor for memory
        Instruction instruction = Instruction::decode(this->_pc, memory_view);
        std::cout << instruction.to_string();
        std::cin.get();
    }
}
