include(GNUInstallDirs)

add_library(zench STATIC)
# ALIAS target to export a namespaced target even when building in-tree
add_library(Zench::zench ALIAS zench)
# add source files
add_subdirectory(src)
# library public header files location
target_include_directories(
    zench PUBLIC
    # different include directory path depending on if using the local or installed version of library
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# pass in version of project as preprocessor definitions
target_compile_definitions(
    zench PRIVATE
    -DZENCH_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DZENCH_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DZENCH_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    -DZENCH_VERSION_STRING=${ZENCH_ESCAPED_VERSION_STRING}
)
# set up version and soversion for the main library object
set_target_properties(
    zench PROPERTIES
    VERSION ${ZENCH_VERSION_STRING}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_Zench_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_Zench_MINOR_VERSION ${PROJECT_VERSION_MINOR}
)
set_property(
    TARGET zench
    APPEND PROPERTY COMPATIBLE_INTERFACE_STRING "${Zench_MAJOR_VERSION}.${Zench_MINOR_VERSION}"
)
# inherit common project compiler options
target_link_libraries(
    zench
        PRIVATE
            $<BUILD_INTERFACE:zench-compiler-options>
)

# install if we're not being built as a sub-project
if (NOT ZENCH_SUBPROJECT)
    # library
    install(
        TARGETS zench
        EXPORT ZenchTargets
        # when a static library
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        # when a shared library on UNIX
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        # when a DLL on Windows
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    # public headers
    install(
        DIRECTORY "include/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    # CMake export
    install(
        EXPORT ZenchTargets
        FILE ZenchTargets.cmake
        NAMESPACE Zench::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zench"
    )
    # CMake package
    include(CMakePackageConfigHelpers)
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/ZenchConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zench"
    )
    # generate the version file for the config file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ZenchConfigVersion.cmake"
        VERSION "${version}"
        COMPATIBILITY SameMinorVersion
    )
    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ZenchConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ZenchConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zench"
    )
endif()
