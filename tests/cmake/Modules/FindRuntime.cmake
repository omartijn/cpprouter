#[=======================================================================[.rst:

FindRuntime
###########

This module finds required runtime libraries. Use the
:imp-target:`cxx::runtime` imported target to link to it.

This script will first try to link without specifying a
custom runtime, only if this fails will other runtimes
be tried. If no custom runtimes are required, linking to
:imp-target:`cxx::runtime` will not change linker flags.

#]=======================================================================]


if (TARGET cxx::runtime)
    # This module has already been processed. Don't do it again.
    return()
endif()

include(CMakePushCheckState)
include(CheckCXXSourceCompiles)

cmake_push_check_state()

# All of our tests require C++17 or later
set(CMAKE_CXX_STANDARD 17)

# the test script to compile
set(code [[
    #include <charconv>
    #include <cstddef>
    
    int main()
    {
        std::size_t number;
        std::from_chars("100", "100", number, 10);
    }
]])

# can we link in one way or another?
set(can_link TRUE)

# first check if we can link without specifying a custom runtime
check_cxx_source_compiles("${code}" CXX_RUNTIME_NO_LINK_NEEDED)

# can we compile without adding a runtime?
if (NOT CXX_RUNTIME_NO_LINK_NEEDED)
    # add the llvm runtime and retry the check
    # note: CMAKE_REQUIRED_LINK_OPTIONS would be better, but
    #       it's only supported in CMake 3.14+
    set(CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} -rtlib=compiler-rt -lgcc_s)

    # now compile again to see if we can now build our program
    check_cxx_source_compiles("${code}" CXX_RUNTIME_COMPILER_RT)

    # if linking fails again we simply cannot link
    set(can_link ${CXX_RUNTIME_COMPILER_RT})
endif()

cmake_pop_check_state()

# did we manage to link the example program
if (can_link)
    # create the target
    add_library(cxx::runtime INTERFACE IMPORTED)

    # do we need to link to clangs compiler runtime?
    if (CXX_RUNTIME_COMPILER_RT)
        # add the linker options
        target_link_options(cxx::runtime INTERFACE -rtlib=compiler-rt)
        target_link_libraries(cxx::runtime INTERFACE -lgcc_s)
    endif()
endif()

# if we can link we have found the runtime
set(Runtime_FOUND ${can_link} CACHE BOOL "TRUE if we can compile and link a program using cxx::runtime" FORCE)

# did we fail to find the required module?
if(Runtime_FIND_REQUIRED AND NOT Runtime_FOUND)
    message(FATAL_ERROR "Cannot Compile simple program using cxx::runtime")
endif()
