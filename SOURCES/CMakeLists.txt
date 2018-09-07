cmake_minimum_required(VERSION 3.10)
project(compression-utility)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -std=c++17 ")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_GLIBCXX_DEBUG -DDEBUG")
  set(CMAKE_CXX_FLAGS_COVERAGE "-g --coverage")
elseif (MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /permissive- /D_HAS_TR1_NAMESPACE /D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING ")
  include_directories(.)
  add_subdirectory(gtest)
  include_directories(${CMAKE_SOURCE_DIR}/gtest)
  set(LINK_FLAGS "-pthread ${LINK_FLAGS}")
endif()


add_executable(compression-utility utility.cpp)
add_executable(compression-testing testing.cpp)

add_library(compression compression.cpp)

target_link_libraries(compression-utility compression)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  target_link_libraries(compression-testing compression -lgtest)
elseif (MSVC)
  target_link_libraries(compression-testing compression gtest)
endif()

