cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project(PtP_Chat LANGUAGES C)

file(GLOB PtP_Sources
    src/*.h
    src/*.c
)

add_executable(PtP_Chat ${PtP_Sources})

if(WIN32)
	target_link_libraries(PtP_Chat ws2_32)
else()
    target_link_libraries(PtP_Chat pthread)
endif()
target_link_libraries(PtP_Chat m)

target_include_directories(PtP_Chat PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(PtP_Chat PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
