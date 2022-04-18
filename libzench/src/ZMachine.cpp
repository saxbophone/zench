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
            Byte result_ref; // variable to store result in, if any
            std::size_t argument_count; // number of arguments passed to this routine
            std::vector<Word> local_variables; // current contents of locals --never more than 15 of them
            std::deque<Word> local_stack; // the "inner" stack directly accessible to routine

            StackFrame() {}

            StackFrame(Address return_pc, Byte result_ref, std::size_t argument_count, std::size_t locals_count)
              : return_pc(return_pc)
              , result_ref(result_ref) 
              , argument_count(argument_count)
              , local_variables(locals_count)
              {}
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
        Word load_word(Address address) {
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
        // NOTE: we'll need a fancier version of this in the future, one that
        // returns some special reference-type to Word, allows reading from
        // 2 bytes out of memory that make up a word, and writing back to it
        // as if it's a word (but the actual bytes are written instead!)
        Word get_variable(Byte number) {
            return memory[0]; // XXX: bad bad bad! sending the version number!
        }
        // TODO: local stack access/manipulation

        static Address expand_packed_address(PackedAddress packed) {
            return 2 * packed; // XXX: version 1..3 only
        }

        // looks up the operand type and global, local variables (if needed)
        // returns the actual value intended, either literal or value stored in
        // denoted variable
        Word operand_value(Instruction::Operand operand) {
            switch (operand.type) {
            case Instruction::OperandType::LARGE_CONSTANT:
                return operand.word;
            case Instruction::OperandType::SMALL_CONSTANT:
                return operand.byte;
            case Instruction::OperandType::VARIABLE:
                // TODO: needs access to local and global variables!
                // return variable(operand.byte);
                // XXX: dummy version
                return get_variable(operand.byte);
            default:
                throw Exception(); // ERROR! type can't be OMITTED!
            }
        }

        void opcode_call(const Instruction& instruction) {
            // must have 1..4 operands --routine address + 0..3 arguments
            if (not (0 < instruction.operands.size() and instruction.operands.size() <= 4)) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // construct a new StackFrame for this routine, populated appropriately
            Address routine_address = expand_packed_address(operand_value(instruction.operands[0]));
            // XXX: handle special case: call address 0 returns false (0)
            if (routine_address == 0) {
                // TODO: needs access to local and global variables!
                // variable(instruction.store_variable) = 0;
                return;
            }
            Byte args_count = (Byte)(instruction.operands.size() - 1);
            Byte locals_count = this->memory[routine_address];
            StackFrame routine{
                this->pc, // return address, i.e. the byte after this call instruction
                instruction.store_variable.value(),
                args_count,
                locals_count
            };
            // populate local variables
            for (Byte l = 0; l < locals_count; l++) {
                routine.local_variables[l] = this->load_word(routine_address + l * 2);
            }
            // now, write in any arguments to local variables, but stop when the range of either is exceeded
            for (Byte a = 0; a < locals_count and a < args_count; a++) {
                auto operand = instruction.operands[1 + a];
                routine.local_variables[a] = operand_value(operand);
            }
            // finally, just push the new StackFrame to the call stack and move PC to new routine
            this->call_stack.push_back(routine);
            this->pc = routine_address + 1 + locals_count * 2; // start execution from end of routine header
        }

        void opcode_ret(const Instruction& instruction) {
            // must have 1 operand only!
            if (instruction.operands.size() != 1) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // preserve return value variable number
            Byte store_variable = this->call_stack.back().result_ref;
            // move pc to return address
            this->pc = this->call_stack.back().return_pc;
            // pop the stack
            this->call_stack.pop_back();
            // set result variable
            auto operand = instruction.operands[0];
            // TODO: needs access to local and global variables!
            if (operand.type == Instruction::OperandType::LARGE_CONSTANT) {
                // variable(store_variable) = operand.word;
            } else {
                // variable(store_variable) = operand.byte;
            }
        }

        void opcode_jump(const Instruction& instruction) {
            // must have 1 operand only!
            if (instruction.operands.size() != 1) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // the jump address is a 2-byte signed offset to apply to the PC
            SWord offset = (SWord)instruction.operands[0].word;
            /*
             * The destination of the jump opcode is:
             * Address after instruction + Offset - 2
             */
            this->pc = (Address)((int)this->pc + offset - 2);
        }

        // NOTE: this method advances the Program Counter (_pc) and writes to stdout
        void execute_next_instruction() {
            std::span<const Byte> memory_view{memory}; // read only accessor for memory
            Instruction instruction = Instruction::decode(pc, memory_view); // modifies pc in-place
            std::cout << std::string(this->call_stack.size() - 1, '>') << instruction.to_string();
            // XXX: this branching works for now when only 3 opcodes are implemented
            if (instruction.category == Instruction::Category::VAR and instruction.opcode == 0x0) { // call
                return this->opcode_call(instruction);
            } else if (instruction.category == Instruction::Category::_1OP) {
                if (instruction.opcode == 0xb) { // ret
                    return this->opcode_ret(instruction);
                } else if (instruction.opcode == 0xc) { // jump
                    return this->opcode_jump(instruction);
                }
            }
            // default:
            // XXX: no throw for now, will throw later on unimplemented instructions
            std::cout << " [WARN]: Instruction not implemented";
            // throw UnimplementedInstructionException();
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
        this->_impl->execute_next_instruction();
        std::cin.get();
    }
}
