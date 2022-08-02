/**
 * This is a sample private header file.
 *
 * <Copyright information goes here>
 */

#ifndef COM_SAXBOPHONE_ZENCH_VARIABLE_PROXY_HPP
#define COM_SAXBOPHONE_ZENCH_VARIABLE_PROXY_HPP

#include <cstddef>         // size_t

#include <deque>           // deque
#include <functional>      // reference_wrapper
#include <optional>        // optional
#include <span>            // span

#include <zench/zench.hpp> // base library definitions of core types

namespace com::saxbophone::zench {
    /*
     * VariableProxy allows covenient read/write access to Word-sized
     * variables that may exist in memory as arrays of Bytes.
     *
     * Assignment-operator and cast-to-Word operator are overloaded to allow
     * reading and writing to them as Words, which then writes the
     * corresponding Word (or Bytes if stored as two Bytes).
     *
     * Three kinds of Variable location are allowed:
     * - the top of the stack (stack pointer)
     * - a reference to a specific Word
     * - a global variable
     */
    class VariableProxy {
    private:
        // for global variables:
        std::span<Byte> _memory;
        std::size_t _base_addr;
        // for Word references:
        std::optional<std::reference_wrapper<Word>> _word;
        // for Stack Pointer:
        std::optional<std::reference_wrapper<std::deque<Word>>> _stack;
    public:
        // init VariableProxy for stack pointer access
        VariableProxy(std::deque<Word>& stack);
        // init VariableProxy for Word reference access
        VariableProxy(Word& word);
        // init VariableProxy for global variable access
        VariableProxy(std::span<Byte> memory, std::size_t base_addr);
        // malformed VariableProxy objects have multiple sources set
        bool is_valid() const;
        // assignment operator writes back to Variable, wherever it's stored
        VariableProxy& operator=(Word w);
        // cast-to-Word operator reads from Variable, wherever it's stored
        operator Word() const;
    };
}

#endif // include guard
