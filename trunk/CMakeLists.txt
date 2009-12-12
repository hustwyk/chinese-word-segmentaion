cmake_minimum_required (VERSION 2.6)

project (glsegmentor)

set (CMAKE_CXX_FLAGS "-O3 -std=c++0x -fshort-wchar")
set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_CXX_FLAGS -fpermissive)
set (CMAKE_EXE_LINKER_FLAGS  -lgtest) #for google test

#set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g -ggdb")
#set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

#cmake -DDEFINE_DEBUG=ON 
#现在还没有完全弄清楚
#1.首先ccmake ..可以设定是debug还是release版本
#但是我设定debug后,并不能用#ifdef DEBUG,也就是它没有被设置为1,怎么试着设置都不行,包括上面的CMAKE_CXX_FLAGS_DEBUG
#-DDEUG 但是可意在程序中用#ifndef NDEBUG 也就是说debug版本,这里NDEBUG没有被设置,不过relase版本没有试,是不是
#也没有被设置:) 如何能够打印编译全部过程呢?

#2.设置能够使用DEBUG的方法.
#cmake -DDEFINE_DEBUG=ON
#make
#如果想要去掉DEBUG不输出调试信息
#cmake -DDEFINE_DEBUG=OFF
#make
#这样做就完全不考虑区分debug和release的情况了

#OPTION(DEFINE_DEBUG
#  "Build the project using debugging code"
#  OFF)
#IF(DEFINE_DEBUG)
#  MESSAGE("Adding Debug flag...")
#  ADD_DEFINITIONS(-DDEBUG)
#  SET(CMAKE_BUILD_TYPE Debug)
#  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
#  MESSAGE("CMAKE_CXX_FLAGS IS " ${CMAKE_CXX_FLAGS})
#ENDIF(DEFINE_DEBUG)
OPTION(DEBUG
  "Build the project using debugging mode"
  OFF)
IF(DEBUG)
  MESSAGE("Adding Debug flag...")
  ADD_DEFINITIONS(-DDEBUG)
  SET(CMAKE_BUILD_TYPE Debug)
  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
  MESSAGE("CMAKE_CXX_FLAGS IS " ${CMAKE_CXX_FLAGS})
ENDIF(DEBUG)

OPTION(DEBUG2
  "Build the project using debugging mode levle 2"
  OFF)
IF(DEBUG2)
  MESSAGE("Adding Debug flag...")
  ADD_DEFINITIONS(-DDEBUG2)
  SET(CMAKE_BUILD_TYPE Debug)
  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
  MESSAGE("CMAKE_CXX_FLAGS IS " ${CMAKE_CXX_FLAGS})
ENDIF(DEBUG2)


OPTION(DEFILE_PROFILE
  "Profiling the project using -pg, gprof, gprof2odt"
  OFF)
IF(DEFINE_PROFILE)
  MESSAGE("Adding profile info...")
  set (CMAKE_CXX_FLAGS -pg)
  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
  MESSAGE("CMAKE_CXX_FLAGS IS " ${CMAKE_CXX_FLAGS})
ENDIF(DEFINE_PROFILE)

OPTION(DEFILE_PROFILE2
  "Profiling the project using -pg, gprof, gprof2odt"
  OFF)
IF(DEFINE_PROFILE2)
  MESSAGE("Adding profile info...")
  set (CMAKE_CXX_FLAGS "-O2 -pg")
  MESSAGE("Build type is " ${CMAKE_BUILD_TYPE})
  MESSAGE("CMAKE_CXX_FLAGS IS " ${CMAKE_CXX_FLAGS})
ENDIF(DEFINE_PROFILE2)

#for python2.6 and boost.python
include_directories(/usr/local/boost1.4/include /usr/local/include/python2.6)  
link_directories(/usr/local/boost1.4/lib /usr/local/lib)

include_directories (${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/utils/include) 
MESSAGE("Including directories for this project is " ${include_directories})
#add_subdirectory(utils/src lib)

#add_subdirectory (test bin)

add_subdirectory(unit_test bin)
