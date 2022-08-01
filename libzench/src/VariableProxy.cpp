/*
 * This is a sample source file corresponding to a private header file.
 *
 * <Copyright information goes here>
 */

#include <cstddef>         // size_t

#include <deque>           // deque
#include <span>            // span

#include <zench/zench.hpp> // base library definitions of core types
#include "VariableProxy.hpp"

namespace {
    /*
     * Helper function, variadic xor
     * It works by counting the number of true values and returning true only
     * if this number is 1.
     */

    template <typename T>
    constexpr std::size_t only_one_helper(T t) {
        return (bool)t;
    }

    template <typename T, typename... Args>
    constexpr std::size_t only_one_helper(T t, Args... args) { // recursive variadic function
        return only_one_helper(t) + only_one_helper(args...);
    }

    template <typename T, typename... Args>
    constexpr bool only_one(T t, Args... args) { // recursive variadic function
        return only_one_helper(t, args...) == 1;
    }
}

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
    // init VariableProxy for stack pointer access
    VariableProxy::VariableProxy(std::deque<Word>& stack) : _stack(stack) {}
    // init VariableProxy for Word reference access
    VariableProxy::VariableProxy(Word& word) : _word(word) {}
    // init VariableProxy for global variable access
    VariableProxy::VariableProxy(std::span<Byte> memory, std::size_t base_addr)
      : _memory(memory)
      , _base_addr(base_addr)
      {}
    // malformed VariableProxy objects have multiple sources set
    bool VariableProxy::is_valid() const {
        return only_one(_memory.size() != 0, _word, _stack);
    }
    // assignment operator writes back to Variable, wherever it's stored
    VariableProxy& VariableProxy::operator=(Word w) {
        // guard against assignment when malformed
        if (not this->is_valid()) {
            throw Exception();
        }
        if (this->_stack) {
            // this long-winded access is due to optional<reference-wrapper<Word>>
            this->_stack.value().get().push_back(w);
        } else if (this->_word) {
            this->_word = w;
        } else {
            this->_memory[this->_base_addr] = w >> 8;
            this->_memory[this->_base_addr + 1] = w & 0x00ff;
        }
        return *this;
    }
    // cast-to-Word operator reads from Variable, wherever it's stored
    VariableProxy::operator Word() const {
        // guard against value access when malformed
        if (not this->is_valid()) {
            throw Exception();
        }
        if (this->_stack) {
            // this long-winded access is due to optional<reference-wrapper<Word>>
            auto stack = this->_stack.value().get();
            Word value = stack.back();
            stack.pop_back();
            return value;
        } else if (this->_word) {
            return this->_word.value();
        } else {
            return
                (this->_memory[this->_base_addr] << 8) +
                this->_memory[this->_base_addr + 1];
        }
        // XXX: unreachable
    }
}
