/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <cstddef>   // size_t

#include <algorithm> // clamp
#include <bitset>    // bitset
#include <deque>     // deque
#include <iostream>  // XXX: debug
#include <istream>   // istream
#include <iterator>  // istreambuf_iterator
#include <memory>    // unique_ptr
#include <optional>  // optional
#include <span>      // span
#include <vector>    // vector

#include <zench/zench.hpp>
#include <zench/ZMachine.hpp>

#include "Instruction.hpp"

namespace com::saxbophone::zench {
    class ZMachine::ZMachineImpl {
    public:
        struct StackFrame {
            Address return_pc; // address to return to from this routine
            std::optional<Byte> result_ref; // variable to store result in, if any
            std::bitset<7> arguments_supplied;
            std::vector<Word> local_variables; // current contents of locals --never more than 15 of them
            std::deque<Word> local_stack; // the "inner" stack directly accessible to routine
        };

        static constexpr std::size_t HEADER_SIZE = 64;
        static constexpr std::size_t STORY_FILE_MAX_SIZE = 128 * 1024; // Version 1-3: 128KiB

        ZMachine& parent;

        bool is_running = false; // whether the machine has not quit

        ByteAddress static_memory_begin; // derived from header
        ByteAddress static_memory_end; // we have to work this out
        ByteAddress high_memory_begin; // "high memory mark", derived from header

        Address pc = 0x000000; // program counter
        /*
         * the entire main memory of the VM, comprising of dynamic, static and
         * high memory all joined together continguously.
         * NOTE: use the specific accessor properties to access each of the sub
         * ranges of memory only
         */
        std::vector<Byte> memory;
        // memory region accessors --NOTE: we might not even need these, memory is normally addressed from base!
        std::span<Byte> dynamic_memory;
        // the Z-code program is not allowed to modify this, even though we are!
        std::span<Byte> static_memory;
        // probably more useful, accessors for the ranges of memory that are writeable and readable (by Z-code)
        std::span<Byte> writeable_memory; // dynamic memory only
        std::span<Byte> readable_memory; // both dynamic and static memory
        // the Z-code program is not allowed to modify this, even though we are!
        std::span<Byte> high_memory;
        /*
         * function call stack
         * NOTE: to make things more consistent across different Z-code versions,
         * when not in V6 (which has an explicit "main" routine), we initialise
         * the call stack with a mostly-empty dummy stack frame which represents
         * the execution entrypoint. Just like V6's explicit main, it is a fatal
         * error to return or catch from this frame, or to throw to it.
         */
        std::deque<StackFrame> call_stack;

        ZMachineImpl(ZMachine& vm) : parent(vm) {}

        // TODO: create a WordDelegate class which can refer to the bytes its
        // made up of and write back to them when =operator is used on it
        Word load_word(ByteAddress address) {
            return (memory[address] << 8) + memory[address + 1];
        }
        // loads file header only
        void load_header(std::istream& story_file) {
            memory.resize(ZMachine::HEADER_SIZE); // pre-allocate enough for header
            for (auto& byte : memory) {
                auto next = story_file.get();
                if (not story_file) { // handle EOF/failbit
                    throw InvalidStoryFileException(); // header ended prematurely
                }
                byte = next;
            }
            // work out the memory map
            static_memory_begin = this->load_word(0x0e);
            // validate size of dynamic memory (must be at least 64 bytes)
            if (static_memory_begin < ZMachine::HEADER_SIZE) {
                throw InvalidStoryFileException();
            }
            // skip _static_memory_end for now --we won't know it until we've read the rest of the file
            high_memory_begin = this->load_word(0x04);
            // bottom of high memory must not overlap top of dynamic memory
            if (high_memory_begin < static_memory_begin) {
                throw InvalidStoryFileException();
            }
        }
        // loads the rest of the file after header has been loaded
        void load_remaining(std::istream& story_file) {
            // pre-allocate to the maximum allowed storyfile size
            memory.reserve(ZMachine::STORY_FILE_MAX_SIZE);
            // read in the remainder of the memory in the storyfile
            for (auto it = std::istreambuf_iterator<char>(story_file); it != std::istreambuf_iterator<char>(); it++) {
                if (memory.size() == ZMachine::STORY_FILE_MAX_SIZE) {
                    throw InvalidStoryFileException(); // storyfile too large
                }
                memory.push_back((Byte)*it);
            }
            // re-allocate memory down to exact size --we're not going to resize it again
            memory.shrink_to_fit();
            // we can now work out where the end of static memory is
            static_memory_end = std::clamp((Address)(memory.size() - 1), Address{0x0}, Address{0x0ffff});
        }
        // sets up span accessors for reading according to memory map
        void setup_accessors() {
            writeable_memory = std::span<Byte>{memory}.subspan(0, static_memory_begin);
            readable_memory = std::span<Byte>{memory}.subspan(0, static_memory_end - 1);
        }
        // TODO: consider whether these two functions should be merged
        Word& global_variable(Byte number);
        Word& local_variable(Byte number);
        // TODO: local stack access/manipulation

        // NOTE: this method advances the Program Counter (_pc) and writes to stdout
        void print_next_instruction() {
            std::span<const Byte> memory_view{memory}; // read only accessor for memory
            Instruction instruction = Instruction::decode(pc, memory_view); // modifies pc in-place
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
        this->_impl->pc = this->_impl->load_word(0x06); // load initial program counter
        this->_impl->call_stack.emplace_back(); // setup dummy stack frame
        this->_impl->is_running = true;
    }

    ZMachine::~ZMachine() = default; // needed to allow pimpl idiom to work
    // see: https://www.fluentcpp.com/2017/09/22/make-pimpl-using-unique_ptr/

    bool ZMachine::is_running() {
        return this->_impl->is_running;
    }

    void ZMachine::execute() {
        // XXX: debug
        this->_impl->print_next_instruction();
    }
}
