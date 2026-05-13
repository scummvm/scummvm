This directory supports the creation of the CxxTest User Guide using
asciidoc and a2x commands.

HTML

The command

    make html

creates the guide.html file.


PDF

The command

    make pdf

creates the guide.tex file, which generates the guide.pdf file using
dblatex.



EPUB

The command

    make epub

creates the file make.epub.  Note that the `catalog.xml` file is
used, which configures asciidoc to use the docbook XML data in the
`epub` directory.  This is a bit of a hack.  It apparently works
around a limitation of the MacPorts installation of asciidoc.


MANPAGES

The command

    make manpages

creates CxxTest man pages in the doc/man directory.
