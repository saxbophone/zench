#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

using namespace com::saxbophone::zench;

int main(int argc, const char* argv[]) {
    InputFile game;
    FileSystem fs;
    Screen screen;
    Keyboard keyboard;

    ZMachine vm(game, fs, screen, keyboard);
}
