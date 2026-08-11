# Example: Using a facade
This example will show how to access various aspects of an ISO with a [facade](examples.md#facades).  Here's the complete code for the example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new(joliet=3)

joliet = iso.get_joliet_facade()

foostr = b'foo\n'
joliet.add_fp(BytesIO(foostr), len(foostr), joliet_path='/foo')

joliet.add_directory(joliet_path='/dir1')

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

Create a new PyCdlib object, and then create a new ISO with that object.  As in previous examples, make it a Joliet ISO by passing `joliet=3` to the [new](pycdlib-api.html#PyCdlib-new) method.

```python
joliet = iso.get_joliet_facade()
```

Fetch a Joliet facade for the PyCdlib object.  We'll use this for manipulation of the object below.

```python
foostr = b'foo\n'
joliet.add_fp(BytesIO(foostr), len(foostr), joliet_path='/foo')
```

As in earlier examples, create a new file on the ISO from a string.  Note that we call the `add_fp` method on the facade, and that we *only* pass the joliet_path to the facade.  This simplifies things considerably for us.  However, this also means that the file will only show up in the Joliet context.  If we want it to show up in other contexts (like Rock Ridge or UDF), we have two options.  We can either use the more complicated form of the `add_fp` method on the original `PyCdlib` object, or we can call `add_hard_link` on the original `PyCdlib` object to make the file appear in other contexts.

```python
joliet.add_directory(joliet_path='/dir1')
```

Create a new directory on the ISO.  Again we use the facade to create the directory, so we only have to pass the `joliet_path`.

```python
iso.write('new.iso')
iso.close()
```

Write the new ISO out to a file, then close out the ISO.  Note that we use the original `PyCdlib` object, and not the facades for this.  Since there is nothing context-specific about these calls, the original object works just fine.

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
      <a href="example-modifying-file-in-place.html">Example: Modifying a file in place --></a>
    </div>
</div>
