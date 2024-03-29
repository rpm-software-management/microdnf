set (DNF_SRCS dnf-command.c dnf-utils.c)

glib_compile_resources (DNF_COMMAND_INSTALL plugins/install/dnf-command-install.gresource.xml
                        C_PREFIX dnf_command_install
                        INTERNAL)
list (APPEND DNF_COMMAND_INSTALL "plugins/install/dnf-command-install.c")

glib_compile_resources (DNF_COMMAND_REINSTALL plugins/reinstall/dnf-command-reinstall.gresource.xml
                        C_PREFIX dnf_command_reinstall
                        INTERNAL)
list (APPEND DNF_COMMAND_REINSTALL "plugins/reinstall/dnf-command-reinstall.c")

glib_compile_resources (DNF_COMMAND_REMOVE plugins/remove/dnf-command-remove.gresource.xml
                        C_PREFIX dnf_command_remove
                        INTERNAL)
list (APPEND DNF_COMMAND_REMOVE "plugins/remove/dnf-command-remove.c")

glib_compile_resources (DNF_COMMAND_UPGRADE plugins/upgrade/dnf-command-upgrade.gresource.xml
                        C_PREFIX dnf_command_upgrade
                        INTERNAL)
list (APPEND DNF_COMMAND_UPGRADE "plugins/upgrade/dnf-command-upgrade.c")

glib_compile_resources (DNF_COMMAND_SWAP plugins/swap/dnf-command-swap.gresource.xml
                        C_PREFIX dnf_command_swap
                        INTERNAL)
list (APPEND DNF_COMMAND_SWAP "plugins/swap/dnf-command-swap.c")

glib_compile_resources (DNF_COMMAND_DISTROSYNC plugins/distrosync/dnf-command-distrosync.gresource.xml
                        C_PREFIX dnf_command_distrosync
                        INTERNAL)
list (APPEND DNF_COMMAND_DISTROSYNC "plugins/distrosync/dnf-command-distrosync.c")

glib_compile_resources (DNF_COMMAND_REPOLIST plugins/repolist/dnf-command-repolist.gresource.xml
                        C_PREFIX dnf_command_repolist
                        INTERNAL)
list (APPEND DNF_COMMAND_REPOLIST "plugins/repolist/dnf-command-repolist.c")

glib_compile_resources (DNF_COMMAND_REPOQUERY plugins/repoquery/dnf-command-repoquery.gresource.xml
                        C_PREFIX dnf_command_repoquery
                        INTERNAL)
list (APPEND DNF_COMMAND_REPOQUERY "plugins/repoquery/dnf-command-repoquery.c")

glib_compile_resources (DNF_COMMAND_LEAVES plugins/leaves/dnf-command-leaves.gresource.xml
                        C_PREFIX dnf_command_leaves
                        INTERNAL)
list (APPEND DNF_COMMAND_LEAVES "plugins/leaves/dnf-command-leaves.c")

glib_compile_resources (DNF_COMMAND_CLEAN plugins/clean/dnf-command-clean.gresource.xml
                        C_PREFIX dnf_command_clean
                        INTERNAL)
list (APPEND DNF_COMMAND_CLEAN "plugins/clean/dnf-command-clean.c")

glib_compile_resources (DNF_COMMAND_DOWNLOAD plugins/download/dnf-command-download.gresource.xml
                        C_PREFIX dnf_command_download
                        INTERNAL)
list (APPEND DNF_COMMAND_DOWNLOAD "plugins/download/dnf-command-download.c")

glib_compile_resources (DNF_COMMAND_MAKECACHE plugins/makecache/dnf-command-makecache.gresource.xml
                        C_PREFIX dnf_command_makecache
                        INTERNAL)
list (APPEND DNF_COMMAND_MAKECACHE "plugins/makecache/dnf-command-makecache.c")

glib_compile_resources (DNF_COMMAND_MODULE_ENABLE plugins/module_enable/dnf-command-module_enable.gresource.xml
                        C_PREFIX dnf_command_module_enable
                        INTERNAL)
list (APPEND DNF_COMMAND_MODULE_ENABLE "plugins/module_enable/dnf-command-module_enable.c")

glib_compile_resources (DNF_COMMAND_MODULE_DISABLE plugins/module_disable/dnf-command-module_disable.gresource.xml
                        C_PREFIX dnf_command_module_disable
                        INTERNAL)
list (APPEND DNF_COMMAND_MODULE_DISABLE "plugins/module_disable/dnf-command-module_disable.c")

glib_compile_resources (DNF_COMMAND_MODULE_RESET plugins/module_reset/dnf-command-module_reset.gresource.xml
                        C_PREFIX dnf_command_module_reset
                        INTERNAL)
list (APPEND DNF_COMMAND_MODULE_RESET "plugins/module_reset/dnf-command-module_reset.c")


include_directories (${CMAKE_CURRENT_SOURCE_DIR})
add_executable (microdnf dnf-main.c ${DNF_SRCS}
                ${DNF_COMMAND_INSTALL}
                ${DNF_COMMAND_REINSTALL}
                ${DNF_COMMAND_REMOVE}
                ${DNF_COMMAND_UPGRADE}
                ${DNF_COMMAND_SWAP}
                ${DNF_COMMAND_DISTROSYNC}
                ${DNF_COMMAND_REPOLIST}
                ${DNF_COMMAND_REPOQUERY}
                ${DNF_COMMAND_LEAVES}
                ${DNF_COMMAND_CLEAN}
                ${DNF_COMMAND_DOWNLOAD}
                ${DNF_COMMAND_MAKECACHE}
                ${DNF_COMMAND_MODULE_ENABLE}
                ${DNF_COMMAND_MODULE_DISABLE}
                ${DNF_COMMAND_MODULE_RESET})

target_link_libraries (microdnf
                       ${GLIB_LIBRARIES}
                       ${GOBJECT_LIBRARIES}
                       ${PEAS_LIBRARIES}
                       ${LIBDNF_LIBRARIES}
                       ${SCOLS_LIBRARIES})
target_compile_definitions (microdnf
                            PRIVATE -DBUILDDIR="${CMAKE_CURRENT_BINARY_DIR}"
                            PRIVATE -DSRCDIR="${CMAKE_CURRENT_SOURCE_DIR}")

add_subdirectory (plugins)

install (TARGETS microdnf
         RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
