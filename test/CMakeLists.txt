cmake_minimum_required(VERSION 3.24)

project(test_vocabulary_builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME}
    main.cpp
    unittest.cpp
    utility/catch_formatters.hpp
)

target_compile_definitions(${PROJECT_NAME}
    PRIVATE TESTING
)

target_include_directories(${PROJECT_NAME}
    PRIVATE libs
            ../src
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE st
)

add_test(${PROJECT_NAME} ${PROJECT_NAME})
