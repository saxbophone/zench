/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <string>    // string

#include <zench/zench.hpp>

namespace com::saxbophone::zench {
    const std::string VERSION = ZENCH_VERSION_STRING;
    const std::string VERSION_DESCRIPTION = "zench v" ZENCH_VERSION_STRING;
}
