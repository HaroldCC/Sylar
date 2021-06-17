cmake_minimum_required(VERSION 3.10)

#set project name
project(sylar)

#重新定义当前目标的源文件的__FILE__宏
function(force_redefine_file_macro_for_sources targetname)
    #获取当前目标的所有源文件
    get_target_property(source_files "${targetname}" SOURCES)
    #遍历源文件
    foreach(sourcefile ${source_files})
        #获取当前源文件的编译参数
        get_property(defs SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS)
        #获取当前文件的绝对路径
        get_filename_component(filepath "${sourcefile}" ABSOLUTE)
        #将绝对路径中的项目路径替换成空,得到源文件相对于项目路径的相对路径
        string(REPLACE ${PROJECT_SOURCE_DIR}/ "" relpath ${filepath})
        #将我们要加的编译参数(__FILE__定义)添加到原来的编译参数里面
        list(APPEND defs "__FILE__=\"${relpath}\"")
        #重新设置源文件的编译参数
        set_property(
            SOURCE "${sourcefile}"
            PROPERTY COMPILE_DEFINITIONS ${defs}
            )
    endforeach()
endfunction()

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

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
force_redefine_file_macro_for_sources(sylar) 

add_executable(test tests/test_log.cpp)
force_redefine_file_macro_for_sources(test) 
add_dependencies(test sylar)
target_link_libraries(test sylar  ${YAMLCPP})

add_executable(test_config tests/test_config.cpp)
force_redefine_file_macro_for_sources(test_config) 
add_dependencies(test_config sylar)
target_link_libraries(test_config sylar ${YAMLCPP})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
