cmake_minimum_required(VERSION 2.8.12)
project(cgal-examples)

#
# 全局置为C++17
#   https://cmake.org/cmake/help/latest/prop_tgt/CXX_STANDARD.html
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#
# 让find_package找到conan安装的包
#
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build)  #添加到find_package module模式搜索路径
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/build)  #添加到find_package config模式搜索路径

#
# CGAL
#
find_package(CGAL REQUIRED)

#
# 项目配置
# 

set(DataDir "${CMAKE_SOURCE_DIR}/data/")

#工程公共头文件
set(CommonIncludeDir ${CMAKE_SOURCE_DIR}/include)  #工程头文件目录
include_directories(${CommonIncludeDir})           #添加include目录（全局）
file(GLOB_RECURSE CommonHeadersFiles ${CommonIncludeDir}/*.h ${CommonIncludeDir}/*.hpp)

#
# 子工程
#
add_subdirectory(examples)

#
# 其他配置
#

# 代码分组
source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${CommonHeadersFiles})
# 开启工程分组
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
# 关闭编译器优化
set(CMAKE_CXX_FLAGS "-fno-elide-constructors ${CMAKE_CXX_FLAGS}")