# Example: Creating a new, basic ISO
This example will show how to create a new, basic ISO with no extensions.  Here's the complete code for this example:

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

First import the relevant libraries, including pycdlib itself.

```python
iso = pycdlib.PyCdlib()
```

Create a new PyCdlib object.  At this point, the object can only do one of two things: open up an existing ISO, or create a new ISO.

```python
iso.new()
```

Create a new ISO using the [new](pycdlib-api.html#PyCdlib-new) method.  The [new](pycdlib-api.html#PyCdlib-new) method has quite a few available arguments, but by passing no arguments, we ask for a basic interchange level 1 ISO with no extensions.  At this point, we could write out a valid ISO image, but it won't have any files or directories in it, so it wouldn't be very interesting.

```python
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/FOO.;1')
```

Now we add a new file to the ISO.  There are a few details to notice in this code.  The first detail is that there are two related APIs called [add_file](pycdlib-api.html#PyCdlib-add_file) and [add_fp](pycdlib-api.html#PyCdlib-add_fp).  The [add_file](pycdlib-api.html#PyCdlib-add_file) API takes the pathname to a file on the local disk to get the contents from.  The [add_fp](pycdlib-api.html#PyCdlib-add_fp) API takes a file-like object to get the contents from; this can be a normal file-object (such as that returned by standard python [open](https://docs.python.org/3.6/library/functions.html#open)), or this can be any other object that acts like a file.  In this case, we use a python [BytesIO](https://docs.python.org/3/library/io.html#binary-i-o) object, which behaves like a file-object but is backed by a string.  The second detail to notice is that the second argument to [add_fp](pycdlib-api.html#PyCdlib-add_fp) is the length of the content to add to the ISO.  Since file-like objects don't have a standard way to get the length, this must be provided by the user.  The [add_file](pycdlib-api.html#PyCdlib-add_file) API can use the length of the file itself for this purpose, so the second argument isn't required there.  The third detail to notice is that the final argument to [add_fp](pycdlib-api.html#PyCdlib-add_fp) is the location of the file on the resulting ISO (also known as the `iso_path`).  The `iso_path` is specified using something similar to a Unix file path.  These paths differ from Unix file paths in that they *must* be absolute paths, since PyCdlib has no concept of a current working directory.  All intermediate directories along the path must exist, otherwise the [add_fp](pycdlib-api.html#PyCdlib-add_fp) call will fail (the `/` root directory always exists and doesn't have to be explicitly created).  Also note that ISO9660-compliant filenames have a slightly odd format owing to their history.  In standard ISO interchange level 1, filenames have a maximum of 8 characters, followed by a required dot, followed by a maximum 3 character extension, followed by a semicolon and a version.  The filename and the extension are both optional, but one or the other must exist.  Only uppercase letters, numbers, and underscore are allowed for either the name or extension.  If any of these rules are violated, PyCdlib will throw an exception.

```python
iso.add_directory('/DIR1')
```

Here we add a new directory to the ISO called `DIR1`.  Like [add_fp](pycdlib-api.html#PyCdlib-add_fp), the `iso_path` argument to [add_directory](pycdlib-api.html#PyCdlib-add_directory) is an absolute, Unix like pathname.  The rules for ISO directory names are similar to that of filenames, except that directory names do not have extensions and do not have versions.

```python
iso.write('new.iso')
```

Now we finally get to write out the ISO we just created.  The process of writing out an ISO is sometimes called "mastering".  In any case, this is the process of writing the contents of the ISO out to a file on disk.  Similar to the [add_file](pycdlib-api.html#PyCdlib-add_file) and [add_fp](pycdlib-api.html#PyCdlib-add_fp) methods, there are the related [write](pycdlib-api.html#PyCdlib-write) and [write_fp](pycdlib-api.html#PyCdlib-write_fp) methods, the former of which takes a filename to write to, and the latter of which takes a file-like object.

```python
iso.close()
```

Close out the PyCdlib object, releasing all resources and invalidating the contents.  After this call, the object can be reused to create a new ISO or open up an existing ISO.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="examples.html"><-- Examples</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-opening-existing-iso.html">Example: Opening an existing ISO --></a>
    </div>
</div>
