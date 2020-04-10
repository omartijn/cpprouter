set(test-sources
    main.cpp
)

add_executable(test ${test-sources})
target_link_libraries(test router::router)
