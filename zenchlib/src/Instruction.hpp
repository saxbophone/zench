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
            Operand(OperandType type) : type(type) {}
        };

        enum class Form {
            LONG, SHORT, EXTENDED, VARIABLE,
        };

        enum class Category {
            _0OP, _1OP, _2OP, VAR, EXT,
        };

        struct Branch {
            bool on_true; // whether branch is on true (otherwise, on false)
            SWord offset : 14; // branch offset
        };

        struct StringLiteral {
            Address address;
            std::size_t length = 0;
        };

        Opcode opcode;
        Form form; // form specifies the structure of an instruction
        Category category; // categories are rather misleadingly named after their operand counts, a trait that doesn't quite hold true
        std::vector<Operand> operands;
        std::optional<Byte> store_variable;
        std::optional<Branch> branch;
        std::optional<StringLiteral> trailing_string_literal;
        // strictly metadata fields for assembly output:
        Address location; // address of the first byte of this instruction
        std::vector<Byte> bytecode; // the raw bytes that encode this instruction

        static bool _is_instruction_store(Instruction instruction) {
            // NOTE: store opcodes from versions greater than v3 ignored
            // also extended form, but we're not handling those right now
            if (instruction.category == Instruction::Category::VAR) {
                switch (instruction.opcode) {
                case 0x00: case 0x07:
                    return true;
                default:
                    return false;
                }
            } else if (instruction.form == Instruction::Form::EXTENDED) {
                // let's trap on extended instructions anyway (should never reach here)
                throw UnsupportedVersionException();
            }
            // otherwise...
            switch (instruction.category) {
            case Instruction::Category::_0OP: // 0OP
                return false; // no 0OP opcodes store in v3 (v5 does have one)
            case Instruction::Category::_1OP: // 1OP
                switch (instruction.opcode) {
                case 0x01: case 0x02: case 0x03: case 0x04: case 0x0e: case 0x0f:
                    return true;
                default:
                    return false;
                }
            case Instruction::Category::_2OP: // 2OP
                switch (instruction.opcode) {
                case 0x08: case 0x09: case 0x0f: case 0x10: case 0x11: case 0x12:
                case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: case 0x18:
                    return true;
                default:
                    return false;
                }
            default: // XXX: should never reach here
                throw Exception();
            }
            return false;
        }

        static bool _is_instruction_branch(Instruction instruction) {
            // NOTE: branch opcodes from versions greater than v3 ignored
            // also extended form, but we're not handling those right now
            if (instruction.category == Instruction::Category::VAR) {
                return false; // There are NO branching VAR instructions in v3!
            } else if (instruction.form == Instruction::Form::EXTENDED) {
                // let's trap on extended instructions anyway (should never reach here)
                throw UnsupportedVersionException();
            }
            // otherwise...
            switch (instruction.category) {
            case Instruction::Category::_0OP: // 0OP
                switch (instruction.opcode) {
                case 0x05: case 0x06: case 0x0d:
                    return true;
                default:
                    return false;
                }
            case Instruction::Category::_1OP: // 1OP
                return instruction.opcode < 3; // 0, 1 and 2 all branch
            case Instruction::Category::_2OP: // 2OP
                return
                    (0 < instruction.opcode and instruction.opcode < 8)
                    or instruction.opcode == 0x0a;
            default: // XXX: should never reach here
                throw Exception();
            }
            return false;
        }

        // NOTE: modifies pc in-place!
        static Instruction decode(Address& pc, std::span<const Byte> memory_view) {
            Instruction instruction;
            // store instruction origin address
            instruction.location = pc;
            Byte first = memory_view[pc++]; // first byte of instruction
            instruction.bytecode.push_back(first);
            // determine the instruction's form first, this is useful mainly for categorising instructions
            if (first == 0xBE) { // extended mode
                // XXX: extended mode not implemented, we're only targeting version 3 right now
                // TODO: implement extended mode when running Version 5
                throw UnsupportedVersionException();
            } else {
                // read top two bits to determine instruction form
                Byte top_bits = first >> 6;
                switch (top_bits) {
                case 0b11:
                    instruction.form = Instruction::Form::VARIABLE;
                    // if bit 5 is not set, then it's *categorised* as 2-OP but it's not actually limited to only 2 operands!
                    if ((first & 0b00100000) == 0) {
                        // in other words, a 2OP-category opcode assembled in variable form
                        instruction.category = Instruction::Category::_2OP;
                    } else {
                        // otherwise, it is in fact a *true* VAR opcode assembled in variable form!
                        instruction.category = Instruction::Category::VAR;
                    }
                    // opcode is always bottom 5 bits
                    instruction.opcode = first & 0b00011111;
                    break;
                case 0b10: {
                    instruction.form = Instruction::Form::SHORT;
                    // determine argument type and opcode --type is bits 4 and 5
                    Instruction::OperandType type = (Instruction::OperandType)((first & 0b00110000) >> 4);
                    // don't push OMITTED types into operand list
                    if (type != Instruction::OperandType::OMITTED) {
                        instruction.category = Instruction::Category::_1OP;
                        instruction.operands = {type};
                    } else {
                        instruction.category = Instruction::Category::_0OP;
                    }
                    // opcode is always bottom 4 bits
                    instruction.opcode = first & 0b00001111;
                    break;
                }
                default:
                    instruction.form = Instruction::Form::LONG;
                    instruction.category = Instruction::Category::_2OP;
                    // long form is always 2-OP --op types are packed into bits 6 and 5
                    instruction.operands = {
                        first & 0b01000000 ? Instruction::OperandType::VARIABLE : Instruction::OperandType::SMALL_CONSTANT,
                        first & 0b00100000 ? Instruction::OperandType::VARIABLE : Instruction::OperandType::SMALL_CONSTANT,
                    };
                    // opcode is always bottom 5 bits
                    instruction.opcode = first & 0b00011111;
                    break;
                }
            }
            // handle variable-form operand types
            if (instruction.form == Instruction::Form::VARIABLE) {
                // XXX: handle double-var opcodes (two operand type bytes)
                if (instruction.opcode == 12 or instruction.opcode == 26) {
                    throw UnsupportedVersionException();
                }
                // read operand types from the next byte
                Byte operand_types = memory_view[pc++];
                instruction.bytecode.push_back(operand_types);
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
                    instruction.bytecode.push_back(memory_view[pc]);
                    instruction.bytecode.push_back(memory_view[pc + 1]);
                    pc += 2;
                } else {
                    // both SMALL_CONSTANT and VARIABLE are byte-sized
                    instruction.bytecode.push_back(memory_view[pc]);
                    operand.byte = memory_view[pc++];
                }
            }
            // handle store if this instruction stores a result
            if (Instruction::_is_instruction_store(instruction)) {
                instruction.bytecode.push_back(memory_view[pc]);
                instruction.store_variable = memory_view[pc++];
            }
            // handle branch if this instruction is branching
            if (Instruction::_is_instruction_branch(instruction)) {
                // decode branch address and store in branch_offset
                Byte branch = memory_view[pc++];
                instruction.bytecode.push_back(branch);
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
                    instruction.bytecode.push_back(memory_view[pc]);
                    instruction.branch->offset = ((Word)(branch & 0b00111111) << 8) + memory_view[pc++];
                }
            }
            // as a special case, instructions *print* and *print_ret* have a literal string following them, which we need to skip
            if (instruction.category == Instruction::Category::_0OP) {
                if (instruction.opcode == 2 or instruction.opcode == 3) {
                    instruction.trailing_string_literal = {
                        .address = pc,
                        .length = 0,
                    };
                    // Z-characters are encoded in 2-byte chunks, the string ends with a chunk whose first byte has its highest bit set
                    while ((memory_view[pc] & 0b10000000) == 0) {
                        instruction.bytecode.push_back(memory_view[pc]);
                        instruction.bytecode.push_back(memory_view[pc + 1]);
                        pc += 2;
                        instruction.trailing_string_literal->length += 2;
                    }
                    instruction.bytecode.push_back(memory_view[pc]);
                    instruction.bytecode.push_back(memory_view[pc + 1]);
                    pc += 2;
                    instruction.trailing_string_literal->length += 2;
                }
            }
            return instruction;
        }

        std::string address_string() const {
            std::stringstream address;
            address << std::hex << std::setw(6) << this->location;
            return address.str();
        }

        std::string get_2op_name() const {
            switch (opcode) {
            case 0x01: return "je";
            case 0x02: return "jl";
            case 0x03: return "jg";
            case 0x04: return "dec_chk";
            case 0x05: return "inc_chk";
            case 0x06: return "jin";
            case 0x07: return "test";
            case 0x08: return "or";
            case 0x09: return "and";
            case 0x0a: return "test_attr";
            case 0x0b: return "set_attr";
            case 0x0c: return "clear_attr";
            case 0x0d: return "store";
            case 0x0e: return "insert_obj";
            case 0x0f: return "loadw";
            case 0x10: return "loadb";
            case 0x11: return "get_prop";
            case 0x12: return "get_prop_addr";
            case 0x13: return "get_next_prop";
            case 0x14: return "add";
            case 0x15: return "sub";
            case 0x16: return "mul";
            case 0x17: return "div";
            case 0x18: return "mod";
            default: return "mnemonic?";
            }
        }

        std::string get_1op_name() const {
            switch (opcode) {
            case 0x0: return "jz";
            case 0x1: return "get_sibling";
            case 0x2: return "get_child";
            case 0x3: return "get_parent";
            case 0x4: return "get_prop_len";
            case 0x5: return "inc";
            case 0x6: return "dec";
            case 0x7: return "print_addr";
            case 0x9: return "remove_obj";
            case 0xa: return "print_obj";
            case 0xb: return "ret";
            case 0xc: return "jump";
            case 0xd: return "print_paddr";
            case 0xe: return "load";
            case 0xf: return "not";
            default: return "mnemonic?";
            }
        }

        std::string get_0op_name() const {
            switch (opcode) {
            case 0x0: return "rtrue";
            case 0x1: return "rfalse";
            case 0x2: return "print";
            case 0x3: return "print_ret";
            case 0x4: return "nop";
            case 0x5: return "save";
            case 0x6: return "restore";
            case 0x7: return "restart";
            case 0x8: return "ret_popped";
            case 0x9: return "pop";
            case 0xa: return "quit";
            case 0xb: return "new_line";
            case 0xc: return "show_status";
            case 0xd: return "verify";
            default: return "mnemonic?";
            }
        }

        std::string get_var_name() const {
            switch (opcode) {
            case 0x00: return "call";
            case 0x01: return "storew";
            case 0x02: return "storeb";
            case 0x03: return "put_prop";
            case 0x04: return "sread";
            case 0x05: return "print_char";
            case 0x06: return "print_num";
            case 0x07: return "random";
            case 0x08: return "push";
            case 0x09: return "pull";
            case 0x0a: return "split_window";
            case 0x0b: return "set_window";
            case 0x13: return "output_stream";
            case 0x14: return "input_stream";
            case 0x15: return "sound_effect"; // XXX: not sure if this is in v3
            default: return "mnemonic?";
            }
        }

        std::string get_ext_name() const {
            return "EXT";
        }

        std::string mnemonic_string() const {
            switch (category) {
            case Category::_0OP:
                return get_0op_name();
            case Category::_1OP:
                return get_1op_name();
            case Category::_2OP:
                return get_2op_name();
            case Category::VAR:
                return get_var_name();
            case Category::EXT:
                return get_ext_name();
            default:
                return "op-count?";
            }
        }

        std::string arguments_string() const {
            std::stringstream arguments;
            if (operands.size() > 0) {
                arguments << " ";
            }
            for (std::size_t i = 0; i < operands.size(); i++) {
                if (operands[i].type != OperandType::VARIABLE) {
                    arguments << "#";
                }
                arguments << std::hex;
                if (operands[i].type == OperandType::LARGE_CONSTANT) {
                    arguments << std::setfill('0') << std::setw(4);
                    arguments << operands[i].word;
                } else {
                    arguments << std::setfill('0') << std::setw(2);
                    arguments << (Word)operands[i].byte;
                }
                if (i < operands.size() - 1) {
                    arguments << ",";
                }
            }
            return arguments.str();
        }

        std::string literal_string() const {
            return trailing_string_literal ? " {Z-str}" : "";
        }

        std::string store_string() const {
            if (not store_variable) { return ""; }
            std::stringstream store;
            store << " -> " << std::hex << std::setfill('0') << std::setw(2) << (Word)*store_variable;
            return store.str();
        }

        std::string branch_string() const {
            if (not branch) { return ""; }
            std::stringstream jump;
            jump << " ?" << (branch->on_true ? " " : "! ");
            jump << branch->offset;
            return branch ? jump.str() : "";
        }

        std::string metadata() const {
            std::stringstream data;
            switch (form) {
            case Form::LONG:
                data << "long";
                break;
            case Form::SHORT:
                data << "short";
                break;
            case Form::EXTENDED:
                data << "extended";
                break;
            case Form::VARIABLE:
                data << "variable";
                break;
            default:
                data << "????";
            }
            data << " form; count ";
            switch (category) {
            case Category::_0OP:
                data << "0OP";
                break;
            case Category::_1OP:
                data << "1OP";
                break;
            case Category::_2OP:
                data << "2OP";
                break;
            case Category::VAR:
                data << "VAR";
                break;
            case Category::EXT:
                data << "EXT";
                break;
            default:
                data << "????";
            }
            data << "; opcode number " << std::hex << (Word)opcode << ";";
            return data.str();
        }

        std::string bytecode_string() const {
            std::stringstream bytes;
            for (std::size_t i = 0; i < this->bytecode.size(); i++) {
                if (i != 0) {
                    bytes << " ";
                }
                bytes << std::hex << std::setfill('0') << std::setw(2) << (Word)this->bytecode[i];
            }
            return bytes.str();
        }

        std::string to_string() const {
            return
                address_string() + ": @" + mnemonic_string() +
                arguments_string() + literal_string() +
                store_string() + branch_string() + ";\n\t" + metadata() + "\n\t" + bytecode_string();
        }
    };
}

#endif // include guard
