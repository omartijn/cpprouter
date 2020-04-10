include(CheckCXXCompilerFlag)

function (target_supported_compile_options target)
    set(options BEFORE)
    set(arguments INTERFACE PUBLIC PRIVATE)
    cmake_parse_arguments(OPTION "${options}" "" "${arguments}" ${ARGN})

    foreach(compile_option ${OPTION_INTERFACE})
        # create the flag to use - CMake reuses the output variable
        # and will skip the check if the variable already exists
        string(REPLACE "-" "_" supported ${compile_option}_cxx)

        check_cxx_compiler_flag(${compile_option} ${supported})
        if (${${supported}})
            if (${OPTION_BEFORE})
                target_compile_options(${target} BEFORE INTERFACE ${compile_option})
            else()
                target_compile_options(${target} INTERFACE ${compile_option})
            endif()
        endif()
    endforeach(compile_option)

    foreach(compile_option ${OPTION_PUBLIC})
        # create the flag to use - CMake reuses the output variable
        # and will skip the check if the variable already exists
        string(REPLACE "-" "_" supported ${compile_option}_cxx)

        check_cxx_compiler_flag(${compile_option} ${supported})
        if (${${supported}})
            if (${OPTION_BEFORE})
                target_compile_options(${target} BEFORE PUBLIC ${compile_option})
            else()
                target_compile_options(${target} PUBLIC ${compile_option})
            endif()
        endif()
    endforeach(compile_option)

    foreach(compile_option ${OPTION_PRIVATE})
        # create the flag to use - CMake reuses the output variable
        # and will skip the check if the variable already exists
        string(REPLACE "-" "_" supported ${compile_option}_cxx)

        check_cxx_compiler_flag(${compile_option} ${supported})
        if (${${supported}})
            if (${OPTION_BEFORE})
                target_compile_options(${target} BEFORE PRIVATE ${compile_option})
            else()
                target_compile_options(${target} PRIVATE ${compile_option})
            endif()
        endif()
    endforeach(compile_option)

endfunction()
