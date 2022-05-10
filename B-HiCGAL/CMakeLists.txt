cmake_minimum_required(VERSION 2.8.12)
project(hi-cgal)

# 让find_package找到conan安装的包
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/build)  #添加到find_package module模式搜索路径
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_SOURCE_DIR}/build)  #添加到find_package config模式搜索路径

# CGAL
find_package(CGAL 
REQUIRED)

#
# 项目配置
# 

#工程公共头文件
set(PROJECT_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)  #工程头文件目录
include_directories(${PROJECT_INCLUDE_DIR})  #添加include目录（全局）
file(GLOB_RECURSE Project_Headers_Files ${PROJECT_INCLUDE_DIR}/*.h ${PROJECT_INCLUDE_DIR}/*.hpp)

#
# 创建子工程
# 
set(Project_Src_Dir ${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE files "${Project_Src_Dir}/*.cpp")
foreach(file ${files})
  #工程名
  get_filename_component(name ${file} NAME_WE)
  #工程文件夹
  STRING(REGEX REPLACE "${Project_Src_Dir}(.+)/.+" "\\1" DIR ${file})

  #创建工程 
  add_executable(${name} ${file} ${Project_Headers_Files})
  target_link_libraries(${name} CGAL::CGAL)

  # 子工程的文件夹
  set_property(TARGET ${name} PROPERTY FOLDER ${DIR})
  # 代码分组
  source_group(TREE ${CMAKE_SOURCE_DIR} FILES ${Project_Headers_Files})

  #数据路径
  target_compile_definitions(${name} 
  PRIVATE 
    "-DDATA_PATH=\"${CMAKE_SOURCE_DIR}/../data/\""
  )
endforeach()

#
# 其他配置
#

# 开启工程分组
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#关闭编译器优化
set(CMAKE_CXX_FLAGS "-fno-elide-constructors ${CMAKE_CXX_FLAGS}")