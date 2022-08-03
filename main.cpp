/*
 * This file forms part of libzench
 * libzench is a software library that implements a portable and extensible
 * Z-machine interpreter, designed to be embedded within other programs.
 *
 * Created by Joshua Saxby <joshua.a.saxby@gmail.com>, May 2022
 *
 * Copyright Joshua Saxby <joshua.a.saxby@gmail.com> 2022
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <iostream>

#include <zench/StandardFileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

using namespace com::saxbophone::zench;

// prompts for filenames on the console
class ConsoleFilePicker : public StandardFilePicker {
public:
    constexpr const char* name() override {
        return "ConsoleFilePicker";
    }
    std::string get_filename() override {
        std::cout << "Enter a filename: ";
        std::cout.flush();
        std::string filename;
        std::cin >> filename;
        return filename;
    }
};
class StubScreen : public Screen {
public:
    constexpr const char* name() override {
        return "StubScreen";
    }
};
class StubKeyboard : public Keyboard {
public:
    constexpr const char* name() override {
        return "StubKeyboard";
    }
    constexpr bool supports_mouse() override { return false; }
    constexpr bool supports_menus() override { return false; }
    std::vector<Event> get_input() override { return {}; }
};

int main(int argc, const char* argv[]) {
    // TODO: load up story file properly
    StandardFileSystem::InputFile game;
    ConsoleFilePicker picker;
    StandardFileSystem fs(picker);
    StubScreen screen;
    StubKeyboard keyboard;

    ZMachine vm(game, fs, screen, keyboard);

    while (vm.is_ready()) {
        vm.execute();
    }
}
