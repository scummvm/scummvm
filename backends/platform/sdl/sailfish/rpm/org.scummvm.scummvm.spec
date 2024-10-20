%if %{undefined outdir}
%define outdir out-sailfish.%{_arch}
%endif

%define freetype_version 2.13.3
%define flac_version 1.4.3
%define theora_version 1.1.1
%define jpeg_turbo_version 3.0.4
%define libmad_version 0.15.1b
%define libmpeg2_version 0.5.1
%define libfaad_version 2.8.8
%define giflib_version 5.2.2
%define fribidi_version 1.0.16
%define sdl2_version 2.30.7
%define vpx_version 1.14.1

#define engine_config --disable-all-engines --enable-engines=scumm  --disable-tinygl
%define engine_config %{nil}

%define _rpmfilename %%{NAME}-%%{VERSION}-%%{RELEASE}.%%{ARCH}.rpm

Name:       org.scummvm.scummvm
Summary:    ScummVM: Multi-game engine
Version:    0
Release:    1
Group:      Qt/Qt
License:    GPLv3
URL:        https://scummvm.org
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(sdl2)
BuildRequires:  SDL2_net-devel
BuildRequires:  pkgconfig(ogg)
BuildRequires:  pkgconfig(vorbis)
BuildRequires:  pkgconfig(libpng)
BuildRequires:  pkgconfig(egl)
BuildRequires:  pkgconfig(glesv2)
BuildRequires:  cmake
BuildRequires:  git
# libSDL deps
BuildRequires: pkgconfig(wayland-egl)
BuildRequires: pkgconfig(wayland-client)
BuildRequires: pkgconfig(wayland-cursor)
BuildRequires: pkgconfig(wayland-protocols)
BuildRequires: pkgconfig(wayland-scanner)
BuildRequires: pkgconfig(glesv1_cm)
BuildRequires: pkgconfig(xkbcommon)
BuildRequires: pkgconfig(libpulse-simple)

%description
ScummVM: Multi-game engine

