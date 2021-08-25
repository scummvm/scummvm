===========================
Handling Mac game files
===========================

This guide explains how to dump files from Macintosh HFS and HFS+ media, which includes CD-ROMs and floppy disks. It expands on the information contained on the :doc:`Handling game files <../use_scummvm/game_files>` page.

There are three main aspects of Macintosh floppy or CD media that can make it difficult to extract the game data.

HFS and HFS+ File system
=========================

The Macintosh uses a different file system to other operating systems (HFS for older media, and HFS+ for more recent ones). On a Windows or Linux system you need additional software to view and copy the files from HFS and HFS+ floppy disks and CDs.

HFS, Hierarchical File System, or Mac OS Standard, was used on Macintosh hard disk, floppy disks, and CDs until the late 90s. It was superseded by HFS+, or Mac OS Extended, with the release of Mac OS 8.1 in 1998. But most game CDs released before the mid-2000 still use HFS. Modern macOS computer can still access HFS+ media, but support for reading HFS media was dropped in macOS 10.15 (Catalina).

Other systems, such as Linux and Windows, require an additional software to read HFS and HFS+ media.

For more information:
 * `<https://en.wikipedia.org/wiki/Hierarchical_File_System>`_
 * `<https://en.wikipedia.org/wiki/HFS_Plus>`_

Forks
======

Macintosh HFS and HFS+ media have a concept called forks. For those familiar with NTFS, it's the same principle as NTFS streams. A file can consist of two forks, a data fork and a resource fork. Both are important for games to work correctly. Systems other than macOS do not support resource forks, and a regular file copy from a HFS or HFS+ media on such a system will only copy the data fork. To properly preserve the resource fork during the copy, those files need to be converted to a MacBinary file.

For more information:
 * `<https://en.wikipedia.org/wiki/Resource_fork>`_

File name encoding
===================

Filenames in Japanese games are encoded in a special variant that was unique to Apple. The result is that a naive copy will not always show the correct filenames.

Macintosh also allowed special characters to be used that are illegal in filenames on other platforms.

ScummVM runs on a wide variety of platforms and not all those platforms can store Japanese filenames or other special, i.e. non-ascii, characters. ScummVM uses punycode to handle file names that would otherwise not be supported. Punycode converts all non-ascii characters to a special format. Filenames for which the original had special characters will start with ``xn--`` (for example ``xn--Icon-ja6e``).

Files that contain unallowed characters are always punyencoded.

.. note::

    Windows, MacOS and Linux all can store these files and don't need punycode enabled.

For more information:
 * `<https://en.wikipedia.org/wiki/Punycode>`_
 * List of unallowed characters in Windows Filenames: `<https://docs.microsoft.com/en-us/windows/win32/fileio/naming-a-file#naming-conventions>`_

.. _macfiledumper:

Macintosh File Dumper
=====================

ScummVM supplies two versions of dumper-companion: a disk/iso image only one that runs in the browser and a more full fledged one in python. Those dumper can extract files from a HFS medium, encode files to MacBinary and rename files with Punycode if needed.

.. note::

    The dumper companions only suports HFS disks (except for the python dumper companion in MAC mode which also supports HFS+, but requires having a Macintosh). Most, if not all, Macintosh games supported by ScummVM were released on a HFS medium. But if you do have a HFS+ CD then you can check the :ref:`macfileaccessother` section.

Create ISO image
-----------------

The first step is to create an ISO image of your floppy or CD medium.

.. note::

    This step is not needed on macOS for HFS+ media (and HFS media on a macOS 10.14 Mojave system or older) when using the Python dumper in MAC mode.

.. tabbed:: Windows

    .. panels::
        :column: col-lg-12

        You can use an application like IsoBuster<https://www.isobuster.com> to create iso image of CD.

