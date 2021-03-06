cmake_minimum_required(VERSION 3.5)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/Modules/")
include(CheckSymbolExists)
include(CheckCXXCompilerFlag)

project(pekwm)
set(pekwm_VERSION_MAJOR 0)
set(pekwm_VERSION_MINOR 2)
set(pekwm_VERSION_PATCH 0)
set(pekwm_VERSION "${pekwm_VERSION_MAJOR}.${pekwm_VERSION_MINOR}.${pekwm_VERSION_PATCH}")

# Check for C++11
check_cxx_compiler_flag(-std=c++11 HAS_CXX11)
if (HAS_CXX11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else (HAS_CXX11)
  message(FATAL_ERROR "Compiler with C++11 support is required")
endif (HAS_CXX11)

# Add support for more install configuration
include(GNUInstallDirs)

# Look for dependencies
find_package(X11 REQUIRED)
find_package(Iconv REQUIRED)

check_function_exists(localtime_r HAVE_LOCALTIME_R REQUIRED)

# Look for platform specific methods
check_function_exists(swprintf HAVE_SWPRINTF)
check_function_exists(setenv HAVE_SETENV)
check_function_exists(unsetenv HAVE_UNSETENV)
check_function_exists(daemon HAVE_DAEMON)
check_symbol_exists(timersub sys/time.h HAVE_TIMERSUB)

# Look for platform specific tools
find_program(GSED gsed /usr/bin /usr/local/bin /usr/pkg/bin)
if (GSED)
  set(SED ${GSED})
else (GSED)
  find_program(SED sed /usr/bin /usr/local/bin /usr/pkg/bin)
endif (GSED)

find_program(SH sh /usr/xpg4/bin/sh /bin/sh /usr/xpg4/bin)

# Optons
option(ENABLE_SHAPE "include support for Xshape" ON)
option(ENABLE_XINERAMA "include support for Xinerama" ON)
option(ENABLE_RANDR "include support for Xrandr" ON)
option(ENABLE_XFT "include support for Xft fonts" ON)
option(ENABLE_IMAGE_JPEG "include support for JPEG images" ON)
option(ENABLE_IMAGE_PNG "include support for PNG images" ON)
option(ENABLE_IMAGE_XPM "include support for XPM images" ON)

option(PEDANTIC "turn on strict compile-time warnings" OFF)
option(TESTS "include tests" OFF)

if (ENABLE_XFT)
  find_package(Freetype)
endif (ENABLE_XFT)

if (ENABLE_IMAGE_JPEG)
  find_package(JPEG)
endif (ENABLE_IMAGE_JPEG)

if (ENABLE_IMAGE_PNG)
  find_package(PNG)
endif (ENABLE_IMAGE_PNG)

# Compilation
if (ENABLE_SHAPE AND X11_Xshape_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} XShape")
  set(HAVE_SHAPE 1)
endif (ENABLE_SHAPE AND X11_Xshape_FOUND)

if (ENABLE_XINERAMA AND X11_Xinerama_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} Xinerama")
  set(HAVE_XINERAMA 1)
endif (ENABLE_XINERAMA AND X11_Xinerama_FOUND)

if (ENABLE_XFT AND X11_Xft_FOUND AND FREETYPE_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} Xft")
  set(HAVE_XFT 1)
endif (ENABLE_XFT AND X11_Xft_FOUND AND FREETYPE_FOUND)

if (ENABLE_IMAGE_JPEG AND JPEG_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} image-jpeg")
  set(HAVE_IMAGE_JPEG 1)
endif (ENABLE_IMAGE_JPEG AND JPEG_FOUND)

if (ENABLE_IMAGE_PNG AND PNG_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} image-png")
  set(HAVE_IMAGE_PNG 1)
endif (ENABLE_IMAGE_PNG AND PNG_FOUND)

if (ENABLE_IMAGE_XPM AND X11_Xpm_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} image-xpm")
  set(HAVE_IMAGE_XPM 1)
endif (ENABLE_IMAGE_XPM AND X11_Xpm_FOUND)

if (ENABLE_RANDR AND X11_Xrandr_FOUND)
  set(pekwm_FEATURES "${pekwm_FEATURES} Xrandr")
  set(HAVE_XRANDR 1)
endif (ENABLE_RANDR AND X11_Xrandr_FOUND)

if (CMAKE_BUILD_TYPE MATCHES Debug)
  set(pekwm_FEATURES "${pekwm_FEATURES} debug")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
endif (CMAKE_BUILD_TYPE MATCHES Debug)

if (PEDANTIC)
  set(pekwm_FEATURES "${pekwm_FEATURES} pedantic")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
endif (PEDANTIC)

add_definitions(-DVERSION="${pekwm_VERSION}"
  -DFEATURES="${pekwm_FEATURES}"
  -DBINDIR="${CMAKE_INSTALL_PREFIX}/bin"
  -DDATADIR="${CMAKE_INSTALL_PREFIX}/share"
  -DSYSCONFDIR="${CMAKE_INSTALL_FULL_SYSCONFDIR}/pekwm")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH True)

# Subdirectories
add_subdirectory(contrib)
add_subdirectory(data)
add_subdirectory(doc)
add_subdirectory(src)

if (TESTS)
   enable_testing()
   add_subdirectory(test)
endif (TESTS)
