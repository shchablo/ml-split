################################################################################
# CMakeLists.txt for splitter-test.
# Shchablo Konstantin (Shchablo@gmail.com)
################################################################################

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

project(ml-test)


# PROJECT INCLUDES -------------------------------------------------------------
  include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
  include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
#-------------------------------------------------------------------------------

# PROJECT SRC ------------------------------------------------------------------
file(GLOB MAIN_SRC "*.cpp")
  file(GLOB SRC "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
  ##
  set(CPPS ${MAIN_SRC} ${SRC})
#-------------------------------------------------------------------------------

# COMPILER SETTING -------------------------------------------------------------

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
      message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. 
                      Please update your C++ compiler.")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_definitions(-Wall -O3)

add_executable(${PROJECT_NAME} ${CPPS})
# LIBRARIES
# --------------------------------------------------------------------
target_link_libraries(${PROJECT_NAME} pthread)
#-------------------------------------------------------------------------------
################################################################################
