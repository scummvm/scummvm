# Example: Creating an ISO with Rock Ridge extensions
This example will show how to create an ISO with the [Rock Ridge](standards.md#rock-ridge) extensions.  Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new(rock_ridge='1.09')
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')
iso.add_directory('/DIR1', rr_name='dir1')
iso.write('new.iso')
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

As in earlier examples, import the relevant libraries, including pycdlib itself.

```python
iso = pycdlib.PyCdlib()
iso.new(rock_ridge='1.09')
```

Create a new PyCdlib object, and then create a new ISO with that object.  In order to make it have Rock Ridge extensions, we pass the argument `rock_ridge="1.09"` to the [new](pycdlib-api.html#PyCdlib-new) method.  PyCdlib supports Rock Ridge versions 1.09, 1.10, and 1.12, though 1.09 is more common.

```python
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', rr_name='foo')
```

As in earlier examples, create a new file on the ISO from a string.  Because this is a Rock Ridge ISO, we have to also supply the `rr_name` argument to the [add_fp](pycdlib-api.html#PyCdlib-add_fp) method.  Forgetting the `rr_name` argument on a Rock Ridge ISO is an error and PyCdlib will throw an exception.  Note that it is called `rr_name`, and that the argument given is truly a name, not an absolute path.  This is because Rock Ridge is an extension to the original ISO9660, and this alternate name will be stored alongside the original ISO data.

```python
iso.add_directory('/DIR1', rr_name='dir1')
```

Create a new directory on the ISO.  Again we must pass the `rr_name` argument to [add_directory](pycdlib-api.html#PyCdlib-add_directory), for all of the same reasons and with the same restrictions as we saw above for [add_fp](pycdlib-api.html#PyCdlib-add_fp).

```python
iso.write('new.iso')
iso.close()
```

Write the new ISO out to a file, then close out the ISO.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-creating-bootable-iso.html"><-- Example: Creating a bootable ISO (El Torito)</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-joliet-iso.html">Example: Creating an ISO with Joliet --></a>
    </div>
</div>
