# Example: Reading a large file in chunks
It may be useful in some applications to be able to read a file from an ISO a bit at a time and do some processing on it.  PyCdlib provides the context manager [open_file_from_iso](pycdlib-api.html#PyCdlib-open_file_from_iso) API to allow opening a file and reading in parts of it.  Here's the complete code for this example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new()
foostr = b'foofoo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')

with iso.open_file_from_iso(iso_path='/FOO.;1') as infp:
    all1 = infp.read()
    infp.seek(0)
    first = infp.read(3)
    second = infp.read()

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

As we've seen before, import pycdlib.  We also import the [BytesIO](https://docs.python.org/3/library/io.html#binary-i-o) module so we can use a python string as a file-like object.

```python
iso = pycdlib.PyCdlib()
iso.new()
foostr = b'foofoo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
```

This code creates a new ISO, adds a single file to it, and writes it out.  This is very similar to the code in [Creating a new, basic ISO](example-creating-new-basic-iso.md), so see that example for more information.

```python
with iso.open_file_from_iso(iso_path='/FOO.;1') as infp:
```

Here we use the [open_file_from_iso](pycdlib-api.html#PyCdlib-open_file_from_iso) API to get a context manager to the file that we created; this will be used in the rest of the explanations below.

```python
    all = infp.read()
```

The first `read` call reads in all of the data in the file, so at the end of the call the "all" variable will contain `foofoo\n`. 

```python
    infp.seek(0)
```

The 'seek' call then resets the file pointer back to the beginning of the file.

```python
    first = infp.read(3)
```

If the `read` API is passed a number, it will read and return that many bytes.  In this case, the 'first' variable will end up containing `foo`.

```python
    second = infp.read()
```

And now we read the rest of the data, so the 'second' variable will end up containing `foo\n`.

```python
iso.close()
```

As is the case in other examples, we close out the PyCdlib object.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-walking-iso-filesystem.html"><-- Example: Walking the ISO filesystem</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="design.html">Design --></a>
    </div>
</div>
