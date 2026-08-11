# Example: Forcing consistency
As discussed in the example [introduction](examples.md#pycdlib-theory-of-operation), PyCdlib takes a lazy approach to updating metadata.  For performance reasons it is recommended to let PyCdlib handle when and how to update the metadata, but sometimes users need the metadata to be consistent immediately.  PyCdlib offers two solutions for this:

1.  There is an API called [force_consistency](pycdlib-api.html#PyCdlib-force_consistency) that immediately updates all metadata to the latest.
1.  When initially creating the PyCdlib object, the user can set the `always_consistent` parameter.  When this is True, PyCdlib will update the metadata after every operation, ensuring that it is always up-to-date.

Of the two, using lazy metadata updating and only calling [force_consistency](pycdlib-api.html#PyCdlib-force_consistency) when absolutely needed is highly preferred.  Using `always_consistent` is only needed in specialized cases (such as first modifying, then introspecting the extent number that a file exists on the ISO).  The following example will use [force_consistency](pycdlib-api.html#PyCdlib-force_consistency) at a particular point to cause the metadata to be updated.  To learn how to use `always_consistent`, please see the documentation for the `__init__` method for PyCdlib.

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

iso.force_consistency()

iso.add_directory('/DIR1')

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
iso.new()
```

As in earlier examples, create a new PyCdlib object and then create a new ISO with no extensions.

```python
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
```

As in earlier examples, add a new file to the ISO.

```python
iso.force_consistency()
```

Now force consistency on the ISO.  This will cause PyCdlib to update all of the metadata on the ISO, and after this call all of the metadata for all entries on the ISO will be accurate.

```python
iso.add_directory('/DIR1')
```

As in earlier examples, add a new directory to the ISO.  Note that the metadata on the ISO is now out-of-date again, so to accurately look at the metadata, [force_consistency](pycdlib-api.html#PyCdlib-force_consistency) would have to be called again after this modification.

```python
iso.write('new.iso')
```

As in earlier examples, write the ISO out to a file.  The [write](pycdlib-api.html#PyCdlib-write) method implicitly does a metadata update since it needs all of the metadata to be accurate to successfully write out the ISO.

```python
iso.close()
```

As in earlier examples, close the PyCdlib object when we are done with it.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-managing-hard-links.html"><-- Example: Managing hard-links on an ISO</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-hybrid-bootable-iso.html">Example: Creating a "hybrid" bootable ISO --></a>
    </div>
</div>
