cmake_minimum_required(VERSION 3.17)

# Use emscripten upstream by default if no toolchain supplied
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)

  if(DEFINED ENV{EMSCRIPTEN_ROOT_PATH})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSCRIPTEN_ROOT_PATH}/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "Emscripten toolchain file")
  elseif(DEFINED ENV{EMSDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" CACHE PATH "Emscripten toolchain file")
  else()
    message(FATAL_ERROR "Please define EMSDK to point to your SDK path!")
  endif()
endif()

project(SK83RJO.SH)

# Setup paths
get_filename_component(ROOT_PATH "." ABSOLUTE)
get_filename_component(ASSETS_PATH "assets" ABSOLUTE)
get_filename_component(SHELL_PATH "shell.html" ABSOLUTE)

# Setup CMAKE options
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ROOT_PATH}")

set(CMAKE_CXX_FLAGS "-Wall")
set(CMAKE_CXX_FLAGS_DEBUG "-g -g4")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Setup source
file(GLOB_RECURSE SRC CONFIGURE_DEPENDS
    "src/*.h"
    "src/*.cpp"
)

# Setup includes
include_directories(
    "src"
    "lib"
    "lib/Eigen"
    "lib/assimp/include"
)

# Setup libraries
add_subdirectory("lib")
link_libraries("assimp")

# Setup linker
add_link_options(
    "SHELL:-s WASM=1"
    "SHELL:-s USE_SDL=2"
    "SHELL:-s USE_SDL_IMAGE=2"
    "SHELL:-s FULL_ES3=1"
    "SHELL:-s OFFSCREENCANVAS_SUPPORT=1"
    "SHELL:-s OFFSCREEN_FRAMEBUFFER=1"
    "SHELL:-s MIN_WEBGL_VERSION=2"
    "SHELL:-s MAX_WEBGL_VERSION=2"
    "SHELL:-s ALLOW_MEMORY_GROWTH=1"
    "SHELL:--use-preload-plugins"
    "SHELL:--source-map-base http://localhost:8080/"
    "SHELL:--preload-file ${ASSETS_PATH}@assets/"
)

add_executable(index ${SRC})

# Setup asset handling
file(GLOB_RECURSE ASSETS CONFIGURE_DEPENDS
  "assets/*"
)
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
    COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp
    DEPENDS ${ASSETS}
)
add_library(assets STATIC ${CMAKE_CURRENT_BINARY_DIR}/assets.cpp)
add_dependencies(index assets)
