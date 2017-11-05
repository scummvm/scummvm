#------------------------------------------------------------------------------
#   residualvm.spec
#       This SPEC file controls the building of ResidualVM RPM packages.
#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
#   Prologue information
#------------------------------------------------------------------------------
Name		: residualvm
Version		: 0.4.0git
Release		: 1
Summary		: Interpreter for several 3D games
Group		: Amusements/Games
License		: GPL

Url             : http://www.residualvm.org

Source		: %{name}-%{version}.tar.bz2
Source1		: libmad-0.15.1b.tar.gz
Source2		: libmpeg2-0.5.1.tar.gz
BuildRoot	: %{_tmppath}/%{name}-%{version}-root

BuildRequires: desktop-file-utils
BuildRequires: zlib-devel
BuildRequires: freetype-devel
BuildRequires: SDL-devel >= 1.2.2

#------------------------------------------------------------------------------
#   Description
#------------------------------------------------------------------------------
%description

#------------------------------------------------------------------------------
#   install scripts
#------------------------------------------------------------------------------
%prep
%setup -q -a 1 -a 2 -a 3 -n residualvm-%{version}
patch0 -p0
mkdir tmp

%build
(cd libmad-0.15.1b; sed -i "s/optimize=\"\$optimize -fforce-mem\"/#optimize=\"\$optimize -fforce-mem\"/" configure; \
sed -i "s/optimize=\"\$optimize -fforce-mem\"/#optimize=\"\$optimize -fforce-mem\"/" configure.ac; \
./configure --enable-static --disable-shared --prefix=%{_builddir}/residualvm-%{version}/tmp; make -j 4; make install)
(cd libmpeg2-0.5.1; ./configure --enable-static --disable-shared --prefix=%{_builddir}/residualvm-%{version}/tmp; make -j 4; make install)
./configure --with-mad-prefix=%{_builddir}/residualvm-%{version}/tmp --with-mpeg2-prefix=%{_builddir}/residualvm-%{version}/tmp --prefix=%{_prefix} --enable-release
make

%install
install -m755 -D residualvm %{buildroot}%{_bindir}/residualvm
install -m644 -D dists/residualvm.6 %{buildroot}%{_mandir}/man6/residualvm.6
install -m644 -D icons/residualvm.xpm %{buildroot}%{_datadir}/pixmaps/residualvm.xpm
install -m644 -D icons/residualvm.svg %{buildroot}%{_datadir}/icons/hicolor/scalable/apps/residualvm.svg
install -m644 -D dists/redhat/residualvm48.png %{buildroot}%{_datadir}/icons/hicolor/48x48/apps/residualvm.png
install -m644 -D gui/themes/modern.zip %{buildroot}%{_datadir}/residualvm/modern.zip
install -m644 -D dists/engine-data/residualvm-grim-patch.lab %{buildroot}%{_datadir}/residualvm/residualvm-grim-patch.lab
install -m644 -D dists/engine-data/residualvm-emi-patch.m4b %{buildroot}%{_datadir}/residualvm/residualvm-emi-patch.m4b
install -m644 -D dists/engine-data/myst3.dat %{buildroot}%{_datadir}/residualvm/myst3.dat
desktop-file-install --vendor residualvm --dir=%{buildroot}/%{_datadir}/applications dists/residualvm.desktop

%clean
rm -Rf ${RPM_BUILD_ROOT}

%post
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

%postun
touch --no-create %{_datadir}/icons/hicolor || :
if [ -x %{_bindir}/gtk-update-icon-cache ]; then
        %{_bindir}/gtk-update-icon-cache --quiet %{_datadir}/icons/hicolor || :
fi

#------------------------------------------------------------------------------
#   Files listing.
#------------------------------------------------------------------------------
%files
%defattr(0644,root,root,0755)
%doc AUTHORS README.md KNOWN_BUGS NEWS COPYING COPYING.LGPL COPYING.BSD COPYING.FREEFONT COPYING.ISC COPYING.LUA COPYING.TINYGL COPYRIGHT
%attr(0755,root,root)%{_bindir}/residualvm
%{_datadir}/applications/*
%{_datadir}/pixmaps/residualvm.xpm
%{_datadir}/icons/hicolor/48x48/apps/residualvm.png
%{_datadir}/icons/hicolor/scalable/apps/residualvm.svg
%{_datadir}/residualvm/modern.zip
%{_datadir}/residualvm/residualvm-grim-patch.lab
%{_datadir}/residualvm/residualvm-emi-patch.m4b
%{_datadir}/residualvm/myst3.dat
%{_mandir}/man6/residualvm.6*

#------------------------------------------------------------------------------
#   Change Log
#------------------------------------------------------------------------------
%changelog
