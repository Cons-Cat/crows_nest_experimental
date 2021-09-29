cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
# TODO: Set compiler-dependant flags for stricter conformance.

project(crows_nest CXX)
add_compile_definitions(CMAKE_GAME_TITLE="Crow's Nest")
# TODO: Make CMAKE_DEBUG easily configurable.
# if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
add_compile_definitions(CMAKE_DEBUG)
# endif()

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(SDL2 REQUIRED FATAL_ERROR)
# find_package(fmt REQUIRED FATAL_ERROR)

add_executable(${CMAKE_PROJECT_NAME} src/main.cpp)

target_include_directories(
  crows_nest
  PUBLIC src/hpp/
  PUBLIC ${VULKAN_INCLUDE_DIRS}
  PUBLIC ${SDL2_INCLUDE_DIRS}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext)

target_link_libraries(
  crows_nest PRIVATE
  ${Vulkan_LIBRARIES}
  ${SDL2_LIBRARIES}
)

target_sources(
  crows_nest PRIVATE
  src/cpp/vk_globals.cpp
  src/cpp/window.cpp
  src/cpp/vk_init.cpp
  src/cpp/render_loop.cpp
  src/cpp/swapchain.cpp
  src/cpp/buffer.cpp
)
