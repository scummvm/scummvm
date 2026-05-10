# ScummVM AppImage build scripts for Raspberry Pi

This is the initial import of felsqualle's Raspberry Pi build scripts written in 2023.
They act as a template for future development of an AppImage container and possibly
other formats for this platform, like other forms of static builds or maybe even
self-containing SD card images.

## Warning
The scripts in their current state are *highly* experimental. They are targeting
the Raspberry Pi 4/400, using an old version of Debian/Raspberry Pi OS.

Running these scripts without modification on a modern installation of Raspberry Pi OS
will *very likely* destroy your system. There's no containerization, no chroot, nothing.
The build script runs directly on your host system without any isolation.

*Do not use it if you don't know exactly what you are doing*

This set of scripts will also act as a template for felsqualle's future
development on the Raspberry Pi, so stay tuned.
