cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(raytracing CXX)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 3.3 REQUIRED FATAL_ERROR)

add_executable(raytracing src/main.cpp)

target_include_directories(
  raytracing
  PUBLIC src/hpp/
  PUBLIC ${VULKAN_INCLUDE_DIRS}
)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext)

target_link_libraries(
  raytracing PRIVATE
  WSIWindow
  ${Vulkan_LIBRARIES}
)

# target_compile_options(raytracing PRIVATE -fno-exceptions)

target_sources(
  raytracing PRIVATE
)
