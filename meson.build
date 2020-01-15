project('microdnf', 'c',
        version : '3.4.0',
        license : 'GPL-3.0+',
        default_options : [
          'b_asneeded=True',
          'b_lundef=True',
          'c_std=gnu11',
          'warning_level=1',
        ],
        meson_version : '>=0.36.0')

cc = meson.get_compiler('c')
test_cflags = [
  '-fstrict-aliasing',
  '-Wformat=2',
  '-Wunused',
  '-Wuninitialized',
  '-Wstrict-prototypes',
  '-Wmissing-prototypes',
  '-Werror=implicit',
  '-Werror=init-self',
  '-Werror=main',
]
foreach cflag : test_cflags
  if cc.has_argument(cflag)
    add_project_arguments(cflag, language : 'c')
  endif
endforeach

gnome = import('gnome')

glib = dependency('glib-2.0', version : '>=2.44.0')
gobject = dependency('gobject-2.0', version : '>=2.44.0')
libpeas = dependency('libpeas-1.0', version : '>=1.20.0')
libdnf = dependency('libdnf', version : '>=0.7.0')
scols = dependency('smartcols')

pkg_libdir = join_paths(get_option('prefix'), get_option('libdir'), 'dnf')
pkg_datadir = join_paths(get_option('prefix'), get_option('datadir'), 'dnf')
add_project_arguments(
  '-DPACKAGE_LIBDIR="@0@"'.format(pkg_libdir),
  '-DPACKAGE_DATADIR="@0@"'.format(pkg_datadir),
  language : 'c',
)

subdir('dnf')

help2man = find_program('help2man', required: true)
if help2man.found()
  help2man_opts = [
    '--version-string=' + meson.project_version(),
    '--no-info',
    '--section=8',
    '--name=Micro DNF',
  ]

  custom_target('microdnf.8',
                output: 'microdnf.8',
                command: [
                  help2man, help2man_opts, '--output=@OUTPUT@', microdnf
                ],
                install: true,
                install_dir: get_option('mandir') + '/man8',
  )
endif
