cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

project(nexus_chat_shorts)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -g")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -g")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif()

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

add_library(nexus_chat_shorts SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/clipboard.cpp)
target_link_libraries(nexus_chat_shorts PRIVATE imgui wininet)
set_property(TARGET nexus_chat_shorts PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
