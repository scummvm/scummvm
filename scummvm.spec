#------------------------------------------------------------------------------
#   scummvm.spec
#       This SPEC file controls the building of custom ScummVM RPM 
#       packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm
Version		: 0.2.0
Release		: @RELEASE@
Summary		: SCUMM graphic adventure game interpreter
Group		: Interpreters
License		: GPL

Url             : http://www.scummvm.org

Source		: %{name}-%{version}.tar.gz
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description
ScummVM is an interpreter that will play graphic adventure games written for
LucasArts' SCUMM virtual machine. It uses the SDL library for outputting
graphics.

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -n scummvm

%build
make

%install
install -m755 -D scummvm %{buildroot}%{_bindir}/scummvm
install -m644 -D scummvm.6 %{buildroot}%{_mandir}/man6/scummvm.6

%clean
rm -Rf %{buildroot}

#------------------------------------------------------------------------------
#   Files listing.  
#------------------------------------------------------------------------------
%files
%defattr(0644,root,root,0755)
%doc readme.txt whatsnew.txt copying.txt scummvm.xpm
%attr(0755,root,root)%{_bindir}/scummvm
%{_mandir}/man6/scummvm.6*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* 0.2.0 (2002-04-14)
  - core engine rewrite
  - enhanced ingame GUI, including options/volume settings.
  - auto-save feature
  - added more command-line options, and configuration file
  - new ports and platforms (MorphOS, Macintosh, Dreamcast, Solaris, IRIX, etc)
  - graphics filtering added (2xSAI, Super2xSAI, SuperEagle, AdvMame2x)
  - support for MAD MP3 compressed audio
  - support for first non-SCUMM games (Simon the Sorcerer)
  - support for V4 games (Loom CD)
  - enhanced V6 game support (Sam and Max is now completable)
  - experimental support for V7 games (Full Throttle/The Dig)
  - experimental support for V3 games (Zak256/Indy3)
* 0.1.0 (2002-01-13)
  - loads of changes
* 0.0.2 (2001-10-12):
  - bug fixes
  - save & load support
* 0.0.1 (2001-10-08):
  - initial version

