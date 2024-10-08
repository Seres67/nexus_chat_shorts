cmake_minimum_required(VERSION 3.18)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(BUILD_SHARED_LIBS OFF)
set(VCPKG_TARGET_TRIPLET "x64-windows-static")
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type configuration" FORCE)
    message(STATUS "Setting default build type: ${CMAKE_BUILD_TYPE}")
endif ()
if (UNIX)
    set(CMAKE_SYSTEM_NAME Windows)
    set(TOOLCHAIN_PREFIX x86_64-w64-mingw32)

    # cross compilers to use for C, C++ and Fortran
    set(PRIVATE_MINGW_PATH /usr/sbin)
    set(CMAKE_C_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-gcc)
    set(CMAKE_CXX_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-g++)
    set(CMAKE_Fortran_COMPILER ${PRIVATE_MINGW_PATH}/${TOOLCHAIN_PREFIX}-gfortran)
    set(CMAKE_RC_COMPILER /usr/bin/${TOOLCHAIN_PREFIX}-windres)

    # target environment on the build host system
    set(CMAKE_FIND_ROOT_PATH /usr/${TOOLCHAIN_PREFIX})

    # modify default behavior of FIND_XXX() commands
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -static -O2")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -O2")
endif ()

project(nexus_chat_shorts)

include_directories(include)

# load imgui as a cmake project
add_subdirectory(src/imgui)

find_package(cpr CONFIG REQUIRED)

add_library(nexus_chat_shorts SHARED src/main.cpp src/globals.cpp src/gui.cpp src/settings.cpp src/clipboard.cpp)
target_link_libraries(nexus_chat_shorts PRIVATE imgui cpr::cpr)
set_property(TARGET nexus_chat_shorts PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# copy dll to game directory
add_custom_command(TARGET nexus_chat_shorts
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:nexus_chat_shorts>
        "C:\\Games\\Guild Wars 2\\addons\\nexus_chat_shorts.dll.update"
        COMMENT "Copying nexus_chat_shorts.dll.update to game directory"
)
