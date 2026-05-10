# Example: Walking the ISO filesystem
In some circumstances it may be useful to walk all or some of the filesystem tree on the ISO.  For that purpose, PyCdlib provides the [walk](pycdlib-api.html#PyCdlib-walk) API.  Much like the built-in Python [os.walk](https://docs.python.org/3.6/library/os.html#os.walk) API, this method takes a PyCdlib full path, and iterates over the entire filesystem starting at that root.  For each directory, the method returns a 3-tuple of `(dirpath, dirnames, filenames)` that can be used by the user.  Here's the complete code for this example:

```python
try:
    from cStringIO import StringIO as BytesIO
except ImportError:
    from io import BytesIO

import pycdlib

iso = pycdlib.PyCdlib()
iso.new()

iso.add_directory('/DIR1')
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/DIR1/FOO.;1')
barstr = b'bar\n'
iso.add_fp(BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')
iso.add_directory('/DIR2')
bazstr = b'baz\n'
iso.add_fp(BytesIO(bazstr), len(bazstr), '/DIR2/BAZ.;1')

for dirname, dirlist, filelist in iso.walk(iso_path='/'):
    print("Dirname:", dirname, ", Dirlist:", dirlist, ", Filelist:", filelist)

for dirname, dirlist, filelist in iso.walk(iso_path='/DIR1'):
    print("Dirname:", dirname, ", Dirlist:", dirlist, ", Filelist:", filelist)

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

Create a new PyCdlib object, and then create a new ISO with that object.

```python
iso.add_directory('/DIR1')
foostr = b'foo\n'
iso.add_fp(BytesIO(foostr), len(foostr), '/DIR1/FOO.;1')
barstr = b'bar\n'
iso.add_fp(BytesIO(barstr), len(barstr), '/DIR1/BAR.;1')
iso.add_directory('/DIR2')
bazstr = b'baz\n'
iso.add_fp(BytesIO(bazstr), len(bazstr), '/DIR2/BAZ.;1')
```

As in earlier examples, create some new directories and files on the ISO from strings.

```python
for dirname, dirlist, filelist in iso.walk(iso_path='/'):
    print("Dirname:", dirname, ", Dirlist:", dirlist, ", Filelist:", filelist)
```

Use the [walk](pycdlib-api.html#PyCdlib-walk) API to iterate over the directories and files on the ISO, printing them out in turn.  Note that [walk](pycdlib-api.html#PyCdlib-walk) takes one and only one of `iso_path`, `rr_path`, `joliet_path`, or `udf_path`, and only accepts `rr_path`, `joliet_path`, or `udf_path` if the ISO supports those extensions.  Since we added a number of files and directories on the ISO, the above code will print out:

```python
Dirname: / , Dirlist: ['DIR2', 'DIR1'] , Filelist: []
Dirname: /DIR1 , Dirlist: [] , Filelist: ['FOO.;1', 'BAR.;1']
Dirname: /DIR2 , Dirlist: [] , Filelist: ['BAZ.;1']
```

This can be interpreted as follows.  The "dirname" is the name of the directory currently being examined; for the root it is `/`, while it is `/DIR1` and `/DIR2` for the respective directories.  The "dirlist" is the list of sub-directores at this level of the hierarchy; the root contains `DIR1` and `DIR2`, while each sub-directory has no sub-sub-directories and hence has an empty list.  The "filelist" is the list of files at this level of the hierarchy; the root contains no files so has an empty list, while `DIR1` contains `FOO.;1` and `BAR.;1` and `DIR2` contains `BAZ.;1`.

```python
for dirname, dirlist, filelist in iso.walk(iso_path='/DIR1'):
    print("Dirname:", dirname, ", Dirlist:", dirlist, ", Filelist:", filelist)
```

The second call to [walk](pycdlib-api.html#PyCdlib-walk) starts in the `DIR1` subdirectory, so the output from this loop looks like:

```sh
Dirname: /DIR1 , Dirlist: [] , Filelist: ['FOO.;1', 'BAR.;1']
```

Like we saw before, the "dirname" for this level is `/DIR1`, while the "dirlist" is empty since there are no subdirectories, and the filelist contains `FOO.;1` and `BAR.;1`.

```python
iso.close()
```

Close out the ISO object to release resources and allow it to be reused.

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="example-creating-hybrid-bootable-iso.html"><-- Example: Creating a "hybrid" bootable ISO</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-reading-file-in-chunks.html">Example: Reading a large file in chunks --></a>
    </div>
</div>
