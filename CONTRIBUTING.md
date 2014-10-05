Contributing to ScummVM
=======================
Anyone are welcome to post Pull Requests to ScummVM, however, a few things are worth considering before you do so,
and while our Wiki contains a more comprehensive list of details for this matter, a short version is listed here:

Code formatting
---------------
We try to adhere to a set of [code formatting guidelines](http://wiki.scummvm.org/index.php/Code_Formatting_Conventions)

As a quick reminder:
* Tabs for indentation.
* Brackets without newline before them.
* Spacing between tokens.
* Capitalization: `ClassName::functionName, _memberVar, normalVariable`.
* Constants, either `ALL_CAPS_CONSTANT` or `kCamelCaseConstant`.
Do try to follow these as closely as possible.

Commits & Commit Messages
-------------------------
Try to make each commit do as little as possible, many small commits are preferable to
few large.

Commit messages should be descriptive, and must contain a prefix, as defined in our [Commit Guidelines](http://wiki.scummvm.org/index.php/Commit_Guidelines)

Example:
`SUBSYSTEM: Short (50 chars or less) summary of changes`

Portability
-----------
In general:
* Write C++
* Avoid exceptions
* Avoid RTTI 
* Avoid STL
* No inclusion of system headers inside engines.
* File I/O should be performed through our stream APIs in common/.

For details, see our [Coding Conventions](http://wiki.scummvm.org/index.php/Coding_Conventions)

When to ask before changing things
----------------------------------
In general, bug fixes for existing code are always welcome. New functionality is also
appreciated, however, if you change existing functionality, you might want to ask the
maintainers of the engine first (See the [Wiki](http://wiki.scummvm.org/) for details on who that is). Additions to
the common code outside engines/ should be discussed with the core team first. (Use the
mailing list, or #scummvm for this)

Adding a new engine
-------------------
See [HOWTO:Engines](http://wiki.scummvm.org/index.php/HOWTO-Engines) for a guide on how to get started writing
an engine. When you are ready to open a Pull Request for merging though, there is a rather specific 
process for this, see [HOWTO: Engine Inclusion](http://wiki.scummvm.org/index.php/HOWTO-Engine_Inclusion) for details.
