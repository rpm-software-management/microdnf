cmake_minimum_required (VERSION 2.8.5)
project (microdnf C)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include (CheckCCompilerFlag)
include (GNUInstallDirs)
include (GResource)

check_c_compiler_flag ("--std=gnu11" SUPPORT_GNU11_STD)
if (NOT SUPPORT_GNU11_STD)
  message (FATAL_ERROR "Support for --std=gnu11 is mandatory")
endif ()
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --std=gnu11")

find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB REQUIRED glib-2.0>=2.44.0)
pkg_check_modules (GOBJECT REQUIRED gobject-2.0>=2.44.0)
pkg_check_modules (PEAS QUIET libpeas-1.0>=1.20.0)
pkg_check_modules (LIBDNF REQUIRED libdnf>=0.7.0)

if (NOT PEAS_FOUND)
  include (ExternalProject)
  set (PEAS_ARCHIVE ${CMAKE_CURRENT_LIST_DIR}/libpeas-1.20.0.tar.xz)
  set (PEAS_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/libpeas)
  pkg_check_modules (PEAS_INTERNALS REQUIRED glib-2.0>=2.38.0 gobject-2.0>=2.38.0 gmodule-2.0>=2.38.0 gio-2.0>=2.32.0 gobject-introspection-1.0>=1.39.0)
  ExternalProject_Add (
    libpeas_external
    URL https://download.gnome.org/sources/libpeas/1.20/libpeas-1.20.0.tar.xz
    URL_HASH SHA256=f392fffe2dc00072a51bb2a1b274224a06bdc5f9c3b3c0ac4816909e9933d354
    CONFIGURE_COMMAND
      COMMAND ./configure --prefix=${PEAS_INSTALL_PREFIX} --enable-static --disable-shared --disable-lua5.1 --disable-luajit --disable-python2 --disable-python3 --disable-gtk --disable-glade-catalog
    BUILD_COMMAND
      COMMAND make
    INSTALL_COMMAND
      COMMAND make install
    BUILD_IN_SOURCE 1
  )

  add_library (libpeas STATIC IMPORTED)
  add_dependencies (libpeas libpeas_external)
  set_target_properties (libpeas PROPERTIES IMPORTED_LOCATION ${PEAS_INSTALL_PREFIX}/lib/libpeas-1.0.a)
  set (PEAS_INCLUDE_DIRS ${PEAS_INSTALL_PREFIX}/include/libpeas-1.0;${PEAS_INTERNALS_STATIC_INCLUDE_DIRS})
  set (PEAS_LIBRARIES libpeas;${PEAS_INTERNALS_STATIC_LIBRARIES})
endif ()

set (PKG_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR}/dnf)
set (PKG_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/dnf)
add_definitions (-DPACKAGE_LIBDIR="${PKG_LIBDIR}")
add_definitions (-DPACKAGE_DATADIR="${PKG_DATADIR}")

include_directories (${GLIB_INCLUDE_DIRS})
include_directories (${GOBJECT_INCLUDE_DIRS})
include_directories (${PEAS_INCLUDE_DIRS})
include_directories (${LIBDNF_INCLUDE_DIRS})

add_subdirectory (dnf)
