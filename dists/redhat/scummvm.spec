#------------------------------------------------------------------------------
#   scummvm.spec
#       This SPEC file controls the building of custom ScummVM RPM 
#       packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: scummvm
Version		: 0.5.0pre-cvs
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
%doc README NEWS COPYING scummvm.xpm
%attr(0755,root,root)%{_bindir}/scummvm
%{_mandir}/man6/scummvm.6*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
* 0.4.1 (2003-05-25)
  - Added AdvMame3x filter
  - Fixed crash Curse of Monkey Island (and possibly other games as well)
  - Fixed airport doors in Zak256
  - Fixed crash in SDL backend
  - Fixed various iMuse bugs
* 0.4.0 (2003-05-11)
  - Curse of Monkey Island (comi) support (experimental)
  - Added support for the EGA versions of Loom, Monkey Island and Indy3
  - Improved music support in Indy3 and the floppy versions of Monkey Islands
  - Many Simon the Sorcerer 1 & 2 improvements and fixes
  - Very pre-alpha Beneath a Steel Sky code. Don't expect it to do anything. 
  - Even more pre-alpha support for V2 SCUMM games (Maniac Mansion and Zak)
  - Preliminary support for early Humongous Entertainment titles (very experimental)
  - New debug console and several GUI/Launcher enhancements
  - New Save/Load code (easier to expand while retaining compatibility)
  - DreamCast port now works with new games added for 0.3.0b
  - New official PalmOS port
  - Various minor and not so minor SCUMM game fixes
  - Large memory leak fixed for The Dig/ComI
  - SMUSH code optimised, frame dropping added for slower machines
  - Code cleanups
* 0.3.0 (2002-12-01)
  - massive cleanup work for iMUSE. Sam and Max music now plays correctly
  - many bugfixes for Zak256, + sound and music support
  - music support for Simon the Sorcerer on any platform with real MIDI
  - experimental support for Indy3 (VGA) - Indiana Jones + Last Crusade
  - completed support for Monkey1 VGA Floppy, The Dig
  - added akos16 implementation for The Dig and Full Throttle costumes
  - added digital iMUSE implementation for The Dig and Full Throttle music.
  - Loom CD speech+music syncronisation improved greatly
  - added midi-emulation via adlib, for platforms without sequencer support
  - code seperation of various engine parts into several libraries
  - several fixes to prevent Simon the Sorcerer crashing and hanging
  - hundreds of bugfixes for many other games
  - new SMUSH video engine, for Full Throttle and The Dig
  - new in-game GUI
  - launcher dialog
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

