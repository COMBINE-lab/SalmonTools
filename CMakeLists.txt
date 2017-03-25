cmake_minimum_required (VERSION 3.0)

enable_testing()

project (SalmonTools)

set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "1")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
set(CPACK_PACKAGE_VENDOR "Stony Brook University")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SalmonTools - Tools for working with Salmon output.")
set(CPACK_PACKAGE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_PACKAGE_FILE_NAME
  "${CMAKE_PROJECT_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-Source")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

if (APPLE)
set (WARNING_IGNORE_FLAGS "-Wno-deprecated-register")
else()
set (WARNING_IGNORE_FLAGS "")
endif()

set (BOOST_CXX_FLAGS "${WARNING_IGNORE_FLAGS} -std=c++11")
## Prefer static to dynamic libraries
SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})

## Set the standard required compile flags
# Nov 18th --- removed -DHAVE_CONFIG_H
set (CMAKE_CXX_FLAGS "-ftree-vectorize -funroll-loops -fPIC -fomit-frame-pointer -O3 -DRAPMAP_SALMON_SUPPORT -DHAVE_ANSI_TERM -DHAVE_SSTREAM -Wall -Wno-unknown-pragmas -Wno-reorder -Wno-unused-variable -std=c++11 -Wreturn-type -pthread")

##
# OSX is strange (some might say, stupid in this regard).  Deal with it's quirkines here.
##
if (APPLE)
    # To allow ourselves to build a dynamic library, we have to tell the compiler
    # that, yes, the symbols will be around at runtime.
    # set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -undefined dynamic_lookup")
    set (LIBSALMONTOOLS_LINKER_FLAGS "-all_load")
    # In order to "think different", we also have to use non-standard suffixes
    # for our shared libraries
    set(SHARED_LIB_EXTENSION "dylib")
else()
    # We're in sane linux world
   set (SHARED_LIB_EXTENSION "so")
   set (LIBSALMON_LINKER_FLAGS "")
endif()

## this get's set differently below if we
## are on clang & apple
set (NON_APPLECLANG_LIBS gomp rt)

if(UNIX AND NOT APPLE)
	set(LIBRT rt)
endif()

set (PTHREAD_LIB)

##
# Let us check the sha sum of our pacakges if we have the right tools
##
set(SHASUM ${CMAKE_CURRENT_SOURCE_DIR}/scripts/check_shasum.sh)

##
# Compiler-specific C++11 activation.
# http://stackoverflow.com/questions/10984442/how-to-detect-c11-support-of-a-compiler-with-cmake
##
##
# First take care of what to do if we have gcc
##
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU")
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    # If we're on OSX
    if (APPLE AND NOT (GCC_VERSION VERSION_GREATER 4.8.2 OR GCC_VERSION VERSION_EQUAL 4.8.2))
        message(FATAL_ERROR "When building under OSX, ${PROJECT_NAME} requires "
                            "either clang or g++ >= 4.8.2")
    elseif (NOT (GCC_VERSION VERSION_GREATER 4.7 OR GCC_VERSION VERSION_EQUAL 4.7))
        message(FATAL_ERROR "${PROJECT_NAME} requires g++ 4.7 or greater.")
    endif ()

    set (GCC TRUE)

    # Put complete static linking on hold for the time-being
    # If we're not on OSX, make an attempt to compile everything statically
    #if (NOT APPLE)
    #set (CMAKE_CXX_FLAGS "-static ${CMAKE_CXX_FLAGS}")
    #set (CMAKE_EXE_LINK_FLAGS "-static")
    set (PTHREAD_LIB "pthread")
    #endif()

    # If we're on Linux (i.e. not OSX) and we're using
    # gcc, then set the -static-libstdc++ flag
    if (NOT APPLE)
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
    endif()

    set (WARNING_IGNORE_FLAGS "${WARNING_IGNORE_FLAGS} -Wno-unused-local-typedefs")
# Clang 
elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(CLANG TRUE)
    # If we have libc++, then try and use it
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag(-stdlib=libc++ HAVE_LIBCPP)
    if (HAVE_LIBCPP)
        message ("It appears that you're compiling with clang and that libc++ is available, so I'll use that")
        set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
	    set (BOOST_TOOLSET "clang")
        set (BOOST_CONFIGURE_TOOLSET "--with-toolset=clang")
	    set (BCXX_FLAGS "-stdlib=libc++ -DBOOST_HAS_INT128")
	    set (BOOST_EXTRA_FLAGS toolset=clang cxxflags=${BCXX_FLAGS} linkflags="-stdlib=libc++")
        set (JELLYFISH_CXX_FLAGS "-stdlib=libc++")
    # Otherwise, use libstdc++ (and make it static)
    else()
        #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
    endif()
    # There's currently a bug with clang-3.4 & Boost 1.55 -- this hack fixes it
    # but we should do something better (does this break things if CPU doesn't
    # have 128-bit support)?
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DBOOST_HAS_INT128")

    if (APPLE)
        message("Building on a Mac")
        set (NON_APPLECLANG_LIBS "")
    else()
        set (PTHREAD_LIB "pthread")
    endif()
