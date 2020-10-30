%global libdnf_version 0.55.0

Name:           microdnf
Version:        3.4.0
Release:        1%{?dist}
Summary:        Minimal C implementation of DNF

License:        GPLv3+
URL:            https://github.com/rpm-software-management/microdnf
Source0:        %{url}/archive/v%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.36.0
BuildRequires:  pkgconfig(glib-2.0) >= 2.44.0
BuildRequires:  pkgconfig(gobject-2.0) >= 2.44.0
BuildRequires:  pkgconfig(libpeas-1.0) >= 1.20.0
BuildRequires:  pkgconfig(libdnf) >= %{libdnf_version}
BuildRequires:  pkgconfig(smartcols)
BuildRequires:  help2man

Requires:       libdnf%{?_isa} >= %{libdnf_version}

%description
Micro DNF is a very minimal C implementation of DNF's install, upgrade,
remove, repolist, and clean commands, designed to be used for doing simple
packaging actions in containers when you don't need full-blown DNF and
you want the tiniest useful containers possible.

That is, you don't want any interpreter stack and you want the most
minimal environment possible so you can build up to exactly what you need.

This is not a substitute for DNF for real systems, and many of DNF's
capabilities are intentionally not implemented in Micro DNF.


%prep
%autosetup -p1

%build
%meson
%meson_build

%install
%meson_install

%check
%meson_test

%files
%license COPYING
%doc README.md
%{_mandir}/man8/microdnf.8*
%{_bindir}/%{name}

%changelog
