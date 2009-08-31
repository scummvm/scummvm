#------------------------------------------------------------------------------
#   scummvm-tools.spec
#       This SPEC file controls the building of ScummVM Tools RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm-tools
Version		: 1.0.0rc1
Release		: 1
Summary		: ScummVM-related tools
Group		: Interpreters
License		: GPL

Url             : http://www.scummvm.org

Source		: %{name}-%{version}.tar.bz2
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires	: zlib-devel
BuildRequires	: wxGTK-devel
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
echo -e "                This script is installed as\n                "%{_datadir}/scummvm-tools/convert_dxa.sh.sample >> README

%install
install -m755 -d %{buildroot}%{_bindir}
install -m755 -D compress_{agos,gob,kyra,queen,saga,scumm_bun,scumm_san,scumm_sou,sword1,sword2,tinsel,touche,tucker} %{buildroot}%{_bindir}
install -m755 -D de{cine,gob,kyra,scumm,sword2} %{buildroot}%{_bindir}
install -m755 -D encode_dxa %{buildroot}%{_bindir}/encode_dxa
install -m755 -D extract_{agos,cine,gob_stk,kyra,loom_tg16,mm_apple,mm_c64,mm_nes,parallaction,scumm_mac,t7g_mac,zak_c64} %{buildroot}%{_bindir}
install -m644 -D convert_dxa.sh %{buildroot}%{_datadir}/scummvm-tools/convert_dxa.sh.sample

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
%attr(0755,root,root)%{_bindir}/encode_dxa
%attr(0644,root,root)%{_datadir}/scummvm-tools/convert_dxa.sh.sample

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* Sat Mar 26 2005 (0.7.1)
  - first tools package
