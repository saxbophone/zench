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

#include <cstdint>
#include <bitset>
#include <istream>
#include <optional>
#include <span>
#include <unordered_set>
#include <vector>

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        ZMachine(std::istream& story_file);

        explicit operator bool(); // returns true if the ZMachine is runnable

        bool is_running(); // returns true if a runnable machine has not quit

        void execute(); // executes one instruction

        static const std::unordered_set<char> SUPPORTED_VERSIONS;

    private:
        using Byte = std::uint8_t;
        using Word = std::uint16_t;
        using SWord = std::int16_t; // signed Word
        // NOTE: not a packed address --it's an index to a specific byte.
        using PC = std::uint32_t; // should store these as a 19-bit bitfield, max is 512KiB which fits in that many bits

        struct StackFrame {
            PC return_pc : 19; // address to return to from this routine
            std::optional<Byte> result_ref; // variable to store result in, if any
            std::bitset<7> arguments_supplied;
            std::vector<Word> local_variables; // current contents of locals
            std::vector<Word> local_stack; // the "inner" stack directly accessible to routine
        };
        bool _load_header(std::istream& story_file);
        // TODO: consider whether these two functions should be merged
        Word& _global_variable(Byte number);
        Word& _local_variable(Byte number);
        // TODO: local stack access/manipulation

        bool _state_valid = false; // whether the machine is runnable
        bool _is_running = false; // whether the machine has not quit

        PC _pc : 19 = 0x000; // program counter
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
        std::vector<StackFrame> _call_stack;
    };
}

#endif // include guard
