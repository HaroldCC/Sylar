cmake_minimum_required(VERSION 3.10)

#set project name
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(/usr/local/lib)
link_directories(/usr/local/lib)

find_library(YAMLCPP libyaml-cpp)
message("***************************************",${YAMLCPP})

set(LIB_SRC
	sylar/log/log.cpp
	sylar/util/util.cpp
	sylar/config/config.cpp
	)

add_library(sylar SHARED ${LIB_SRC})

add_executable(test tests/test_log.cpp)
add_dependencies(test sylar)
target_link_libraries(test sylar  ${YAMLCPP})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar ${YAMLCPP})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
