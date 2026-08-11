Name: cxxtest
Summary: CxxTest Testing Framework for C++
Version: %{version}
Release: 1
Copyright: LGPL
Group: Development/C++
Source: cxxtest-%{version}.tar.gz
BuildRoot: /tmp/cxxtest-build
BuildArch: noarch
Prefix: /usr

%description
CxxTest is a unit testing framework for C++ that is similar in
spirit to JUnit, CppUnit, and xUnit. CxxTest is easy to use because
it does not require precompiling a CxxTest testing library, it
employs no advanced features of C++ (e.g. RTTI) and it supports a
very flexible form of test discovery.

%prep
%setup -n cxxtest

%build

%install
install -m 755 -d $RPM_BUILD_ROOT/usr/bin $RPM_BUILD_ROOT/usr/include/cxxtest
install -m 755 bin/cxxtestgen $RPM_BUILD_ROOT/usr/bin/
install -m 644 cxxtest/* $RPM_BUILD_ROOT/usr/include/cxxtest/

%clean
rm -rf $RPM_BUILD_ROOT

%files
%attr(-, root, root) %doc README
%attr(-, root, root) %doc sample
%attr(-, root, root) /usr/include/cxxtest
%attr(-, root, root) /usr/bin/cxxtestgen