.. tabbed:: macOS

    .. panels::
        :column: col-lg-12

        You can use the following commands in a Terminal to find the identifier of the media and then create an iso image of that media.

        List disk to get the identifier for the CD or floppy disk partition.
        .. code-block::

            diskutil list

        You can also find this information using the Disk Utility application. Select the CD partition and look at the name under the *device* section.

        Unmount the disk if needed. By default disks are mounted when they are inserted, and they need to be unmounted so that you can create an iso file. But since HFS is no longer supported on macOS 10.15 and above, those disk cannot be mounted and do not need to be unmounted.
        .. code-block::

            sudo umount /dev/diskID

        Create the iso image.
        .. code-block::

            sudo dd if=/dev/diskID of=/path/name.iso

        Eject the CD.
        .. code-block::

            drutil tray eject

        .. dropdown:: Example

            In the example below the CD is the last disk listed (you can recognize it from the ``CD_partition_scheme`` type name) and the identifier for the HFS partition is ``disk5s1s2``.

            .. code-block::

                iMac:~ ego$ diskutil list
                /dev/disk0 (internal, physical):
                   #:                       TYPE NAME                    SIZE       IDENTIFIER
                   0:      GUID_partition_scheme                        *121.3 GB   disk0
                   1:                        EFI EFI                     209.7 MB   disk0s1
                   2:                 Apple_APFS Container disk4         121.1 GB   disk0s2

                /dev/disk1 (internal, physical):
                   #:                       TYPE NAME                    SIZE       IDENTIFIER
                   0:      GUID_partition_scheme                        *3.0 TB     disk1
                   1:                        EFI EFI                     209.7 MB   disk1s1
                   2:                 Apple_APFS Container disk4         3.0 TB     disk1s2

                /dev/disk2 (synthesized):
                   #:                       TYPE NAME                    SIZE       IDENTIFIER
                   0:      APFS Container Scheme -                      +3.1 TB     disk2
                                                Physical Stores disk0s2, disk1s2
                   1:                APFS Volume Macintosh HD            1.8 TB     disk2s1
                   2:                APFS Volume Preboot                 42.2 MB    disk2s2
                   3:                APFS Volume Recovery                510.6 MB   disk2s3
                   4:                APFS Volume VM                      20.5 KB    disk2s4

                /dev/disk3 (external, physical):
                   #:                       TYPE NAME                    SIZE       IDENTIFIER
                   0:        CD_partition_scheme                        *783.5 MB   disk3
                   1:     Apple_partition_scheme                         682.2 MB   disk3s1
                   2:        Apple_partition_map                         1.0 KB     disk3s1s1
                   3:                  Apple_HFS disque 1                681.6 MB   disk5s1s2
                iMac:~ ego$ sudo umount /dev/disk3s1s2
                Password:
                iMac:~ ego$ sudo dd if=/dev/disk3s1s2 of=/Users/ego/BS1-fr.iso
                1331200+0 records in
                1331200+0 records out
                681574400 bytes transferred in 396.380454 secs (1719495 bytes/sec)
                iMac:~ ego$ drutil tray eject

.. tabbed:: Linux

    .. panels::
        :column: col-lg-12

        You can use the following commands to create an ISO from an inserted HFS or HFS+ medium.

        .. code-block::

            sudo dd if=/dev/cdrom of=/path/name.iso

        Usually the device will be ``/dev/cdrom``, ``/dev/dvd``, or ``/dev/floppy``. You can also for example use ``sudo blkid`` to list the mounted and unmounted devices with their ID.

        .. dropdown:: Example

            .. code-block::

                ~$ sudo blkid
                /dev/sr0: PTTYPE="mac"
                ~$ sudo dd if=/dev/sr0 of=/home/ego/BS1-fr.iso
                1332420+0 records in
                1332420+0 records out
                682199040 bytes (682 MB, 651 MiB) copied, 421.918 s, 1.6 MB/s


Browser based dumper companion
-------------------------------

 `<https://servilla.org/dumper-companion/>`_

The browser based dumper companion has support for reading HFS as isos and disk images such as floppies. It generates a zip file that can be extracted and imported in ScummVM.

It's options are:
 * Language: Choose Japanese when dumping Japanese disks
 * Unicode: Default is on. Deselect it when your platform doesn't work with unicode.

The steps are:
 1. Upload your disk image
 2. Click the dump button
 3. Download the zip file
 4. Extract the zip file
 5. Add the extracted directory to scummvm

.. note::

    This web dumper-companion works entirely in your web browser and does not transfer the data to any server.

Python dumper companion
------------------------

`<https://github.com/scummvm/scummvm/blob/master/devtools/dumper-companion.py>`_

Installation
~~~~~~~~~~~~~

Prerequisite: Python3 with pip installed. Information on how to install Python can be found here: `<https://wiki.python.org/moin/BeginnersGuide/Download>`_

Download the dumper companion file.

Install it's requirements:

.. tabbed:: macOS

    .. panels::
        :column: col-lg-12

        .. code-block::

            pip3 install machfs xattr

.. tabbed:: Others

    .. panels::
        :column: col-lg-12

        .. code-block::

            pip3 install machfs

Modes
~~~~~~

The dumper-companion supports three modes: `iso`, `dir` and `mac`. The last mode is specific to macOS.

The dumper-companion has help text:

.. code-block::

    ./dumper-companion.py --help

