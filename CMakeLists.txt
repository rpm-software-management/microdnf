cmake_minimum_required (VERSION 2.8.5)
project (microdnf C)
set (PROJECT_VERSION 3.8.0)

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
pkg_check_modules (PEAS REQUIRED libpeas-1.0>=1.20.0)
pkg_check_modules (LIBDNF REQUIRED libdnf>=0.62.0)
pkg_check_modules (SCOLS REQUIRED smartcols)

set (PKG_LIBDIR ${CMAKE_INSTALL_FULL_LIBDIR}/dnf)
set (PKG_DATADIR ${CMAKE_INSTALL_FULL_DATADIR}/dnf)
add_definitions (-DPACKAGE_LIBDIR="${PKG_LIBDIR}")
add_definitions (-DPACKAGE_DATADIR="${PKG_DATADIR}")

find_file (HELP2MAN_EXECUTABLE help2man)
if (NOT HELP2MAN_EXECUTABLE)
  message (FATAL_ERROR "unable to find help2man")
endif ()

set (MANPAGE ${CMAKE_CURRENT_BINARY_DIR}/microdnf.8)
add_custom_command (
  DEPENDS microdnf
  OUTPUT ${MANPAGE}
  COMMAND ${HELP2MAN_EXECUTABLE} $<TARGET_FILE:microdnf>
  --version-string=${PROJECT_VERSION}
  --no-info
  --section=8
  --name="Micro DNF"
  --output=${MANPAGE}
)
add_custom_target (manpage ALL DEPENDS ${MANPAGE})
install (FILES ${MANPAGE} DESTINATION ${CMAKE_INSTALL_FULL_MANDIR}/man8)

include_directories (${GLIB_INCLUDE_DIRS})
include_directories (${GOBJECT_INCLUDE_DIRS})
include_directories (${PEAS_INCLUDE_DIRS})
include_directories (${LIBDNF_INCLUDE_DIRS})
include_directories (${SCOLS_INCLUDE_DIRS})

add_subdirectory (dnf)
