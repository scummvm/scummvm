Name: scummvm
Version: 0.0.3
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
* Mon Nov 19 2001 Claudio Matsuoka <claudio@helllabs.org>
+ scummvm-0.0.3-1cm
- package creation

