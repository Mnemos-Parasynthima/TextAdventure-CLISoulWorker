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
    SaveLoad.c
    itoa.s
    DArray.c
    Misc.c
    Battle.c
    getopt.c
)

include_directories(headers)

enable_language(ASM)
set_source_files_properties(itoa.s PROPERTIES LANGUAGE ASM)

add_executable(${PROJECT_NAME} ${SOURCES})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)