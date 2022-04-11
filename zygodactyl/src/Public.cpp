/*
 * This is a sample source file corresponding to a public header file.
 *
 * <Copyright information goes here>
 */

#include <zygodactyl/Public.hpp>

#include "Private.hpp"

namespace com::saxbophone::zygodactyl {
    bool library_works() {
        return PRIVATE::library_works();
    }
}
