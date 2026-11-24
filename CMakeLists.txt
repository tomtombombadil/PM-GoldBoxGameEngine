# Gold Box Game Engine for PortMaster: the CYD engine's game code (the cyd/
# submodule) with an SDL2 front end.
cmake_minimum_required(VERSION 3.16)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" PM_VERSION LIMIT_COUNT 1)
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/cyd/VERSION" CYD_VERSION LIMIT_COUNT 1)
project(goldbox VERSION ${PM_VERSION} LANGUAGES CXX)

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cyd/src/game/play.cpp")
    message(FATAL_ERROR "The engine isn't here: run  git submodule update --init")
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

find_package(SDL2 REQUIRED)
find_package(Threads REQUIRED)

# The engine: plain C++ (cyd/src/engine), the games (cyd/src/game/play.cpp;
# its .inc files are part of it). The boards' own front end and drivers
# (cyd/src/ui, hal, boards, app/*.cpp) are not built here.
file(GLOB ENGINE_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/cyd/src/engine/*.cpp")
add_library(gbengine STATIC ${ENGINE_SOURCES} cyd/src/game/play.cpp cyd/src/game/companion.cpp
    cyd/src/game/music_make.cpp cyd/src/game/tidy.cpp src/compat.cpp)
target_include_directories(gbengine PUBLIC cyd/src)
# (src/compat.h first in every engine file: strlcpy / strlcat of our own)
target_compile_options(gbengine PRIVATE -O2 -Wno-format-truncation -Wno-stringop-overflow
    -include "${CMAKE_CURRENT_SOURCE_DIR}/src/compat.h")

add_executable(goldbox
    src/main.cpp src/video.cpp src/library.cpp src/settings.cpp
    src/platform_sdl.cpp src/audio_sdl.cpp src/task.cpp src/art.cpp src/panel.cpp src/jour.cpp src/touchpad.cpp src/shell.cpp)
target_compile_definitions(goldbox PRIVATE PM_VERSION="${PM_VERSION}" CYD_VERSION="${CYD_VERSION}")
target_compile_options(goldbox PRIVATE -Wall -Wextra)
if(TARGET SDL2::SDL2)
    target_link_libraries(goldbox PRIVATE gbengine SDL2::SDL2 Threads::Threads)
else()
    target_include_directories(goldbox PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(goldbox PRIVATE gbengine ${SDL2_LIBRARIES} Threads::Threads)
endif()
# (the handhelds' firmwares differ in their C++ runtime: carry ours)
option(PM_STATIC_RUNTIME "Link libstdc++ / libgcc statically" ON)
if(PM_STATIC_RUNTIME)
    target_link_options(goldbox PRIVATE -static-libstdc++ -static-libgcc)
endif()

# Claude's check tool (not in the package): what each made journal holds
add_executable(jlist EXCLUDE_FROM_ALL tools/jlist.cpp src/library.cpp src/platform_sdl.cpp src/audio_sdl.cpp)
target_include_directories(jlist PRIVATE src)
if(TARGET SDL2::SDL2)
    target_link_libraries(jlist PRIVATE gbengine SDL2::SDL2 Threads::Threads)
else()
    target_include_directories(jlist PRIVATE ${SDL2_INCLUDE_DIRS})
    target_link_libraries(jlist PRIVATE gbengine ${SDL2_LIBRARIES} Threads::Threads)
endif()
