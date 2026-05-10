# Example: Creating a "hybrid" bootable ISO
The first 32768 bytes of any ISO are designated as "system use".  In a normal ISO (even an El Torito bootable one), these bytes are all zero, but this space can also be used to add in alternative booting mechanisms.  In particular, this space can be used to embed boot code so that the file can be written to a USB stick and booted.  These so called "hybrid" ISO files thus have two booting mechanisms: if the file is actually burned to a CD, then "El Torito" is used to boot, but if it is written to a USB stick, then the system use boot code is used to boot.  PyCdlib supports creating hybrid bootable ISOs through the main API, and the following example will show how.

Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()

iso.new()

bootstr = b'boot\n'
iso.add_fp(BytesIO(bootstr), len(bootstr), '/BOOT.;1')

iso.add_eltorito('/BOOT.;1')

iso.add_isohybrid()

iso.write('eltorito.iso')

iso.close()
```

Let's take a closer look at the code.

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib
```

As usual, import the necessary libraries, including pycdlib.

```python
iso = pycdlib.PyCdlib()

iso.new()
```

Create a new PyCdlib object, and then create a new, basic ISO.

```python
isolinuxstr = b'\x00'*0x40 + b'\xfb\xc0\x78\x70'
iso.add_fp(BytesIO(isolinuxstr), len(isolinuxstr), '/BOOT.;1')
```

Add a file called /BOOT.;1 to the ISO.  The contents of this conform to the expected boot start sequence as specified by isolinux.  A complete discussion of the correct form of the file is out of scope for this tutorial; see [isolinux](http://www.syslinux.org/wiki/index.php?title=ISOLINUX) for more details.  The above is the minimum code that conforms to the sequence, though it is not technically bootable.

```python
iso.add_eltorito('/BOOT.;1', boot_load_size=4)
```

Add El Torito to the ISO, making the boot file "/BOOT.;1", and setting the `boot_load_size` to 4.  The `boot_load_size` is the number of 512-bytes sectors to read during initial boot.  While other values may be allowed for this, all current examples (from cdrkit or isolinux) use this value.  After this call, the ISO is El Torito bootable, but not yet a hybrid ISO.

```python
iso.add_isohybrid()
```

Add the boot file to the system use area, making this a hybrid ISO.  There are various parameters that can be passed to control how the hybrid file is added, but the defaults are typically good enough for creating a hybrid ISO similar to those made for most Linux distributions.

```python
iso.write('eltorito.iso')

iso.close()
```

Write the ISO out to a file, and close out the PyCdlib object.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-forcing-consistency.html"><-- Example: Forcing consistency</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-walking-iso-filesystem.html">Example: Walking the ISO filesystem --></a>
    </div>
</div>
