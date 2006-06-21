#------------------------------------------------------------------------------
#   scummvm-tools.spec
#       This SPEC file controls the building of ScummVM Tools RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm-tools
Version		: 0.9.0
Release		: 1
Summary		: ScummVM-related tools
Group		: Interpreters
License		: GPL

Url             : http://www.scummvm.org

Source		: %{name}-%{version}.tar.bz2
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires	: zlib-devel
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
make

%install
install -m755 -D compress_{saga,scumm_san,scumm_sou,scumm_bun,simon,sword1,sword2,queen} %{buildroot}%{_bindir}
install -m755 -D de{kyra,scumm,sword2} %{buildroot}%{_bindir}
install -m755 -D extract_{kyra,loom_tg16,mm_nes,mm_c64,zak_c64,scumm_mac,simon1_amiga} %{buildroot}%{_bindir}

%clean
rm -Rf ${RPM_BUILD_ROOT}

#------------------------------------------------------------------------------
#   Files listing.  
#------------------------------------------------------------------------------
%files
%doc README COPYING
%attr(0755,root,root)%{_bindir}/compress_*
%attr(0755,root,root)%{_bindir}/de*
%attr(0755,root,root)%{_bindir}/extract_*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Sat Mar 26 2005 (0.7.1)
  - first tools package
