set(test-sources
    main.cpp
    path.cpp
    table.cpp
)

add_executable(test ${test-sources})
target_link_libraries(test router::router)
