cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_COMPILER "/usr/bin/g++")
set(CMAKE_TOOLCHAIN_FILE "~/vcpkg/scripts/buildsystems/vcpkg.cmake")
project(Artemis VERSION 1.0 DESCRIPTION "Torret" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  MESSAGE("Clang")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  MESSAGE("GNU")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  MESSAGE("Intel")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  MESSAGE("MSVC")
endif()

set(VCPKG_PLATFORM_TOOLSET v142)

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_library(client src/client/client.cpp src/client/client.hpp)
include_directories(src/client)
target_link_libraries(client ${Boost_LIBRARIES})

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
include_directories(src/voice_recognition)
add_library(control  src/Controller_Input/serial_port.cpp src/Controller_Input/controller_linux.cpp src/Controller_Input/environment_manager.cpp src/voice_recognition/voice.cpp)
include_directories(src/Controller_Input)
target_link_libraries(control ${SDL2_LIBRARIES})

set(vision_DIR "src/face_recognition")
include_directories(${vision_DIR})
add_library(vision src/face_recognition/vision.cpp src/face_recognition/vision.hpp ${faces_src})

target_link_libraries(vision ${OpenCV_LIBS})
target_link_libraries(vision client)
target_link_libraries(vision control)

add_executable(main main.cpp)
target_link_libraries(main control)
target_link_libraries(main vision)


message(${OpenCV_INCLUDE_DIRS})
