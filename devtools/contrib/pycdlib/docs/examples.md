# Examples
The easiest way to learn PyCdlib is to see some examples.  Before getting to the examples, we first start out with the PyCdlib theory of operation.

## PyCdlib theory of operation
PyCdlib aims to allow users to manipulate ISOs in arbitrary ways, from creating new ISOs to modifying and writing out existing ISOs.  Along the way, the PyCdlib API is meant to hide the details of the above standards, letting users concentrate on the modifications they wish to make.

To start using the PyCdlib API, the user must create a new PyCdlib object.  A PyCdlib object cannot do very much until it is initialized, either by creating a new ISO (using the [new](pycdlib-api.html#PyCdlib-new) method), or by opening an existing ISO (using the [open](pycdlib-api.html#PyCdlib-open) method).  Once a PyCdlib object is initialized, files can be added or removed, directories can be added or removed, the ISO can be made bootable, and various other manipulations of the ISO can happen.  Once the user is happy with the current layout of the ISO, the [write](pycdlib-api.html#PyCdlib-write) method can be called, which will write out the current state of the ISO to a file (or file-like object).

Due to some historical aspects of the ISO standards, making modifications to an existing ISO can involve shuffling around a lot of metadata.  In order to maintain decent performance, PyCdlib takes a "lazy" approach to updating that metadata, and only does the update when it needs the results.  This allows the user to make several modifications and effectively "batch" operations without significantly impacting speed.  The minor downside to this is that the metadata stored in the PyCdlib object is not always consistent, so if the user wants to reach into the object to look at a particular field, it may not always be up-to-date.  PyCdlib offers a [force\_consistency](pycdlib-api.html#PyCdlib-force_consistency) API that immediately updates all metadata for just this reason.

## Facades
The base PyCdlib object hides many of the details of the ISO and related standards, but still has some complexity to allow arbitrary modifications to be made.  The "facades", on the other hand, sacrifice some of this flexibility for an even simpler interface that allows the user to operate in just one of the contexts.  The benefit is that the user can just work in the context they prefer, and the facade will handle the details of compatibility with the other contexts.  The downside is that making modifications via the facades only modifies the context that the facade is for, leaving it out of the other contexts.  For instance, if the "PyCdlibJoliet" facade is being used, and a file is added via "add_fp", then that file will *only* show up in the Joliet context.  This means that it won't be visible by default to Unix operating systems (which generally prefer to use the Rock Ridge context).

## Example format
We'll start out each example with the entire source code needed to run the example, and then break down the code to show what the individual pieces do.  Note that in most cases, error handling is elided for brevity, though it probably shouldn't be in real code.

* [Creating a new, basic ISO](example-creating-new-basic-iso.md)
* [Opening an existing ISO](example-opening-existing-iso.md)
* [Extracting data from an existing ISO](example-extracting-data-from-iso.md)
* [Creating a bootable ISO (El Torito)](example-creating-bootable-iso.md)
* [Creating an ISO with Rock Ridge](example-creating-rock-ridge-iso.md)
* [Creating an ISO with Joliet](example-creating-joliet-iso.md)
* [Creating an ISO with UDF](example-creating-udf-iso.md)
* [Using a facade](example-using-facade.md)
* [Modifying a file in place](example-modifying-file-in-place.md)
* [Managing hard-links on an ISO](example-managing-hard-links.md)
* [Forcing consistency](example-forcing-consistency.md)
* [Creating a "hybrid" bootable ISO](example-creating-hybrid-bootable-iso.md)
* [Walking the ISO filesystem](example-walking-iso-filesystem.md)
* [Reading a large file in chunks](example-reading-file-in-chunks.md)

---

<div style="width: 100%; display: table;">
  <div style="display: table-row;">
    <div style="width: 33%; display: table-cell; text-align: left;">
      <a href="python-compatibility.html"><-- Python compatibility</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: center;">
      <a href="https://clalancette.github.io/pycdlib/">Top</a>
    </div>
    <div style="width: 33%; display: table-cell; text-align: right;">
      <a href="example-creating-new-basic-iso.html">Example: Creating a new, basic ISO --></a>
    </div>
</div>
