cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(COMPILE_EXPORT_COMPILE_COMMANDS ON)

project(raytracing CXX)

option(DEBUG "Sanitizers, GDB symbols, etc." ON)

add_definitions(-DGLFW_INCLUDE_VULKAN)

find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 3.3 REQUIRED FATAL_ERROR)

add_executable(raytracing src/main.cpp)

target_include_directories(raytracing PUBLIC
  PUBLIC src/hpp/
  PUBLIC ${VULKAN_INCLUDE_DIRS}
  )

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/ext)

target_link_libraries(raytracing PUBLIC
  glfw PRIVATE
  stx PRIVATE
  ${Vulkan_LIBRARIES}
  )

target_compile_options(raytracing PUBLIC
    # C++ features
    -fno-exceptions -fno-rtti
    )
  
if (DEBUG)  
  set(CMAKE_EXE_LINKER_FLAGS "-fno-omit-frame-pointer -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow")

  target_link_libraries(raytracing PUBLIC
    rt PRIVATE
    )

  target_compile_options(raytracing PUBLIC
    # Debugging
    -ggdb -fno-omit-frame-pointer
    # Warnings
    #-Wpedantic -Wall -Wextra
    # Runtime checks
    # -fsanitize=address
    -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow
    )
else()
  add_link_options(raytracing PUBLIC
    -Wl
    )

  target_compile_options(raytracing PUBLIC
    # Optimize out unused functions
    -ffunction-sections -fdata-sections
    # Etc.
    -fomit-frame-pointer -fno-strict-aliasing
    -march=native
	-s
    )
endif()

target_sources(raytracing PRIVATE
  src/main.cpp
  src/hpp/app.hpp
  src/cpp/app.cpp
  src/hpp/memory.hpp
  src/cpp/memory.cpp
  )
