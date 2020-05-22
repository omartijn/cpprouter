set(test-sources
    main.cpp
    path.cpp
    slug.cpp
    table.cpp
    variables.cpp
    tuple_slice.cpp
)

add_executable(test ${test-sources})
target_link_libraries(test router::router)
