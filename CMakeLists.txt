cmake_minimum_required(VERSION 3.5)

project(vocabulary_builder LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_subdirectory(libs/sqlite_database)
add_subdirectory(libs/string_essentials)
add_subdirectory(libs/tiled_ncurses)

if (APPLE)
    set(FRAMEWORKS
        "-framework ApplicationServices"
        "-framework Foundation"
    )
    set(APPLE_SOURCES
        src/utility/apple_script.h
        src/utility/apple_script.mm
        src/utility/speech_engine_mac.cpp
    )
    set_source_files_properties(
        src/utility/apple_script.mm
        PROPERTIES COMPILE_FLAGS "-x objective-c++"
    )
else()
    set(FRAMEWORKS "")
    set(APPLE_SOURCES src/utility/speech_engine_none.cpp)
endif()

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
    src/utility/curl_request.cpp
    src/utility/curl_request.h
    src/utility/speech_engine.h
    src/utility/tools.cpp
    src/utility/tools.h
    ${APPLE_SOURCES}
)

target_include_directories(${PROJECT_NAME}
    PRIVATE ./
            ${CURL_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    string_essentials
    tiled_ncurses
    sqlite_database
    ${CURL_LIBRARIES}
    ${FRAMEWORKS}
)

target_compile_options(${PROJECT_NAME}
    PUBLIC -march=native
           -fno-rtti
           -fPIE
           -fstack-protector-all
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT ${PROJECT_NAME}
)
