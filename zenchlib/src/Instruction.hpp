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

#ifndef COM_SAXBOPHONE_ZENCH_INSTRUCTION_HPP
#define COM_SAXBOPHONE_ZENCH_INSTRUCTION_HPP

#include <cstddef>   // size_t

#include <iomanip>   // setfill
#include <optional>  // optional
#include <span>      // span
#include <string>    // string
#include <sstream>   // stringstream
#include <vector>    // vector

#include <zench/zench.hpp>

namespace com::saxbophone::zench {
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
                default:
                    throw "InvalidStoryFileException()";
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

        static bool _is_instruction_store(Instruction instruction) {
            // NOTE: store opcodes from versions greater than v3 ignored
            // also extended form, but we're not handling those right now
            if (instruction.arity == Instruction::Arity::VAR) {
                switch (instruction.opcode) {
                case 0x00: case 0x07:
                    return true;
                default:
                    return false;
                }
            } else if (instruction.form == Instruction::Form::EXTENDED) {
                // let's trap on extended instructions anyway (should never reach here)
                throw "UnsupportedVersionException()";
            }
            // otherwise...
            switch (instruction.arity) {
            case Instruction::Arity::OP0: // 0OP
                return false; // no 0OP opcodes store in v3 (v5 does have one)
            case Instruction::Arity::OP1: // 1OP
                switch (instruction.opcode) {
                case 0x01: case 0x02: case 0x03: case 0x04: case 0x0e: case 0x0f:
                    return true;
                default:
                    return false;
                }
            case Instruction::Arity::OP2: // 2OP
                switch (instruction.opcode) {
                case 0x08: case 0x09: case 0x0f: case 0x10: case 0x11: case 0x12:
                case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
                    return true;
                default:
                    return false;
                }
            default: // XXX: should never reach here
                throw "Exception()";
            }
            return false;
        }

        static bool _is_instruction_branch(Instruction instruction) {
            // NOTE: branch opcodes from versions greater than v3 ignored
            // also extended form, but we're not handling those right now
            if (instruction.arity == Instruction::Arity::VAR) {
                return false; // There are NO branching VAR instructions in v3!
            } else if (instruction.form == Instruction::Form::EXTENDED) {
                // let's trap on extended instructions anyway (should never reach here)
                throw "UnsupportedVersionException()";
            }
            // otherwise...
            switch (instruction.arity) {
            case Instruction::Arity::OP0: // 0OP
                switch (instruction.opcode) {
                case 0x05: case 0x06: case 0x0d:
                    return true;
                default:
                    return false;
                }
            case Instruction::Arity::OP1: // 1OP
                return instruction.opcode < 3; // 0, 1 and 2 all branch
            case Instruction::Arity::OP2: // 2OP
                return
                    (0 < instruction.opcode and instruction.opcode < 8)
                    or instruction.opcode == 0x0a;
            default: // XXX: should never reach here
                throw "Exception()";
            }
            return false;
        }

