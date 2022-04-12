/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zench/Public.hpp>

#include "Private.hpp"

namespace com::saxbophone::zench {
    bool library_works() {
        return PRIVATE::library_works();
    }
}
