cmake_minimum_required(VERSION 3.0)

set(COMMON_COMPILE_FLAGS "-Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_COMPILE_FLAGS} -std=c17")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_COMPILE_FLAGS} -std=c++17")

if(CMAKE_DEBUG_BUILD)
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG -D_DEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -DDEBUG -D_DEBUG")
else()
    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native -DNDEBUG")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -march=native -DNDEBUG")
endif()

add_executable(raytrace
    src/app/app.cpp
    src/app/device.cpp
    src/app/instance.cpp
    src/app/shader.cpp
    src/app/util.cpp
    src/app/window.cpp
    src/main.cpp
)

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${CMAKE_BINARY_DIR}/include
)

target_link_libraries(raytrace
    glfw
    vulkan
)
