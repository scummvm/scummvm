# Example: Creating an ISO with UDF
This example will show how to create an ISO with the [UDF](standards.md#udf) bridge format.  Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new(udf='2.60')
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', udf_path='/foo')
iso.add_directory('/DIR1', udf_path='/dir1')
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
iso.new(udf='2.60')
```

Create a new PyCdlib object, and then create a new ISO with that object.  In order to make it have UDF, we pass the argument `udf='2.60'` to the [new](pycdlib-api.html#PyCdlib-new) method.

```python
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1', udf_path='/foo')
```

As in earlier examples, create a new file on the ISO from a string.  Because this is a UDF ISO, we have to provide the `udf_path` argument to [add_fp](pycdlib-api.html#PyCdlib-add_fp) as well.  Like Joliet, UDF is a completely different context from the original ISO9660 structure, and so the argument to be passed here must be an absolute path, not a name.  Because of this, the UDF file can be on a completely different part of the directory structure, or be omitted completely (in which case the file will only show up on the ISO9660 portion of the ISO).

```python
iso.add_directory('/DIR1', udf_path='/dir1')
```

Create a new directory on the ISO.  Again we must pass the `udf_path` argument to [add_directory](pycdlib-api.html#PyCdlib-add_directory), for all of the same reasons and with the same restrictions as we saw above for [add_fp](pycdlib-api.html#PyCdlib-add_fp).

```python
iso.write('new.iso')
iso.close()
```

Write the new ISO out to a file, then close out the ISO.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-creating-joliet-iso.html"><-- Example: Creating an ISO with Joliet</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-using-facade.html">Example: Using a facade --></a>
    </div>
</div>
