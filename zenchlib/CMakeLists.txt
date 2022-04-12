include(GNUInstallDirs)

add_library(libzench STATIC)
set_target_properties(libzench PROPERTIES OUTPUT_NAME "zench")
# ALIAS target to export a namespaced target even when building in-tree
add_library(Zench::libzench ALIAS libzench)
# add source files
add_subdirectory(src)
# library public header files location
target_include_directories(
    libzench PUBLIC
    # different include directory path depending on if using the local or installed version of library
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# pass in version of project as preprocessor definitions
target_compile_definitions(
    libzench PUBLIC
    -DZENCH_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DZENCH_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DZENCH_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    -DZENCH_VERSION_STRING=${ZENCH_ESCAPED_VERSION_STRING}
)
# inherit common project compiler options
target_link_libraries(
    libzench
        PRIVATE
            $<BUILD_INTERFACE:zench-compiler-options>
)
