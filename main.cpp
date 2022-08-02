#include <zench/FileSystem.hpp>
#include <zench/Keyboard.hpp>
#include <zench/Screen.hpp>
#include <zench/ZMachine.hpp>

using namespace com::saxbophone::zench;

// these stubs do nothing useful except provide a dummy implementation of the
// abstract classes so the code will compile. These will all be replaced in-turn
// with functional versions.
class StubFileSystem : public FileSystem {
public:
    virtual std::optional<InputFile> open_for_read() { return {}; }
    virtual std::optional<InputFile> open_for_read(char16_t filename[13]) { return {}; }
    virtual std::optional<OutputFile> open_for_write() { return {}; }
    virtual std::optional<OutputFile> open_for_write(char16_t filename[13]) { return {}; }
};
class StubScreen : public Screen {};
class StubKeyboard : public Keyboard {};

int main(int argc, const char* argv[]) {
    InputFile game;
    StubFileSystem fs;
    StubScreen screen;
    StubKeyboard keyboard;

    ZMachine vm(game, fs, screen, keyboard);
}
