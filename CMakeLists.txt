cmake_minimum_required(VERSION 3.5)

project(vocabulary_builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

add_subdirectory(libs/sqlite_database)
add_subdirectory(libs/string_essentials)
add_subdirectory(libs/tiled_ncurses)

set(MM_SOURCE_FILES
    src/utility/apple_script.mm
)

add_executable(${PROJECT_NAME}
    src/app.cpp
    src/app.h
    src/card.cpp
    src/card.h
    src/card_model.cpp
    src/card_model.h
    src/config.cpp
    src/config.h
    src/global.h
    src/main.cpp
    src/utility/anki_client.cpp
    src/utility/anki_client.h
    src/utility/apple_script.h
    src/utility/curl_request.cpp
    src/utility/curl_request.h
    src/utility/speech_engine.cpp
    src/utility/speech_engine.h
    src/utility/tools.cpp
    src/utility/tools.h
    ${MM_SOURCE_FILES}
)

set_source_files_properties(${MM_SOURCE_FILES}
    PROPERTIES COMPILE_FLAGS "-x objective-c++"
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ./
)

target_link_libraries(${PROJECT_NAME}
    string_essentials
    tiled_ncurses
    sqlite_database
    curl
    "-framework ApplicationServices"
    "-framework Foundation"
)

install (TARGETS
    ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)
