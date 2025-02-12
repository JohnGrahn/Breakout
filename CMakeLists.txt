cmake_minimum_required(VERSION 3.13)
project(Breakout)

# Ensure we're using Emscripten
if (NOT EMSCRIPTEN)
    message(FATAL_ERROR "This CMake file is designed to be used with Emscripten only")
endif()

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add source files
set(SOURCES
    src/main.cpp
)

# Add header files
set(HEADERS
    include/game.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)

# Emscripten-specific settings
set(EMSCRIPTEN_FLAGS
    "-s USE_GLFW=3"
    "-s WASM=1"
    "-s ASYNCIFY"
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    "--preload-file assets"
    "-O3"
)

# Link raylib
set(RAYLIB_FLAGS
    "-s USE_GLFW=3"
    "-s ASSERTIONS=1"
    "-s WASM=1"
    "-s NO_EXIT_RUNTIME=1"
    "-s ALLOW_MEMORY_GROWTH=1"
)

# Set output name
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "breakout"
    SUFFIX ".html"
)

# Configure emscripten linker flags
string(JOIN " " EMSCRIPTEN_LINK_FLAGS ${EMSCRIPTEN_FLAGS} ${RAYLIB_FLAGS})
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS ${EMSCRIPTEN_LINK_FLAGS})

# Link raylib (assuming it's built for Emscripten)
target_link_libraries(${PROJECT_NAME} PRIVATE raylib) 