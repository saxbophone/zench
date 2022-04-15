/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <algorithm>
#include <iostream> // XXX: debug
#include <iterator>

#include <zench/ZMachine.hpp>

namespace com::saxbophone::zench {
    ZMachine::ZMachine(std::istream& story_file) {
        // check stream can be read from
        if (not story_file.good()) {
            throw CantReadStoryFileException();
        }
        // check file version
        std::uint8_t file_version = story_file.peek();
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
        // XXX: hacked program counter to test a negative jump
        this->_pc = 0x574e;
        this->_call_stack.emplace_back(); // setup dummy stack frame
        this->_state_valid = true; // this VM is ready to go
    }

    bool ZMachine::is_running() {
        return true;
    }

    void ZMachine::execute() {
        // XXX: debug
        std::cout << std::hex << this->_pc << ": " << this->_decode_instruction().to_string(); // no newline due to cin.get()
        std::cin.get(); // XXX: wait for newline to prevent instruction decoding demo from running too fast
    }

    ZMachine::Word ZMachine::_load_word(ByteAddress address) {
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

    ZMachine::Instruction ZMachine::_decode_instruction() {
        Byte opcode = this->_memory[this->_pc++]; // first byte of instruction
        Instruction instruction;
        // determine the instruction's form first, this is useful mainly for categorising instructions
        if (opcode == 0xBE) { // extended mode
            // XXX: extended mode not implemented, we're only targeting version 3 right now
            // TODO: implement extended mode when running Version 5
            throw UnsupportedVersionException();
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
            // read operand types from the next byte
            Byte operand_types = this->_memory[this->_pc++];
            // if bit 5 is not set, then it's 2-OP
            if ((opcode & 0b00100000) == 0) {
                // instruction.operands = {
                //     (Instruction::OperandType)(operand_types >> 6),
                //     (Instruction::OperandType)((operand_types >> 4) & 0b11),
                // };
                instruction.arity = Instruction::Arity::OP2;
            }
            // } else { // otherwise, read from operand type byte until OMITTED is found
                for (int i = 4; i --> 0;) {
                    Instruction::OperandType type = (Instruction::OperandType)((operand_types >> i * 2) & 0b11);
                    if (type == Instruction::OperandType::OMITTED) {
                        break;
                    }
                    instruction.operands.emplace_back(type);
                }
            // }
            // XXX: handle double-var opcodes (two operand type bytes)
            if (instruction.opcode == 12 or instruction.opcode == 26) {
                throw UnsupportedVersionException();
            }
        }
        // now we can read in the actual operand values
        for (auto& operand : instruction.operands) {
            // this is the only type that pulls a word rather than a byte
            if (operand.type == Instruction::OperandType::LARGE_CONSTANT) {
                operand.word = this->_load_word(this->_pc);
                this->_pc += 2;
            } else {
                // both SMALL_CONSTANT and VARIABLE are byte-sized
                operand.byte = this->_memory[this->_pc++];
            }
        }
        // handle store if this instruction stores a result
        if (this->_is_instruction_store(instruction)) {
            instruction.store_variable = this->_memory[this->_pc++];
        }
        // handle branch if this instruction is branching
        if (this->_is_instruction_branch(instruction)) {
            // decode branch address and store in branch_offset
            Byte branch = this->_memory[this->_pc++];
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
                instruction.branch->offset = ((Word)(branch & 0b00111111) << 8) + this->_memory[this->_pc++];
            }
        }
        // TODO: modulo program counter!
        return instruction;
    }

    bool ZMachine::_is_instruction_store(Instruction instruction) const {
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
            throw UnsupportedVersionException();
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
            throw Exception();
        }
        return false;
    }
    bool ZMachine::_is_instruction_branch(Instruction instruction) const {
        // NOTE: branch opcodes from versions greater than v3 ignored
        // also extended form, but we're not handling those right now
        if (instruction.arity == Instruction::Arity::VAR) {
            return false; // There are NO branching VAR instructions in v3!
        } else if (instruction.form == Instruction::Form::EXTENDED) {
            // let's trap on extended instructions anyway (should never reach here)
            throw UnsupportedVersionException();
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
            throw Exception();
        }
        return false;
    }
}
