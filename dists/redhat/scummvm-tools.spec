#------------------------------------------------------------------------------
#   scummvm-tools.spec
#       This SPEC file controls the building of ScummVM Tools RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm-tools
Version		: 2.10.0git
Release		: 1
Summary		: ScummVM-related tools
Group		: Interpreters
License		: GPL

Url             : https://www.scummvm.org

Source		: %{name}-%{version}.tar.xz
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires: zlib-devel
BuildRequires: wxGTK3-devel
BuildRequires: libmad-devel
BuildRequires: libvorbis-devel
BuildRequires: libogg-devel
BuildRequires: libpng-devel
BuildRequires: boost-devel
BuildRequires: flac-devel
BuildRequires: freetype-devel

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
Tools for compressing ScummVM datafiles and other related tools.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -n scummvm-tools-%{version}

%build
./configure --prefix=%{_prefix}
make %{_smp_mflags}

%install
make DESTDIR=%{buildroot} install
rm %{buildroot}%{_datadir}/scummvm-tools/detaillogo.jpg
rm %{buildroot}%{_datadir}/scummvm-tools/logo.jpg
rm %{buildroot}%{_datadir}/scummvm-tools/scummvmtools.icns
rm %{buildroot}%{_datadir}/scummvm-tools/scummvmtools.ico
rm %{buildroot}%{_datadir}/scummvm-tools/scummvmtools_128.png
rm %{buildroot}%{_datadir}/scummvm-tools/tile.gif

%clean
rm -Rf ${RPM_BUILD_ROOT}

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%doc README COPYING
%attr(0755,root,root)%{_bindir}/*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Thu Nov 23 2017 (2.0.0)
  - remove own libmad since this is now in Fedora itself
* Sat Apr 03 2010 (1.2.0)
  - include libmad
* Sat Mar 26 2005 (0.7.1)
  - first tools package
