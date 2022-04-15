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
#include <exception> // exception
#include <iomanip>   // setfill
#include <istream>   // istream
#include <optional>  // optional
#include <span>      // span
#include <string>    // string
#include <sstream>   // stringstream
#include <vector>    // vector

namespace com::saxbophone::zench {
    class ZMachine {
    public:
        // base class for all of zench's exceptions
        class Exception : public std::exception {};
        class CantReadStoryFileException : public Exception {
            const char* what() const noexcept {
                return "Can't read story file";
            }
        };
        class UnsupportedVersionException : public Exception {
            // TODO: rewrite this out-of-header to report supported versions
            // and actual version given
            const char* what() const noexcept {
                return "Story file version not supported";
            }
        };
        class InvalidStoryFileException : public Exception {
            const char* what() const noexcept {
                return "Invalid story file";
            }
        };

        // NOTE: it is permitted for story_file to be closed for further reading after this constructor returns
        ZMachine(std::istream& story_file);

        bool is_running(); // returns true if a runnable machine has not quit

        void execute(); // executes one instruction
                                                            // v87654321
        static constexpr std::bitset<8> SUPPORTED_VERSIONS = {0b00000100};

    private:
        using Byte = std::uint8_t;
        using Word = std::uint16_t;
        using SWord = std::int16_t; // signed Word
        // these addresses can address any byte in memory. Range depends on version, but never bigger than 19-bit.
        using Address = std::uint32_t;
        using ByteAddress = std::uint16_t; // address to a Byte anywhere in dynamic or static memory
        using WordAddress = std::uint16_t; // address/2 of a Word anywhere in the bottom 128KiB of all memory

        struct Instruction {
            using Opcode = Byte; // TODO: maybe convert to enum?
            enum OperandType : Byte {
                LARGE_CONSTANT = 0b00,
                SMALL_CONSTANT = 0b01,
                VARIABLE = 0b10,
                OMITTED = 0b11,
            };

            struct Operand {
                OperandType type;
                Word word = 0x0000;
                Byte byte = 0x00;

                Operand() : type(OperandType::OMITTED) {}
                Operand(Word constant) : type(OperandType::LARGE_CONSTANT), word(constant) {}
                Operand(OperandType type) : type(type) {}
                Operand(OperandType type, Byte data) : type(type), byte(data) {}

                std::string to_string() const {
                    std::stringstream output;
                    output << std::hex;
                    switch (type) {
                    case OperandType::LARGE_CONSTANT:
                        output << std::setfill('0') << std::setw(4) << word;
                        return output.str();
                    case OperandType::VARIABLE:
                        output << "@";
                    case OperandType::SMALL_CONSTANT:
                        output << std::setw(2) << std::setfill('0') << (Word)byte;
                        return output.str();
                    case OperandType::OMITTED:
                        return "x";
                    }
                }
            };

            enum class Form {
                LONG, SHORT, EXTENDED, VARIABLE,
            };

            enum class Arity {
                OP0, OP1, OP2, VAR,
            };

            struct Branch {
                bool on_true; // whether branch is on true (otherwise, on false)
                SWord offset : 14; // branch offset

                std::string to_string() const {
                    return (on_true ? " #" : " !#") + std::to_string(offset);
                }
            };

            struct StringLiteral {
                Address address;
                std::size_t length = 0;
            };

            Opcode opcode;
            Form form; // XXX: technically, not in the instruction structure table in the spec, but it is mentioned
            Arity arity; // can't actually rely upon operands.size() as some 2-op opcodes have more than 2 args!
            std::vector<Operand> operands;
            std::optional<Byte> store_variable;
            std::optional<Branch> branch;
            std::optional<StringLiteral> trailing_string_literal;

            std::string form_name() const {
                switch (form) {
                case Form::LONG:
                    return "[  long  ]";
                case Form::SHORT:
                    return "[  short ]";
                case Form::EXTENDED:
                    return "[extended]";
                case Form::VARIABLE:
                    return "[variable]";
                default:
                    return "[unspecified]";
                }
            }

            std::string operand_arity() const {
                switch (arity) {
                case Arity::OP0:
                    return "0OP";
                case Arity::OP1:
                    return "1OP";
                case Arity::OP2:
                    return "2OP";
                case Arity::VAR:
                    return "VAR";
                }
            }

            std::string opcode_name() const {
                // just use numbers for now, no name decoding
                std::stringstream output;
                output << form_name() << " " << operand_arity() << ":" << std::left << std::setw(2) << std::hex << (Word)opcode;
                return output.str();
            }

            std::string arguments() const {
                if (operands.size() == 0) {
                    return "";
                }
                std::string arguments = " (";
                for (auto arg : operands) {
                    arguments += " " + arg.to_string();
                }
                arguments += " )";
                return arguments;
            }

            std::string store_code() const {
                std::stringstream output;
                if (store_variable) {
                    output << std::setfill('0') << std::setw(2) << std::hex << (Word)store_variable.value();
                }
                return store_variable ? " -> @" + output.str() : "";
            }

            std::string branch_code() const {
                return branch ? branch.value().to_string() : "";
            }

            std::string string_literal() const {
                std::stringstream output;
                if (trailing_string_literal) {
                    output << std::hex << (Address)trailing_string_literal->address;
                }
                return trailing_string_literal ? " Z-Str[" + std::to_string(trailing_string_literal->length) + "] @" + output.str() : "";
            }

            std::string to_string() const {
                return opcode_name() + arguments() + store_code() + branch_code() + string_literal();
            }
        };

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

        // NOTE: this method advances the Program Counter (_pc)
        Instruction _decode_instruction();

        bool _is_instruction_store(Instruction instruction) const;
        bool _is_instruction_branch(Instruction instruction) const;

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
