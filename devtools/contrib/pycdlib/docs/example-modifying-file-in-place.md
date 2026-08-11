# Example: Modifying a file in place
This example will show how to use one of the unique features of PyCdlib, the ability to modify a file in place.  While this doesn't seem like a big deal, it is actually somewhat difficult to achieve in an ISO.  The reason is that modifying a file usually involves moving around a lot of metadata, and additionally may require moving around data as well.  For these reasons, PyCdlib has limitations when modifying a file in place.  In particular:

1.  Only files can be modified in place; directories cannot be changed.
1.  Only existing files can be modified; no files can be added or removed.
1.  The file cannot be extended beyond the extent that the current file lives in.  In ISO9660 terms, an extent is (almost) always 2048 bytes.  Thus, if the current file is 48 bytes, the modification can only increase the size of the file by an additional 2000 bytes.  Shrinking a file is almost never a problem, but note that if the file contents are modified to be smaller than the original, no size will be saved on the resulting ISO.

Despite these limitations, modifying a file in place is extremely fast, much faster than traditional modification and mastering.  Therefore, if the use case calls for just changing a few bytes in a file, it is well worth it to consider modifying the file in place.

Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new()
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
outiso = BytesIO()
iso.write_fp(outiso)
iso.close()

iso.open_fp(outiso)

bazstr = b'bazzzzzz\n'
iso.modify_file_in_place(BytesIO(bazstr), len(bazstr), '/FOO.;1')

modifiediso = BytesIO()
iso.write_fp(modifiediso)
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
iso.new()
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
outiso = BytesIO()
iso.write_fp(outiso)
iso.close()
```

Create an ISO with a single file called "/FOO.;1" on it.  This is similar to previous examples, with the one exception that we are using the [write_fp](pycdlib-api.html#PyCdlib-write_fp) API to write the ISO out to a string in memory (rather than on-disk).  Note that at this point, the "/FOO.;1" file has the contents 'foo\n' on the ISO.

```python
iso.open_fp(outiso)
```

Open up the ISO that is in the `outiso` BytesIO object.

```python
bazstr = b'bazzzzzz\n'
iso.modify_file_in_place(BytesIO(bazstr), len(bazstr), '/FOO.;1')
```

Here we get to the heart of the example.  We use [modify_file_in_place](pycdlib-api.html#PyCdlib-modify_file_in_place) to modify the "/FOO.;1" file to have the contents 'bazzzzzz\n'.  We are allowed to expand the size of the file because we are still smaller than the size of the extent (the [modify_file_in_place](pycdlib-api.html#PyCdlib-modify_file_in_place) API enforces this).

```python
modifiediso = BytesIO()
iso.write_fp(modifiediso)
iso.close()
```

Write the modified ISO out to the BytesIO object called "modifiediso".  At this point, the "/FOO.;1" file on "modifiediso" has the contents 'bazzzzzz\n'.  Once we are done with this, close out the object.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-creating-udf-iso.html"><-- Example: Creating an ISO with UDF</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-managing-hard-links.html">Example: Managing hard-links on an ISO --></a>
    </div>
</div>
