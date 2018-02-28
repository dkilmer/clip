cmake_minimum_required(VERSION 3.6)
project(clip)

if(WIN32)
    # for Windows operating system in general
    message("-=-= cmake detected WIN32")
    link_directories("deps/chipmunk/lib/win32"
        "deps/portaudio/lib/win32"
        "deps/sdl2/lib/win32")
endif()

if(APPLE)
    # for MacOS X
    message("-=-= cmake detected APPLE")
    link_directories("deps/chipmunk/lib/macos"
        "deps/portaudio/lib/macos"
        "deps/sdl2/lib/macos")
endif()

if(UNIX AND NOT APPLE)
    # for Linux, BSD, Solaris, Minix
    message("-=-= cmake detected UNIX")
    set(GLAD_LIBRARIES dl m)
    link_directories("deps/chipmunk/lib/linux"
        "deps/portaudio/lib/linux"
        "deps/sdl2/lib/linux")
endif()

set(CHIPMUNK_LIBRARY chipmunk)
set(PORTAUDIO_LIBRARY portaudio)
set(SDL2_LIBRARY SDL2)

set(CMAKE_MACOSX_RPATH 1)

include_directories("deps/glad/include/"
        "deps/stb"
        "deps/portaudio/include/"
        "deps/chipmunk/include"
        "deps/sdl2/include")

#link_directories("/usr/local/lib")

file(GLOB DEPS_SOURCES deps/glad/src/glad.c)
file(GLOB PROJECT_HEADERS *.hpp *.h)
file(GLOB PROJECT_SOURCES *.cpp *.c)

source_group("include" FILES ${PROJECT_HEADERS})
source_group("shaders" FILES ${PROJECT_SHADERS})
source_group("src" FILES ${PROJECT_SOURCES})
source_group("deps" FILES ${DEPS_SOURCES})

add_definitions(-DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES}
        ${PROJECT_HEADERS}
        ${PROJECT_SHADERS}
        ${DEPS_SOURCES})

set_target_properties(${PROJECT_NAME} PROPERTIES
        LINKER_FLAGS "/ignore:4099")

target_link_libraries(${PROJECT_NAME} ${GLAD_LIBRARIES} ${CHIPMUNK_LIBRARY} ${SDL2_LIBRARY}
        ${PORTAUDIO_LIBRARY})
