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
)

include_directories(headers)

enable_language(ASM)
set_source_files_properties(itoa.s PROPERTIES LANGUAGE ASM)

add_executable(${PROJECT_NAME} ${SOURCES})

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(clisw-launcher launcher.c getopt.c)
target_include_directories(clisw-launcher PRIVATE ${CMAKE_SOURCE_DIR}/headers)
add_executable(clisw-installer installer.c getopt.c)
target_include_directories(clisw-installer PRIVATE ${CMAKE_SOURCE_DIR}/headers)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/data $<TARGET_FILE_DIR:${PROJECT_NAME}>/data)

set(PACKAGE_DIR "${CMAKE_BINARY_DIR}/CLISW")
set(PACKAGE_NAME "clisw_build.zip")

add_custom_target(package
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:clisw-launcher> ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/version ${PACKAGE_DIR}/
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/maps
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/misc
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PACKAGE_DIR}/data/story
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/maps ${PACKAGE_DIR}/data/maps
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/misc${PACKAGE_DIR}/data/isc
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/data/story ${PACKAGE_DIR}/data/story

    # Use external command for zipping
    COMMAND ${CMAKE_COMMAND} -E tar "cf" "${PACKAGE_NAME}" --format=zip ${PACKAGE_DIR}
    DEPENDS ${PROJECT_NAME} clisw-launcher
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)