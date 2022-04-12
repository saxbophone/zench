#include <fstream>
#include <iostream>
#include <string>

#include <zench/zench.hpp>
#include <zench/ZMachine.hpp>

int main(int argc, const char* argv[]) {
    using namespace com::saxbophone;
    std::cout << zench::VERSION_DESCRIPTION << std::endl;
    // XXX: basic version for now, just pull first arg off if given and use for filename
    if (argc > 1) {
        // try and open the story file
        std::ifstream story_file(argv[1], std::ios::binary);
        // construct new ZMachine instance from the open story file
        if (zench::ZMachine vm = zench::ZMachine(story_file)) { // run the vm if runnable
            while (vm.is_running()) {
                // XXX: may change in the future, but this is the basis
                vm.execute();
            }
        } else {
            std::cerr << "Unable to interpret story file" << std::endl;
            return -2;
        }
        return 0;
    } else {
        std::cerr << "No filename given" << std::endl;
        return -1;
    }
}
