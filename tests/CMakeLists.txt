set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")

find_package(Runtime REQUIRED)
add_subdirectory(Catch2)

set(test-sources
    path.cpp
    slug.cpp
    table.cpp
    variables.cpp
    tuple_slice.cpp
    proxy_table.cpp
)

add_executable(test ${test-sources})
target_link_libraries(test router::router)
target_link_libraries(test cxx::runtime)
target_link_libraries(test Catch2::Catch2WithMain)
