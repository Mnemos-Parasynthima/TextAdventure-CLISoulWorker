cmake_minimum_required(VERSION 3.10)

project(clisw)

set(SOURCES
    cJSON.c
    getline.c
    main.c
    RoomTable.c
    Setup.c
    SoulWorker.c
    Maze.c
    Error.c
    Keyboard.c
)

include_directories(headers)

add_executable(${PROJECT_NAME} ${SOURCES})

add_definitions(-D_CRT_SECTURE_NO_WARNINGS)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

set(CMAKE_BUILD_TYPE Release)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)