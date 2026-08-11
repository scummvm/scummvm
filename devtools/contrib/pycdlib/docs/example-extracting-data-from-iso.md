# Example: Extracting data from an existing ISO
This example will show how to extract data from an existing ISO.  Here's the complete code for this example:

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
out = BytesIO()
iso.write_fp(out)
iso.close()

iso.open_fp(out)
extracted = BytesIO()
iso.get_file_from_iso_fp(extracted, iso_path='/FOO.;1')
iso.close()

print(extracted.getvalue().decode('utf-8'))
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
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
out = BytesIO()
iso.write_fp(out)
iso.close()
```

This code creates a new ISO, adds a single file to it, and writes it out.  This is very similar to the code in [Creating a new, basic ISO](example-creating-new-basic-iso.md), so see that example for more information.  One important difference in this code is that it uses a `BytesIO` object to master the ISO into so we don't have to write any temporary data out to the filesystem; it all happens in memory.

```python
iso.open_fp(out)
```

Here we open up the ISO we created above.  We can safely re-use the PyCdlib object because we called the [close](pycdlib-apihtml#PyCdlib-close) method earlier.  Also note that we use [open_fp](pycdlib-api.html#PyCdlib-open_fp) to open the file-like object we wrote into using [write_fp](pycdlib-api.html#PyCdlib-write_fp) above.

```python
extracted = BytesIO()
iso.get_file_from_iso_fp(extracted, iso_path='/FOO.;1')
```

Now we use the [get_file_from_iso_fp](pycdlib-api.html#PyCdlib-get_file_from_iso_fp) API to extract the data from a file on the ISO.  In this case, we access the "/FOO.;1" file that we created above, and write out the data to the BytesIO object `extracted`.

```python
iso.close()

print(extracted)
```

As is the case in other examples, we close out the PyCdlib object, and print out the data we extracted.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-opening-existing-iso.html"><-- Example: Opening an existing ISO</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-bootable-iso.html">Example: Creating a bootable ISO (El Torito) --></a>
    </div>
</div>
