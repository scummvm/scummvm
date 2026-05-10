# Example: Creating a bootable ISO (El Torito)
This example will show how to create a bootable [El Torito](standards.md#el-torito) ISO.  Here's the complete code for the example:

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
bootstr = b'boot\n'
iso.add_fp(BytesIO(bootstr), len(bootstr), '/BOOT.;1')
```

Add a file called /BOOT.;1 to the ISO.  This is the file that contains the data to be used to boot the ISO when placed into a computer.  The name of the file can be anything (and can even be nested in directories), but the contents have to be very specific.  Getting the appropriate data into the boot file is beyond the scope of this tutorial; see [isolinux](http://www.syslinux.org/wiki/index.php?title=ISOLINUX) for one way of getting the appropriate data.  Suffice it to say that the example code that we are using above will not actually boot, but is good enough to show the PyCdlib API usage.

```python
iso.add_eltorito('/BOOT.;1')
```

Add El Torito to the ISO, making the boot file "/BOOT.;1".  After this call, the ISO is actually bootable.  By default, the [add_eltorito](pycdlib-api.html#PyCdlib-add_eltorito) method will use so-called "no emulation" booting, which allows arbitrary data in the boot file.  "Hard drive" and "floppy" emulation is also supported, though these options are more esoteric and need specifically configured boot data to work properly.

```python
iso.write('eltorito.iso')

iso.close()
```

Write the ISO out to a file, and close out the PyCdlib object.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-extracting-data-from-iso.html"><-- Example: Extracting data from an existing ISO</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-rock-ridge-iso.html">Example: Creating an ISO with Rock Ridge --></a>
    </div>
</div>
