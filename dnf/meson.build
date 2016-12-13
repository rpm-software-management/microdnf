microdnf_srcs = [
  'dnf-main.c',
  'dnf-command.c',
  'dnf-utils.c',

  # install
  gnome.compile_resources(
    'dnf-install',
    'plugins/install/dnf-command-install.gresource.xml',
    c_name : 'dnf_command_install',
    source_dir : 'plugins/install',
  ),
  'plugins/install/dnf-command-install.c',

  # remove
  gnome.compile_resources(
    'dnf-remove',
    'plugins/remove/dnf-command-remove.gresource.xml',
    c_name : 'dnf_command_remove',
    source_dir : 'plugins/remove',
  ),
  'plugins/remove/dnf-command-remove.c',

  # update
  gnome.compile_resources(
    'dnf-update',
    'plugins/update/dnf-command-update.gresource.xml',
    c_name : 'dnf_command_update',
    source_dir : 'plugins/update',
  ),
  'plugins/update/dnf-command-update.c',
]

executable(
  'microdnf',
  sources : microdnf_srcs,
  dependencies : [
    glib,
    gobject,
    libpeas,
    libdnf,
  ],
  c_args : [
    '-DBUILDDIR="@0@"'.format(meson.current_build_dir()),
    '-DSRCDIR="@0@"'.format(meson.current_source_dir()),
  ],
  install : true
)
