/**
 * This is a sample private header file.
 *
 * <Copyright information goes here>
 */

#ifndef COM_SAXBOPHONE_ZENCH_INSTRUCTION_HPP
#define COM_SAXBOPHONE_ZENCH_INSTRUCTION_HPP

#include <optional>  // optional
#include <span>      // span
#include <string>    // string
#include <vector>    // vector

#include <zench/zench.hpp>

namespace com::saxbophone::zench {
    struct Instruction {
    public:
        // NOTE: modifies pc in-place!
        static Instruction decode(Address& pc, std::span<const Byte> memory_view);

        std::string to_string() const;

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

        Opcode opcode;
        Form form; // form specifies the structure of an instruction
        Category category; // categories are rather misleadingly named after their operand counts, a trait that doesn't quite hold true
        std::vector<Operand> operands;
        std::optional<Byte> store_variable;
        std::optional<Branch> branch;
        std::optional<std::span<const Byte>> trailing_string_literal;
        // strictly metadata fields for assembly output:
        Address location; // address of the first byte of this instruction
        std::span<const Byte> bytecode; // the raw bytes that encode this instruction
    private:
        static void _determine_opcode_type(
            Address& pc,
            std::span<const Byte> memory_view,
            Instruction& instruction
        );
        static void _read_in_operand_values(
            Address& pc,
            std::span<const Byte> memory_view,
            Instruction& instruction
        );
        bool _is_instruction_store() const;
        bool _is_instruction_branch() const;
        static void _handle_branch(
            Address& pc,
            std::span<const Byte> memory_view,
            Instruction& instruction
        );
        bool _has_string_literal() const;
        static void _handle_string_literal(
            Address& pc,
            std::span<const Byte> memory_view,
            Instruction& instruction
        );
        std::string _address_string() const;
        std::string _get_2op_name() const;
        std::string _get_1op_name() const;
        std::string _get_0op_name() const;
        std::string _get_var_name() const;
        std::string _get_ext_name() const;
        std::string _mnemonic_string() const;
        std::string _arguments_string() const;
        std::string _literal_string() const;
        std::string _store_string() const;
        std::string _branch_string() const;
        std::string bytecode_string() const;
        std::string _metadata() const;
    };
}

#endif // include guard
