include(GNUInstallDirs)

add_library(zygodactyl STATIC)
# ALIAS target to export a namespaced target even when building in-tree
add_library(Zygodactyl::zygodactyl ALIAS zygodactyl)
# add source files
add_subdirectory(src)
# library public header files location
target_include_directories(
    zygodactyl PUBLIC
    # different include directory path depending on if using the local or installed version of library
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
# pass in version of project as preprocessor definitions
target_compile_definitions(
    zygodactyl PRIVATE
    -DZYGODACTYL_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    -DZYGODACTYL_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    -DZYGODACTYL_VERSION_PATCH=${PROJECT_VERSION_PATCH}
    -DZYGODACTYL_VERSION_STRING=${ZYGODACTYL_ESCAPED_VERSION_STRING}
)
# set up version and soversion for the main library object
set_target_properties(
    zygodactyl PROPERTIES
    VERSION ${ZYGODACTYL_VERSION_STRING}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_Zygodactyl_MAJOR_VERSION ${PROJECT_VERSION_MAJOR}
    INTERFACE_Zygodactyl_MINOR_VERSION ${PROJECT_VERSION_MINOR}
)
set_property(
    TARGET zygodactyl
    APPEND PROPERTY COMPATIBLE_INTERFACE_STRING "${Zygodactyl_MAJOR_VERSION}.${Zygodactyl_MINOR_VERSION}"
)
# inherit common project compiler options
target_link_libraries(
    zygodactyl
        PRIVATE
            $<BUILD_INTERFACE:zygodactyl-compiler-options>
)

# install if we're not being built as a sub-project
if (NOT ZYGODACTYL_SUBPROJECT)
    # library
    install(
        TARGETS zygodactyl
        EXPORT ZygodactylTargets
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
        EXPORT ZygodactylTargets
        FILE ZygodactylTargets.cmake
        NAMESPACE Zygodactyl::
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zygodactyl"
    )
    # CMake package
    include(CMakePackageConfigHelpers)
    configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/ZygodactylConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zygodactyl"
    )
    # generate the version file for the config file
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/ZygodactylConfigVersion.cmake"
        VERSION "${version}"
        COMPATIBILITY SameMinorVersion
    )
    install(
        FILES
        "${CMAKE_CURRENT_BINARY_DIR}/ZygodactylConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/ZygodactylConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/Zygodactyl"
    )
endif()
