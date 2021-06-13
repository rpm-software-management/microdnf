%global libdnf_version 0.62.0

Name:           microdnf
Version:        3.8.0
Release:        1%{?dist}
Summary:        Lightweight implementation of DNF in C

License:        GPLv2+
URL:            https://github.com/rpm-software-management/microdnf
Source0:        %{url}/archive/%{version}/%{name}-%{version}.tar.gz

BuildRequires:  gcc
BuildRequires:  meson >= 0.36.0
BuildRequires:  pkgconfig(glib-2.0) >= 2.44.0
BuildRequires:  pkgconfig(gobject-2.0) >= 2.44.0
BuildRequires:  pkgconfig(libpeas-1.0) >= 1.20.0
BuildRequires:  pkgconfig(libdnf) >= %{libdnf_version}
BuildRequires:  pkgconfig(smartcols)
BuildRequires:  help2man

Requires:       libdnf%{?_isa} >= %{libdnf_version}
%if 0%{?rhel} > 8 || 0%{?fedora}
# Ensure DNF package manager configuration skeleton is installed
Requires:       dnf-data
%endif

%description
Micro DNF is a lightweight C implementation of DNF, designed to be used
for doing simple packaging actions when you don't need full-blown DNF and
you want the tiniest useful environments possible.

That is, you don't want any interpreter stack and you want the most
minimal environment possible so you can build up to exactly what you need.


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