.. note::

    There's also a `str` mode that is used test drive the punyencode parts. For more info see it's help section.

ISO mode
+++++++++

This mode will dump a HFS iso to the OUTPUT directory. This directory can then be added to scummvm.

.. code-block::

    dumper-companion.py iso [-h] [--punycode] [--japanese] INPUT OUTPUT

Options:
 * ``--punycode``: encode all filename characters as punycode, select this when your platform doesn't support UTF-8 filenames
 * ``--japanese``: use this when dumping Japanese disks

DIR mode
++++++++++

This mode will punyencode all files and directories in the current dir.

.. code-block::

    dumper-companion.py dir [-h] directory

MAC mode
++++++++++

This mode is specific to macOS. It will traverse a directory, find all the resource forks and combine them in a MacBinary. It is useful for when you already have all game files on your harddrive and want to import them into ScummVM.

.. code-block::

    dumper-companion.py mac [-h] [--punycode] directory

Options:
 * ``--punyencode``: encode all filename characters as punycode, select this when your platform doesn't support UTF-8 filenames

.. note::

    On macOS ScummVM directly access data and resource forks. But converting to MacBinary can be useful if you plan to transfer those files to a different system.

.. _macfileaccessother:

Other methods
==============

There are various solutions to access HFS and HFS+ media on Windows, macOS, and Linux that allow to copy the files manually.

.. tabbed:: Windows

    .. panels::
        :column: col-lg-12

        For Windows, `HFS Explorer <http://www.catacombae.org/hfsexplorer/>`_  is a basic and free option, which gives you read-only access to both HFS and HFS+ drives. Use the installer rather than the zip file, to ensure it is installed correctly. For files with a resource fork you will need to use the option to exact as MacBinary. Extract files that only have a data fork as a "raw copy, data fork".

        Alternatively `HFVExplorer <https://www.emaculation.com/doku.php/hfvexplorer>`_ can also be used for HFS drives. There is no option to extract as MacBinary, but instead you can extract files with a resource fork as AppleDouble (using the "extract data and resource fork(s)" option).

.. tabbed:: macOS

    .. panels::
        :column: col-lg-12

        On macOS you can read directly HFS+ volumes, and in some cases HFS volume, and copy the files in the usual way in the Finder. Some discs shipped with hidden files that need to be copied. To view hidden files in macOS, press :kbd:`Cmd+Shift+.` in a Finder window.

        There is also no need to convert files with a resource fork into MacBinary files, but if you want to do it, for example to later transfer the files onto a different system, you can use the ``macbinary`` command line tool.

        .. code-block::

            macbinary encode -o <outfile> <infile>

        However the :ref:`macfiledumper` is recommended in some cases:

           * HFS media can only be read on macOS 10.14 (Mojave) and older. Support for reading HFS volumes was removed in macOS 10.15 (Catalina).
           * File may need to be renamed as some names may contain invalid characters.
           * You plan to transfer those files to a different system.

.. tabbed:: Linux

    .. panels::
        :column: col-lg-12


        You can access HFS+ drives using ``hfsplus``. To use hfsplus, use the command line:

           1. Install hfsplus using the software manager. On Debian-based distributions, use ``sudo apt install hfsplus``.
           2. Find the game disc by running ``sudo fdisk -l`` and finding the one with type ``Apple HFS/HFS+``. In this example, this is ``/dev/fd0``.
           3. Create a mount point, for example: ``sudo mkdir /media/macgamedrive``
           4. Mount the device to that moint point: ``sudo mount -t hfsplus /dev/fd0 /media/macgamedrive``
           5. Access the device at ``/media/macgamedrive``. To copy files you can use ``hpcopy``. It takes options to indicate if the files should be converted to macbinary (``-m``) or copied as a raw file (``-r``).

        You can access HFS drives using ``hfsutils``. To use hfsutils, use the command line:

           1. Install hfsutils using the software manager. On Debian-based distributions, use ``sudo apt install hfsutils``.
           2. Find the game disc by running ``sudo fdisk -l`` and finding the one with type ``Apple HFS/HFS+``. In this example, this is ``/dev/fd0``.
           3. Mount the HFS volume by running ``hmount /dev/fd0``
           4. You can list the files and directories on the HFS media using ``hls``, change the working directory on the HFS media using ``hcd`` and copy files using ``hcopy``. The ``hcopy`` commands take options to indicate if the files should be converted to macbinary (``-m``) or copied as a raw file (``-r``). For example ``hcopy -m "PP Disk 1:PP Data:JMP PP Resources" "pegasus/JMP PP Resources"``.
           5. Unmount the HFS media with ``humount /dev/fd0``
