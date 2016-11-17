cmake_minimum_required (VERSION 2.8.5)
project (microdnf C)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include (GNUInstallDirs)
include (GResource)

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=gnu11")

find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB REQUIRED glib-2.0>=2.44.0)
pkg_check_modules (GOBJECT REQUIRED gobject-2.0>=2.44.0)
pkg_check_modules (PEAS REQUIRED libpeas-1.0>=1.15.0)
pkg_check_modules (LIBDNF REQUIRED libdnf>=0.7.0)

set (PKG_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR}/dnf)
set (PKG_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/dnf)
add_definitions (-DPACKAGE_LIBDIR="${PKG_LIBDIR}")
add_definitions (-DPACKAGE_DATADIR="${PKG_DATADIR}")

include_directories (${GLIB_INCLUDE_DIRS})
include_directories (${GOBJECT_INCLUDE_DIRS})
include_directories (${PEAS_INCLUDE_DIRS})
include_directories (${LIBDNF_INCLUDE_DIRS})

add_subdirectory (dnf)