%build
mkdir -p %{outdir}
mkdir -p %{outdir}/scummvm
mkdir -p %{outdir}/pkgconfig
if ! [ -d %{outdir}/freetype-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/freetype-%{freetype_version}.tar.xz
    cd freetype-%{freetype_version}
    ./configure --prefix=$PWD/../freetype-install --disable-shared --enable-static
    make %{?_smp_mflags}
    make install
    cd ../..
fi
if ! [ -d %{outdir}/flac-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/flac-%{flac_version}.tar.xz
    cd flac-%{flac_version}
    ./configure --disable-shared --enable-static --disable-examples --disable-programs
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../flac-install INSTALL_ROOT=$PWD/../flac-install install
    cd ../..
fi
if ! [ -d %{outdir}/theora-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/libtheora-%{theora_version}.tar.xz
    cd libtheora-%{theora_version}
    ./configure --disable-shared --enable-static --disable-examples --disable-programs
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../theora-install INSTALL_ROOT=$PWD/../theora-install install
    cd ../..
fi
if ! [ -d %{outdir}/jpeg-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/libjpeg-turbo-%{jpeg_turbo_version}.tar.gz
    cd libjpeg-turbo-%{jpeg_turbo_version}
    %cmake -DENABLE_SHARED=FALSE
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../jpeg-install INSTALL_ROOT=$PWD/../jpeg-install install
    cd ../..
fi
if ! [ -d %{outdir}/libmad-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/libmad-%{libmad_version}.tar.gz
    cd libmad-%{libmad_version}
%if  "%{_arch}" == "arm"
    ASFLAGS=-marm CFLAGS="-O2 -marm" ./configure --disable-shared --enable-static --disable-examples --disable-programs CFLAGS="-O2 -marm" ASFLAGS=-marm
    make CFLAGS="-O2 -marm" ASFLAGS=-marm %{?_smp_mflags}
%else
    CFLAGS="-O2" ./configure --disable-shared --enable-static --disable-examples --disable-programs CFLAGS="-O2"
    make CFLAGS="-O2" %{?_smp_mflags}
%endif
    make DESTDIR=$PWD/../libmad-install INSTALL_ROOT=$PWD/../libmad-install install
    cd ../..
fi
if ! [ -d %{outdir}/mpeg2-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/libmpeg2-%{libmpeg2_version}.tar.gz
    cd libmpeg2-%{libmpeg2_version}
    ./configure --disable-shared --enable-static --disable-examples --disable-programs
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../mpeg2-install INSTALL_ROOT=$PWD/../mpeg2-install install
    cd ../..
fi
if ! [ -d %{outdir}/faad-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/faad2-%{libfaad_version}.tar.gz
    cd faad2-%{libfaad_version}
    echo "Applying patch faad-lrint.patch"
    patch -p1 -i "../../faad-lrint.patch"
    ./configure --disable-shared --enable-static --disable-examples --disable-programs
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../faad-install INSTALL_ROOT=$PWD/../faad-install install
    cd ../..
fi
if ! [ -d %{outdir}/giflib-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/giflib-%{giflib_version}.tar.gz
    cd giflib-%{giflib_version}
    make %{?_smp_mflags} libgif.a
    make DESTDIR=$PWD/../giflib-install INSTALL_ROOT=$PWD/../giflib-install install-include
    install -d "$PWD/../giflib-install/usr/local/lib"
    install -m 644 libgif.a "$PWD/../giflib-install/usr/local/lib/libgif.a"
    cd ../..
fi
if ! [ -d %{outdir}/vpx-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/libvpx-%{vpx_version}.tar.gz
    cd libvpx-%{vpx_version}
    echo "Applying patch vpx-busybox.patch"
    patch -p1 -i "../../vpx-busybox.patch"
    ./configure --disable-shared --enable-static --disable-examples --target=generic-gnu
    make %{?_smp_mflags}
    make DESTDIR=$PWD/../vpx-install INSTALL_ROOT=$PWD/../vpx-install install
    cd ../..
fi
if ! [ -d %{outdir}/fribidi-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/fribidi-%{fribidi_version}.tar.xz
    cd fribidi-%{fribidi_version}
    ./configure --disable-shared --enable-static --prefix=$PWD/../fribidi-install
    make %{?_smp_mflags}
    make install
    cd ../..
fi
if ! [ -d %{outdir}/sdl-install ]; then
    cd %{outdir}
    tar xvf ../rpmdeps/SDL2-%{sdl2_version}.tar.gz
    cd SDL2-%{sdl2_version}
    for x in ../../sdl-patches/*.patch; do
	echo "Applying patch $x"
	patch -p1 -i "$x"
    done
    cd ..
    cmake \
    -Bsdl-build \
    -DSDL_PULSEAUDIO=ON \
    -DSDL_RPATH=OFF \
    -DSDL_STATIC=ON \
    -DSDL_SHARED=OFF \
    -DSDL_WAYLAND=ON \
    -DSDL_X11=OFF \
    -DSDL_DBUS=ON \
    -DSDL_WAYLAND_LIBDECOR=OFF \
    -DSDL_WAYLAND_QT_TOUCH=OFF \
    SDL2-%{sdl2_version}
    make -C sdl-build %{?_smp_mflags}
    make -C sdl-build install DESTDIR=$PWD/sdl-install INSTALL_ROOT=$PWD/sdl-install
    cd ..
fi
sed "s@Libs: .*@Libs: $PWD/%{outdir}/freetype-install/lib/libfreetype.a@g" < %{outdir}/freetype-install/lib/pkgconfig/freetype2.pc > %{outdir}/pkgconfig/freetype2.pc
sed "s@Libs: .*@Libs: $PWD/%{outdir}/fribidi-install/lib/libfribidi.a@g" < %{outdir}/fribidi-install/lib/pkgconfig/fribidi.pc > %{outdir}/pkgconfig/fribidi.pc
export PKG_CONFIG_PATH=$PWD/%{outdir}/pkgconfig:$PKG_CONFIG_PATH
export PKG_CONFIG_LIBDIR=$PWD/%{outdir}/pkgconfig:$PKG_CONFIG_LIBDIR
cd %{outdir}/scummvm;
../../../../../../configure --host=sailfish \
		--with-jpeg-prefix=../jpeg-install/usr \
		--with-mad-prefix=../libmad-install/usr/local \
		--with-flac-prefix=../flac-install/usr/local \
		--with-theoradec-prefix=../theora-install/usr/local \
		--with-mpeg2-prefix=../mpeg2-install/usr/local \
		--with-faad-prefix=../faad-install/usr/local \
		--with-gif-prefix=../giflib-install/usr/local \
		--enable-fribidi --with-fribidi-prefix=../fribidi-install  \
		--enable-vpx --with-vpx-prefix=../vpx-install/usr/local \
		--with-sdl-prefix=../sdl-install/usr/local --enable-static \
		%{engine_config}
cd ../..
%{__make} -C %{outdir}/scummvm %{_make_output_sync} %{?_smp_mflags}

%install
rm -rf %{buildroot}/*
%{__make} -C %{outdir}/scummvm DESTDIR=%{buildroot} INSTALL_ROOT=%{buildroot} install
# TODO: Move this stuff into make
mkdir -p %{buildroot}/usr/share/applications
mkdir -p %{buildroot}/usr/share/icons/hicolor/86x86/apps
mkdir -p %{buildroot}/usr/share/icons/hicolor/108x108/apps
mkdir -p %{buildroot}/usr/share/icons/hicolor/128x128/apps
mkdir -p %{buildroot}/usr/share/icons/hicolor/172x172/apps
cp ../../../../dists/sailfish/org.scummvm.scummvm.desktop %{buildroot}/usr/share/applications/org.scummvm.scummvm.desktop
cp ../../../../dists/sailfish/86x86.png %{buildroot}/usr/share/icons/hicolor/86x86/apps/org.scummvm.scummvm.png
cp ../../../../dists/sailfish/108x108.png %{buildroot}/usr/share/icons/hicolor/108x108/apps/org.scummvm.scummvm.png
cp ../../../../dists/sailfish/128x128.png %{buildroot}/usr/share/icons/hicolor/128x128/apps/org.scummvm.scummvm.png
cp ../../../../dists/sailfish/172x172.png %{buildroot}/usr/share/icons/hicolor/172x172/apps/org.scummvm.scummvm.png

%files
%defattr(755,root,root,-)
%{_bindir}/org.scummvm.scummvm

%defattr(644,root,root,-)

%{_datadir}/org.scummvm.scummvm/applications/%{name}.desktop
%{_datadir}/org.scummvm.scummvm/doc/scummvm/*
%{_datadir}/org.scummvm.scummvm/icons/hicolor/scalable/apps/org.scummvm.scummvm.svg
%{_datadir}/org.scummvm.scummvm/man/man6/scummvm.6
%{_datadir}/org.scummvm.scummvm/metainfo/org.scummvm.scummvm.metainfo.xml
%{_datadir}/org.scummvm.scummvm/pixmaps/org.scummvm.scummvm.xpm
%{_datadir}/org.scummvm.scummvm/scummvm/*
%{_datadir}/applications/org.scummvm.scummvm.desktop
%{_datadir}/icons/hicolor/108x108/apps/org.scummvm.scummvm.png
%{_datadir}/icons/hicolor/128x128/apps/org.scummvm.scummvm.png
%{_datadir}/icons/hicolor/172x172/apps/org.scummvm.scummvm.png
%{_datadir}/icons/hicolor/86x86/apps/org.scummvm.scummvm.png
