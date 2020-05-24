set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

find_package(Runtime REQUIRED)

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
target_link_libraries(test cxx::runtime)
