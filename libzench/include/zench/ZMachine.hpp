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
 *
 */

#ifndef COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP
#define COM_SAXBOPHONE_ZENCH_ZMACHINE_HPP

#include <cstddef>   // size_t
#include <cstdint>   // fixed-width types

#include <bitset>    // bitset
#include <deque>     // deque
#include <istream>   // istream
#include <optional>  // optional
#include <span>      // span
#include <vector>    // vector

#include <zench/zench.hpp>

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        // NOTE: it is permitted for story_file to be closed for further reading after this constructor returns
        ZMachine(std::istream& story_file);

        bool is_running(); // returns true if a runnable machine has not quit

        void execute(); // executes one instruction
                                                            // v87654321
        static constexpr std::bitset<8> SUPPORTED_VERSIONS = {0b00000100};

    private:
        struct StackFrame {
            Address return_pc; // address to return to from this routine
            std::optional<Byte> result_ref; // variable to store result in, if any
            std::bitset<7> arguments_supplied;
            std::vector<Word> local_variables; // current contents of locals --never more than 15 of them
            std::deque<Word> local_stack; // the "inner" stack directly accessible to routine
        };
        // TODO: create a WordDelegate class which can refer to the bytes its
        // made up of and write back to them when =operator is used on it
        Word _load_word(WordAddress address);
        // loads file header only
        void _load_header(std::istream& story_file);
        // loads the rest of the file after header has been loaded
        void _load_remaining(std::istream& story_file);
        // sets up span accessors for reading according to memory map
        void _setup_accessors();
        // TODO: consider whether these two functions should be merged
        Word& _global_variable(Byte number);
        Word& _local_variable(Byte number);
        // TODO: local stack access/manipulation

        // NOTE: this method advances the Program Counter (_pc) and writes to stdout
        void _print_next_instruction();

        static constexpr std::size_t HEADER_SIZE = 64;
        static constexpr std::size_t STORY_FILE_MAX_SIZE = 128 * 1024; // Version 1-3: 128KiB

        bool _state_valid = false; // whether the machine is runnable
        bool _is_running = false; // whether the machine has not quit

        ByteAddress _static_memory_begin; // derived from header
        ByteAddress _static_memory_end; // we have to work this out
        ByteAddress _high_memory_begin; // "high memory mark", derived from header

        Address _pc = 0x000000; // program counter
        /*
         * the entire main memory of the VM, comprising of dynamic, static and
         * high memory all joined together continguously.
         * NOTE: use the specific accessor properties to access each of the sub
         * ranges of memory only
         */
        std::vector<Byte> _memory;
        // memory region accessors --NOTE: we might not even need these, memory is normally addressed from base!
        std::span<Byte> _dynamic_memory;
        // the Z-code program is not allowed to modify this, even though we are!
        std::span<Byte> _static_memory;
        // probably more useful, accessors for the ranges of memory that are writeable and readable (by Z-code)
        std::span<Byte> _writeable_memory; // dynamic memory only
        std::span<Byte> _readable_memory; // both dynamic and static memory
        // the Z-code program is not allowed to modify this, even though we are!
        std::span<Byte> _high_memory;
        /*
         * function call stack
         * NOTE: to make things more consistent across different Z-code versions,
         * when not in V6 (which has an explicit "main" routine), we initialise
         * the call stack with a mostly-empty dummy stack frame which represents
         * the execution entrypoint. Just like V6's explicit main, it is a fatal
         * error to return or catch from this frame, or to throw to it.
         */
        std::deque<StackFrame> _call_stack;
    };
}

#endif // include guard