else ()
    message(FATAL_ERROR "Your C++ compiler does not support C++11.")
endif ()

if (DO_QUIET_MAKE)
    set( QUIET_MAKE "--silent" )
else()
    set( QUIET_MAKE "")
endif()

include(ExternalProject)

find_package (ZLIB)
if (NOT ZLIB_FOUND)
	message (FATAL_ERROR "zlib must be installed before configuration & building can proceed")
endif()

##
#  Update the CXX flags according to the system and compiler
##
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_IGNORE_FLAGS}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    message ("Making Debug build")
    set (CMAKE_CXX_FLAGS_DEBUG "-g ${CMAKE_CXX_FLAGS}")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    message ("Making Release build")
    set (CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS}")
else ()
    message ("Making Default build type")
endif ()

##
# Record this top-level path
##
set (GAT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Have CMake tell us what it's doing
# set (CMAKE_VERBOSE_MAKEFILE true)

###
#
#  Grab RapMap sources for quasi-mapping code --- DURING CONFIGURE TIME!
#
####
#if(NOT FETCHED_RAPMAP)
#  exec_program(${CMAKE_CURRENT_SOURCE_DIR}/scripts/fetchRapMap.sh)
#  set(FETCHED_RAPMAP TRUE CACHE BOOL "Has RapMap been fetched?" FORCE)
#endif()


#message("Build system will fetch and build the Cereal serialization library")
#message("==================================================================")
#include(ExternalProject)
#ExternalProject_Add(libcereal
#    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
#    DOWNLOAD_COMMAND curl -k -L https://github.com/USCiLab/cereal/archive/v1.2.2.tar.gz -o cereal-v1.2.2.tar.gz &&		 
#                     ${SHASUM} 1921f26d2e1daf9132da3c432e2fd02093ecaedf846e65d7679ddf868c7289c4  cereal-v1.2.2.tar.gz &&
#                     tar -xzvf cereal-v1.2.2.tar.gz
#
#    ##
#    #URL https://github.com/USCiLab/cereal/archive/v1.2.2.tar.gz
#    #DOWNLOAD_NAME cereal-v1.2.2.tar.gz
#    #TLS_VERIFY FALSE
#    #URL_HASH SHA1=ffddf5fc5313cfbb893e07823ca8c473084eebca
#    ##
#    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/cereal-1.2.2
#    INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
#    #UPDATE_COMMAND sh -c "mkdir -p <SOURCE_DIR>/build"
#    BINARY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/cereal-1.2.2/build
#    CONFIGURE_COMMAND ""
#    BUILD_COMMAND ""
#    INSTALL_COMMAND sh -c "mkdir -p <INSTALL_DIR>/include && cp -r <SOURCE_DIR>/include/cereal <INSTALL_DIR>/include"
#)
#ExternalProject_Add_Step(libcereal makedir
#  COMMAND mkdir -p <SOURCE_DIR>/build 
#  COMMENT "Make build directory"
#  DEPENDEES download 
#  DEPENDERS configure)


message("Build system will fetch SPDLOG")
message("==================================================================")
ExternalProject_Add(libspdlog
    DOWNLOAD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external
    DOWNLOAD_COMMAND curl -k -L https://github.com/COMBINE-lab/spdlog/archive/v0.12.0.tar.gz -o spdlog-v0.12.0.tar.gz &&
                     ${SHASUM} 5cfd6a0b3182a88e1eb35bcb65a7ef9035140d7c73b16ba6095939dbf07325b9  spdlog-v0.12.0.tar.gz &&
                     tar -xzf spdlog-v0.12.0.tar.gz 
    ##
    #URL https://github.com/COMBINE-lab/spdlog/archive/v0.12.0.tar.gz
    #DOWNLOAD_NAME spdlog-v0.12.0.tar.gz
    #TLS_VERIFY FALSE
    #URL_HASH SHA1=c868b4d1a27c4d9fb3b748f0566a8f0390d83ffb
    ##
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/spdlog-0.12.0
    INSTALL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/install
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND cp -r <SOURCE_DIR>/include/spdlog <INSTALL_DIR>/include
)

###
#
# Done building external dependencies.
#
###
set (CPACK_SOURCE_IGNORE_FILES
"/build/"
"/bin/"
"/lib/"
"/external/"
".git/")

message("CPACK_SOURCE_IGNORE_FILES = ${CPACK_SOURCE_IGNORE_FILES}")

# Recurse into SalmonTools source directory
add_subdirectory ( src )

# build a CPack driven installer package
include (CPack)

set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | gzip > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.gz
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
