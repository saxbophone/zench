#include <iostream>

#include <zench/StandardFileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

using namespace com::saxbophone::zench;

// prompts for filenames on the console
class ConsoleFilePicker : public StandardFilePicker {
public:
    virtual std::string get_filename() {
        std::cout << "Enter a filename: ";
        std::cout.flush();
        std::string filename;
        std::cin >> filename;
        return filename;
    }
};
class StubScreen : public Screen {};
class StubKeyboard : public Keyboard {};

int main(int argc, const char* argv[]) {
    InputFile game;
    ConsoleFilePicker picker;
    StandardFileSystem fs(picker);
    StubScreen screen;
    StubKeyboard keyboard;

    ZMachine vm(game, fs, screen, keyboard);
}
