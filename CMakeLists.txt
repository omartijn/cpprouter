cmake_minimum_required(VERSION 3.10)
project(router
    VERSION     0.0.1
    LANGUAGES   CXX
)

add_library(router INTERFACE)
add_library(router::router ALIAS router)

target_compile_features(router INTERFACE cxx_std_17)

target_include_directories(router INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

install(
    DIRECTORY   include/router
    DESTINATION include
)

install(
    FILES       include/router.h
    DESTINATION include
)

install(
    TARGETS     router
    EXPORT      router-targets
    DESTINATION lib
)

install(
    EXPORT      router-targets
    NAMESPACE   router::
    DESTINATION lib/cmake/router
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/router/router-config-version.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(
    EXPORT      router-targets
    FILE        "${CMAKE_CURRENT_BINARY_DIR}/router/router-targets.cmake"
    NAMESPACE   router::
)

configure_file(cmake/router-config.cmake
    "${CMAKE_CURRENT_BINARY_DIR}/router/router-config.cmake"
    COPYONLY
)

install(
    FILES
        cmake/router-config.cmake
        "${CMAKE_CURRENT_BINARY_DIR}/router/router-config-version.cmake"
    DESTINATION
        lib/cmake/router
)

add_subdirectory(tests)
