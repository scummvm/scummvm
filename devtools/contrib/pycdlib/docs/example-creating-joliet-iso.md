# Example: Creating an ISO with Joliet extensions
This example will show how to create an ISO with the [Joliet](standards.md#joliet) extensions.  Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new(joliet=3)
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')
iso.add_directory('/DIR1', joliet_path='/dir1')
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
iso.new(joliet=3)
```

Create a new PyCdlib object, and then create a new ISO with that object.  In order to make it have Joliet extensions, we pass the argument `joliet=3` to the [new](pycdlib-api.html#PyCdlib-new) method.  PyCdlib supports Joliet levels 1, 2, and 3, but level 3 is by far the most common, so is recommended.

```python
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', joliet_path='/foo')
```

As in earlier examples, create a new file on the ISO from a string.  Because this is a Joliet ISO, we have to provide the `joliet_path` argument to [add_fp](pycdlib-api.html#PyCdlib-add_fp) as well.  In contrast to Rock Ridge, Joliet is a completely different context from the original ISO9660 structure, and so the argument to be passed here must be an absolute path, not a name.  Because of this, the Joliet file can be on a completely different part of the directory structure, or be omitted completely (in which case the file will only show up on the ISO9660 portion of the ISO).  In practice the Joliet portion of the ISO almost always mirrors the ISO9660 portion of the ISO, so it is recommended to do that when creating new Joliet ISOs.

```python
iso.add_directory('/DIR1', joliet_path='/dir1')
```

Create a new directory on the ISO.  Again we must pass the `joliet_path` argument to [add_directory](pycdlib-api.html#PyCdlib-add_directory), for all of the same reasons and with the same restrictions as we saw above for [add_fp](pycdlib-api.html#PyCdlib-add_fp).

```python
iso.write('new.iso')
iso.close()
```

Write the new ISO out to a file, then close out the ISO.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-creating-rock-ridge-iso.html"><-- Example: Creating an ISO with Rock Ridge</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-udf-iso.html">Example: Creating an ISO with UDF --></a>
    </div>
</div>
