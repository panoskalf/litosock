cmake_minimum_required(VERSION 3.20)
project(litosock)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Help clangd get the intellisense right
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Prepare library - Platform-specific source selection
if(WIN32)
  message(STATUS "Building for Windows")
  add_library(LitoSock STATIC litosock_windows.cpp)
  # So examples can include litosock.h
  target_include_directories(LitoSock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
  target_link_libraries(LitoSock ws2_32)
else()
  message(STATUS "Building for Linux")
  add_library(LitoSock STATIC litosock_linux.cpp)
  target_include_directories(LitoSock PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Add subdirectories
add_subdirectory(examples)
