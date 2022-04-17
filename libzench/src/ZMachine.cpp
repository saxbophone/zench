/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <algorithm> // clamp
#include <iostream>  // XXX: debug
#include <istream>   // istream
#include <iterator>  // istreambuf_iterator
#include <span>      // span

#include <zench/zench.hpp>
#include <zench/ZMachine.hpp>

#include "Instruction.hpp"

namespace com::saxbophone::zench {
    class ZMachine::ZMachineImpl {
    private:
        ZMachine& parent;
    public:
        ZMachineImpl(ZMachine& vm) : parent(vm) {}

        // TODO: create a WordDelegate class which can refer to the bytes its
        // made up of and write back to them when =operator is used on it
        Word load_word(ByteAddress address) {
            return (parent._memory[address] << 8) + parent._memory[address + 1];
        }
        // loads file header only
        void load_header(std::istream& story_file) {
            parent._memory.resize(ZMachine::HEADER_SIZE); // pre-allocate enough for header
            for (auto& byte : parent._memory) {
                auto next = story_file.get();
                if (not story_file) { // handle EOF/failbit
                    throw InvalidStoryFileException(); // header ended prematurely
                }
                byte = next;
            }
            // work out the memory map
            parent._static_memory_begin = this->load_word(0x0e);
            // validate size of dynamic memory (must be at least 64 bytes)
            if (parent._static_memory_begin < ZMachine::HEADER_SIZE) {
                throw InvalidStoryFileException();
            }
            // skip _static_memory_end for now --we won't know it until we've read the rest of the file
            parent._high_memory_begin = this->load_word(0x04);
            // bottom of high memory must not overlap top of dynamic memory
            if (parent._high_memory_begin < parent._static_memory_begin) {
                throw InvalidStoryFileException();
            }
        }
        // loads the rest of the file after header has been loaded
        void load_remaining(std::istream& story_file) {
            // pre-allocate to the maximum allowed storyfile size
            parent._memory.reserve(ZMachine::STORY_FILE_MAX_SIZE);
            // read in the remainder of the memory in the storyfile
            for (auto it = std::istreambuf_iterator<char>(story_file); it != std::istreambuf_iterator<char>(); it++) {
                if (parent._memory.size() == ZMachine::STORY_FILE_MAX_SIZE) {
                    throw InvalidStoryFileException(); // storyfile too large
                }
                parent._memory.push_back((Byte)*it);
            }
            // re-allocate memory down to exact size --we're not going to resize it again
            parent._memory.shrink_to_fit();
            // we can now work out where the end of static memory is
            parent._static_memory_end = std::clamp((Address)(parent._memory.size() - 1), Address{0x0}, Address{0x0ffff});
        }
        // sets up span accessors for reading according to memory map
        void setup_accessors() {
            parent._writeable_memory = std::span<Byte>{parent._memory}.subspan(0, parent._static_memory_begin);
            parent._readable_memory = std::span<Byte>{parent._memory}.subspan(0, parent._static_memory_end - 1);
        }
        // TODO: consider whether these two functions should be merged
        Word& global_variable(Byte number);
        Word& local_variable(Byte number);
        // TODO: local stack access/manipulation

        // NOTE: this method advances the Program Counter (_pc) and writes to stdout
        void print_next_instruction() {
            std::span<const Byte> memory_view{parent._memory}; // read only accessor for memory
            Instruction instruction = Instruction::decode(parent._pc, memory_view); // modifies pc in-place
            std::cout << instruction.to_string();
            std::cin.get();
        }
    };

    ZMachine::ZMachine(std::istream& story_file) : _impl(new ZMachineImpl(*this)) {
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
        this->_impl->load_header(story_file);
        this->_impl->load_remaining(story_file);
        this->_impl->setup_accessors();
        this->_pc = this->_impl->load_word(0x06); // load initial program counter
        this->_call_stack.emplace_back(); // setup dummy stack frame
        this->_is_running = true;
    }

    ZMachine::~ZMachine() = default; // needed to allow pimpl idiom to work
    // see: https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/

    bool ZMachine::is_running() {
        return this->_is_running;
    }

    void ZMachine::execute() {
        // XXX: debug
        this->_impl->print_next_instruction();
    }
}
