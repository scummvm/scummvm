Name: scummvm
Version: 0.2.0
Release: 1cm
Summary: SCUMM graphic adventure game interpreter
Group: Interpreters
License: GPL
Source: %{name}-%{version}.tar.gz
BuildRoot: %{_tmppath}/%{name}-%{version}-root

%description
ScummVM is an interpreter that will play graphic adventure games written for
LucasArts' SCUMM virtual machine. It uses the SDL library for outputting
graphics.

%prep
%setup -q -n scummvm

%build
make

%install
install -m755 -D scummvm %{buildroot}%{_bindir}/scummvm
install -m644 -D scummvm.6 %{buildroot}%{_mandir}/man6/scummvm.6

%clean
rm -Rf %{buildroot}

%files
%defattr(0644,root,root,0755)
%doc readme.txt whatsnew.txt
%attr(0755,root,root)%{_bindir}/scummvm
%{_mandir}/man6/scummvm.6*

%changelog
+ scummvm-0.2.0-1cm
- core engine updates
- add support for cd audio (Monkey Island/LoomCD)
- add support for v3 games (Zak256/LoomCD/Indy3)
- add support for v7 games (Full Throttle/Dig)
- added more command line options
- save game screen works in more games
- auto-save feature
- support for graphic filters
- support for MAD (Mpeg Audio Decoding)
+ scummvm-0.1.0
- bug fix release
+ scummvm-0.0.3
- package creation

