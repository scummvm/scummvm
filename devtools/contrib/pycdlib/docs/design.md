# Design

## Overview
The aim of PyCdlib is to be a pure Python library that can be used to easily interact with the filesystems that make up various optical disks (collectively known as "ISOs" throughout the rest of this document).  This includes the original ISO9660 standard (also known as Ecma-119), the El Torito booting standard, the Joliet and Rock Ridge extensions, and the UDF filesystem.

## Context
The original motivation for writing the library was to replace the subprocess calls to genisoimage in [Oz](https://github.com/clalancette/oz) with something pure Python.  During initial research, no suitably complete, Python-only ISO manipulation library was found.  It was also discovered that [cdrkit](https://launchpad.net/cdrkit) (the upstream project that contains genisoimage) is dormant, embroiled in a somewhat bitter fork, contains several serious bugs, and lacks an externally visible test suite.  PyCdlib was created to address the above problems and provide a pure Python library for ISO introspection and manipulation.

## Goals
* A pure Python library to interact with optical disk filesystems.
* Support for reading, writing (mastering), and introspecting optical disk filesystems.
* Support for all of the common optical disk filesystems and their extensions.
* Compatibility with the large corpus of existing ISOs (this implies reading ISOs that violate the standards).
* Relatively simple API.
* Expansive test coverage.
* In-place modification of existing ISOs, something that none of the other libraries support.
* Performance approaching that of genisoimage.

## Non-Goals
* Support for non-optical disk filesystem (ext4, NTFS, FAT, etc are not supported).
* Speed improvements via a less portable C library.

## Existing solutions
The [cdrkit](https://launchpad.net/cdrkit) project mentioned in the Context section is the canonical Linux ISO filesystem manipulation program on many Linux distributions.  The upstream project that it was forked from is called [cdrtools](http://cdrtools.sourceforge.net/private/cdrecord.html).  While cdrtools is *not* dormant, it does not offer a Python library, and thus does not meet the original criteria for the project.

The [isoparser](https://github.com/barneygale/isoparser) project is a Python library to parse ISO9660 and Rock Ridge filesystems, but is unmaintained and now inactive.

## PyCdlib solution
PyCdlib allows the user to either open an existing ISO or create a new ISO from scratch.  From that point forward, the ISO can be manipulated or introspected in various ways by calling additional APIs on the object.  Once the filesystem has been modified to the users specification, it can then be written out to a file descriptor.

## Testing
PyCdlib has an extensive test suite of hundreds of tests that get run on each release. There are four types of tests that PyCdlib currently runs:

- In unit tests, low-level details of the PyCdlib implementation are tested for completeness and code coverage.
- In parsing integration tests, specific sequences of files and directories are created, and then an ISO is generated using genisoimage from [cdrkit](https://launchpad.net/cdrkit). Then the PyCdlib [open](pycdlib-api.html#PyCdlib-open) method is used to open up the resulting file and check various aspects of the file. This ensures that PyCdlib can successfully open up existing ISOs.
- In new integration tests, a new ISO is created using the PyCdlib [new](pycdlib-api.html#PyCdlib-new) method, and the ISO is manipulated in specific ways. Various aspects of these newly created files are compared against known examples to ensure that things were created as they should be.
- In hybrid integration tests, specific sequences of files and directories are created, and then an ISO is generated using genisoimage from [cdrkit](https://launchpad.net/cdrkit). Then the PyCdlib [open](pycdlib-api.html#PyCdlib-open) method is used to open up the resulting file, and the ISO is manipulated in specific ways. Various aspects of these newly created files are compared against known examples to ensure that things were created as they should be.

PyCdlib currently has 95% code coverage from the tests, and anytime a new bug is found, a test is written to ensure that the bug can’t happen again.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-reading-file-in-chunks.html"><-- Example: Reading a large file in chunks</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="tools.html">Tools --></a>
    </div>
</div>
