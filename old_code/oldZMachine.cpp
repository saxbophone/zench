/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <cstddef>    // size_t

#include <algorithm>  // clamp
#include <bitset>     // bitset
#include <deque>      // deque
#include <functional> // equal_to, less, greater
#include <iostream>   // XXX: debug
#include <istream>    // istream
#include <iterator>   // istreambuf_iterator
#include <memory>     // unique_ptr
#include <optional>   // optional
#include <span>       // span
#include <vector>     // vector

#include <zench/zench.hpp>
#include <zench/ZMachine.hpp>

#include "Instruction.hpp"
#include "VariableProxy.hpp"

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

        ByteAddress globals_address; // global variables start here

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

        // returning a VariableProxy for the word allows read-write of Words into memory Bytes!
        VariableProxy load_word(Address address) {
            return VariableProxy(memory, address);
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
            // global variables base address is given in Word 6 (the 7th Word)
            globals_address = this->load_word(0x0c);
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
        // use VariableProxy object to get a read/write handle to the variable
        VariableProxy get_variable(Byte number) {
            if (number == 0x00) { // stack pointer
                return VariableProxy(this->call_stack.back().local_stack);
            } else if (0x01 <= number and number <= 0x0f) { // locals = 0x01..0x0f
                return VariableProxy(this->call_stack.back().local_variables[number - 1]);
            } else { // globals = 0x10..0xff
                return VariableProxy(memory, globals_address + number - 0x10u);
            }
        }
        // TODO: local stack manipulation

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
                return get_variable(operand.byte);
            default:
                throw Exception(); // ERROR! type can't be OMITTED!
            }
        }

        // uses variable references rather than variable values
        VariableProxy get_variable_referenced_by(const Instruction::Operand& reference) {
            Word address = this->operand_value(reference);
            // address should be in range 0x00..0xFF --error if not
            if (address > 0xFF) {
                throw Exception();
            }
            // now, fetch a proxy to the actual variable referenced by the address
            return get_variable((Byte)address);
        }

        void opcode_call(const Instruction& instruction) {
            // must have 1..4 operands --routine address + 0..3 arguments
            if (not (0 < instruction.operands.size() and instruction.operands.size() <= 4)) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // construct a new StackFrame for this routine, populated appropriately
            Address routine_address = expand_packed_address(operand_value(instruction.operands[0]));
            // handle special case: call address 0 returns false (0)
            if (routine_address == 0) {
                get_variable(instruction.store_variable.value()) = 0;
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

        // this executes the common "return value and pop the call stack" part of all return instructions
        void return_value(Word value) {
            // preserve return value variable number
            Byte store_variable = this->call_stack.back().result_ref;
            // move pc to return address
            this->pc = this->call_stack.back().return_pc;
            // pop the stack
            this->call_stack.pop_back();
            // set result variable
            get_variable(store_variable) = value;
        }

        void opcode_ret(const Instruction& instruction) {
            // return operand value
            this->return_value(this->operand_value(instruction.operands[0]));
        }

        void opcode_jump(const Instruction& instruction) {
            // the jump address is a 2-byte signed offset to apply to the PC
            SWord offset = (SWord)instruction.operands[0].word;
            /*
             * The destination of the jump opcode is:
             * Address after instruction + Offset - 2
             */
            this->pc = (Address)((int)this->pc + offset - 2);
        }

        void opcode_rtrue() {
            this->return_value(1); // true=1
        }

        void opcode_rfalse() {
            this->return_value(0); // false=0
        }

        void opcode_print_ret() {
            // TODO: print the quoted (literal) Z-str
            // TODO: print a newline
            // return true
            this->opcode_rtrue();
        }

        void opcode_ret_popped() {
            // pop top of stack and return that
            this->return_value(get_variable(0x00)); // SP = 0x00
        }

        // helper method for all conditional-jump opcodes
        // NOTE: can't be used with unconditional jump as that has no special
        // meaning for offsets 0 and 1, unlike conditional jumps, which return
        // true or false in those cases.
        void jump_with_offset(SWord offset) {
            // special cases are offsets 0 and 1, handle them first
            switch (offset) {
            // XXX: could optimise into this->return_value(offset) because 0=false and 1=true
            case 0:
                return this->opcode_rfalse(); // return false from current routine
            case 1:
                return this->opcode_rtrue(); // return true from current routine
            default: // otherwise
                // new address = address after branch data + offset - 2
                // pc is already at "address after branch data", so thus:
                this->pc = (Address)((int)this->pc + offset - 2);
                return;
            }
        }

        void opcode_je(const Instruction& instruction) {
            // je with just 1 operand is not permitted
            if (instruction.operands.size() < 2) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // jump if first operand is equal to any subsequent operands
            bool equal = false;
            Word first = this->operand_value(instruction.operands[0]);
            for (std::size_t i = 1; i < instruction.operands.size(); i++) {
                Word value = this->operand_value(instruction.operands[i]);
                if (first == value) {
                    equal = equal or true;
                    // XXX: clarify whether stack pointer should always be popped
                    // if an argument, even if it's not needed because equality
                    // to a previous operand was confirmed before it was reached
                    // if it doesn't need to always be popped if never reached,
                    // we can put back in the break, otherwise, we need to check
                    // every operand even if we know we already need to jump
                    // (otherwise, stack will sometimes be popped, sometimes not)
                    // break;
                }
            }
            // obey branch instruction's on-true/on-false specifier
            if (equal == instruction.branch->on_true) {
                SWord branch_offset = instruction.branch->offset;
                return this->jump_with_offset(branch_offset);
            }
        }

        // executes conditional jump for jump-if-less/jump-if-greater
        // use Compare to specify which kind of comparison to make
        template <class Compare>
        void conditional_jump(const Instruction& instruction) {
            // must have 2 operands only
            if (instruction.operands.size() != 2) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // comparison is *signed*
            SWord lhs = (SWord)this->operand_value(instruction.operands[0]);
            SWord rhs = (SWord)this->operand_value(instruction.operands[1]);
            // obey branch instruction's on-true/on-false specifier
            if (Compare{}(lhs, rhs) == instruction.branch->on_true) {
                SWord branch_offset = instruction.branch->offset;
                return this->jump_with_offset(branch_offset);
            }
        }

        void opcode_jz(const Instruction& instruction) {
            // jump if zero (also obey on-true/on-false specifier)
            bool is_zero = this->operand_value(instruction.operands[0]) == 0;
            if (is_zero == instruction.branch->on_true) {
                SWord branch_offset = instruction.branch->offset;
                return this->jump_with_offset(branch_offset);
            }
        }

        void opcode_pop() {
            // throw top value of stack away
            this->call_stack.back().local_stack.pop_back();
        }

        void opcode_push(const Instruction& instruction) {
            // must have 1 operand only --the value to push
            if (instruction.operands.size() != 1) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // push the only operand onto the local stack
            this->call_stack.back().local_stack.push_back(this->operand_value(instruction.operands[0]));
        }

        void opcode_pull(const Instruction& instruction) {
            // must have 1 operand only --the address of the variable to pull into
            if (instruction.operands.size() != 1) {
                throw WrongNumberOfInstructionOperandsException();
            }
            get_variable_referenced_by(instruction.operands[0]) = this->call_stack.back().local_stack.back();
            // finally, pop the top of the stack
            return this->opcode_pop();
        }

        void opcode_store(const Instruction& instruction) {
            // must have 2 operands only
            if (instruction.operands.size() != 2) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // set the variable referenced by the first operand to the value of the second
            get_variable_referenced_by(instruction.operands[0]) = this->operand_value(instruction.operands[1]);
        }

        void opcode_load(const Instruction& instruction) {
            // The value of the variable referred to by the operand is stored in the result.
            get_variable(instruction.store_variable.value()) = get_variable_referenced_by(instruction.operands[0]);
        }

        void opcode_storeb(const Instruction& instruction) {
            // must have three operands --array, byte_index and value
            if (instruction.operands.size() != 3) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // gather operands
            ByteAddress array = this->operand_value(instruction.operands[0]);
            ByteAddress byte_index = this->operand_value(instruction.operands[1]);
            Word value = this->operand_value(instruction.operands[2]);
            // calculate absolute address of Byte to store
            ByteAddress address = array + byte_index; // may overflow, ignore
            // write byte as long as address is in range of dynamic memory
            if (address < this->writeable_memory.size()) {
                // TODO: whitelist write access to header bytes!
                this->writeable_memory[address] = (Byte)value;
            }
        }

        void opcode_loadb(const Instruction& instruction) {
            // must have two operands --array and byte_index
            if (instruction.operands.size() != 2) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // gather operands
            ByteAddress array = this->operand_value(instruction.operands[0]);
            ByteAddress byte_index = this->operand_value(instruction.operands[1]);
            // calculate absolute address of Byte to load
            ByteAddress address = array + byte_index; // may overflow, ignore
            // read Byte as long as address is in range of static or dynamic memory
            if (address < this->readable_memory.size()) {
                // store byte in result
                get_variable(instruction.store_variable.value()) = this->readable_memory[address];
            }
        }

        void opcode_storew(const Instruction& instruction) {
            // must have three operands --array, word_index and value
            if (instruction.operands.size() != 3) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // gather operands
            ByteAddress array = this->operand_value(instruction.operands[0]);
            ByteAddress word_index = this->operand_value(instruction.operands[1]);
            Word value = this->operand_value(instruction.operands[2]);
            // calculate absolute address of Word to store
            ByteAddress address = array + 2 * word_index; // may overflow, ignore
            // validate if address is in range of writeable memory
            if (address < this->writeable_memory.size()) {
                // TODO: whitelist write access to header bytes!
                this->load_word(address) = value; // assign through proxy
            }
        }

        void opcode_loadw(const Instruction& instruction) {
            // must have two operands --array and word_index
            if (instruction.operands.size() != 2) {
                throw WrongNumberOfInstructionOperandsException();
            }
            // gather operands
            ByteAddress array = this->operand_value(instruction.operands[0]);
            ByteAddress word_index = this->operand_value(instruction.operands[1]);
            // calculate absolute address of Word to load
            ByteAddress address = array + word_index; // may overflow, ignore
            // read Word as long as address is in range of static or dynamic memory
            if (address < this->readable_memory.size()) {
                // store Word in result
                get_variable(instruction.store_variable.value()) = this->load_word(address);
            }
        }

        // NOTE: this method advances the Program Counter (_pc) and writes to stdout
        void execute_next_instruction() {
            std::span<const Byte> memory_view{memory}; // read only accessor for memory
            Instruction instruction = Instruction::decode(pc, memory_view); // modifies pc in-place
            std::cout << std::string(this->call_stack.size() - 1, '>') << instruction.to_string();
            // XXX: this branching works for now when only 3 opcodes are implemented
            switch (instruction.category) {
            case Instruction::Category::VAR:
                switch (instruction.opcode) {
                case 0x0: // call
                    return this->opcode_call(instruction);
                case 0x1: // storew
                    return this->opcode_storew(instruction);
                case 0x2: // storeb
                    return this->opcode_storeb(instruction);
                case 0x8: // push
                    return this->opcode_push(instruction);
                case 0x9: // pull
                    return this->opcode_pull(instruction);
                default:
                    goto unimplemented;
                }
            case Instruction::Category::_2OP:
                switch (instruction.opcode) {
                case 0x01: // je
                    return this->opcode_je(instruction);
                case 0x02: // jl
                    return this->conditional_jump<std::less<SWord>>(instruction);
                case 0x03: // jg
                    return this->conditional_jump<std::greater<SWord>>(instruction);
                case 0x0d: // store
                    return this->opcode_store(instruction);
                case 0x0f: // loadw
                    return this->opcode_loadw(instruction);
                case 0x10: // loadb
                    return this->opcode_loadb(instruction);
                default:
                    goto unimplemented;
                }
            case Instruction::Category::_1OP:
                switch (instruction.opcode) {
                case 0x0: // jz
                    return this->opcode_jz(instruction);
                case 0xb: // ret
                    return this->opcode_ret(instruction);
                case 0xc: // jump
                    return this->opcode_jump(instruction);
                case 0xe: // load
                    return this->opcode_load(instruction);
                default:
                    goto unimplemented;
                }
            case Instruction::Category::_0OP:
                switch (instruction.opcode) {
                case 0x0: // rtrue
                    return this->opcode_rtrue();
                case 0x1: // rfalse
                    return this->opcode_rfalse();
                case 0x3: // print_ret
                    return this->opcode_print_ret();
                case 0x8: // ret_popped
                    return this->opcode_ret_popped();
                case 0x9: // pop
                    return this->opcode_pop();
                default:
                    goto unimplemented;
                }
            default:
                // actually an error --unknown/unsupported instruction category
                throw Exception();
            }
        unimplemented:
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