        static Instruction decode(Address& pc, std::span<const Byte> memory_view) {
            Byte opcode = memory_view[pc++]; // first byte of instruction
            Instruction instruction;
            // determine the instruction's form first, this is useful mainly for categorising instructions
            if (opcode == 0xBE) { // extended mode
                // XXX: extended mode not implemented, we're only targeting version 3 right now
                // TODO: implement extended mode when running Version 5
                throw "UnsupportedVersionException()";
            } else {
                // read top two bits to determine instruction form
                Byte top_bits = opcode >> 6;
                switch (top_bits) {
                case 0b11:
                    instruction.form = Instruction::Form::VARIABLE;
                    instruction.arity = Instruction::Arity::VAR;
                    // opcode is always bottom 5 bits
                    instruction.opcode = opcode & 0b00011111;
                    break;
                case 0b10: {
                    instruction.form = Instruction::Form::SHORT;
                    // determine argument type and opcode --type is bits 4 and 5
                    Instruction::OperandType type = (Instruction::OperandType)((opcode & 0b00110000) >> 4);
                    // don't push OMITTED types into operand list
                    if (type != Instruction::OperandType::OMITTED) {
                        instruction.arity = Instruction::Arity::OP1;
                        instruction.operands = {type};
                    } else {
                        instruction.arity = Instruction::Arity::OP0;
                    }
                    // opcode is always bottom 4 bits
                    instruction.opcode = opcode & 0b00001111;
                    break;
                }
                default:
                    instruction.form = Instruction::Form::LONG;
                    instruction.arity = Instruction::Arity::OP2;
                    // long form is always 2-OP --op types are packed into bits 6 and 5
                    instruction.operands = {
                        opcode & 0b01000000 ? Instruction::OperandType::VARIABLE : Instruction::OperandType::SMALL_CONSTANT,
                        opcode & 0b00100000 ? Instruction::OperandType::VARIABLE : Instruction::OperandType::SMALL_CONSTANT,
                    };
                    // opcode is always bottom 5 bits
                    instruction.opcode = opcode & 0b00011111;
                    break;
                }
            }
            // handle variable-form operand types
            if (instruction.form == Instruction::Form::VARIABLE) {
                // XXX: handle double-var opcodes (two operand type bytes)
                if (instruction.opcode == 12 or instruction.opcode == 26) {
                    throw "UnsupportedVersionException()";
                }
                // read operand types from the next byte
                Byte operand_types = memory_view[pc++];
                // if bit 5 is not set, then it's *categorised* as 2-OP but it's not actually limited to only 2 operands!
                if ((opcode & 0b00100000) == 0) {
                    instruction.arity = Instruction::Arity::OP2;
                }
                for (int i = 4; i --> 0;) {
                    Instruction::OperandType type = (Instruction::OperandType)((operand_types >> i * 2) & 0b11);
                    if (type == Instruction::OperandType::OMITTED) {
                        break;
                    }
                    instruction.operands.emplace_back(type);
                }
            }
            // now we can read in the actual operand values
            for (auto& operand : instruction.operands) {
                // this is the only type that pulls a word rather than a byte
                if (operand.type == Instruction::OperandType::LARGE_CONSTANT) {
                    // would use ZMachine.load_word() but it's not accessible
                    operand.word = ((Word)memory_view[pc] << 8) + memory_view[pc + 1];
                    pc += 2;
                } else {
                    // both SMALL_CONSTANT and VARIABLE are byte-sized
                    operand.byte = memory_view[pc++];
                }
            }
            // handle store if this instruction stores a result
            if (Instruction::_is_instruction_store(instruction)) {
                instruction.store_variable = memory_view[pc++];
            }
            // handle branch if this instruction is branching
            if (Instruction::_is_instruction_branch(instruction)) {
                // decode branch address and store in branch_offset
                Byte branch = memory_view[pc++];
                instruction.branch = Instruction::Branch{
                    .on_true = (branch & 0b10000000) != 0,
                    .offset = 0,
                };
                // bit 6 of the first branch byte is set if the offset value is 1 byte only
                if ((branch & 0b01000000) != 0) { // it's a 1-byte branch
                    // use bottom 6 bits for offset
                    instruction.branch->offset = branch & 0b00111111;
                } else { // it's a 2-byte branch
                    // use bottom 6 bits of first byte and all 8 of the second
                    instruction.branch->offset = ((Word)(branch & 0b00111111) << 8) + memory_view[pc++];
                }
            }
            // as a special case, instructions *print* and *print_ret* have a literal string following them, which we need to skip
            if (instruction.arity == Instruction::Arity::OP0) {
                if (instruction.opcode == 2 or instruction.opcode == 3) {
                    instruction.trailing_string_literal = {
                        .address = pc,
                        .length = 0,
                    };
                    // Z-characters are encoded in 2-byte chunks, the string ends with a chunk whose first byte has its highest bit set
                    while ((memory_view[pc] & 0b10000000) == 0) {
                        pc += 2;
                        instruction.trailing_string_literal->length += 2;
                    }
                    pc += 2;
                    instruction.trailing_string_literal->length += 2;
                }
            }
            // TODO: modulo program counter!
            return instruction;
        }

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
                throw "InvalidStoryFileException()";
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
            default:
                throw "InvalidStoryFileException()";
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
}

#endif // include guard
