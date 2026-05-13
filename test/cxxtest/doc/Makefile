.PHONY: all anchors outputs clean manpages

export XML_CATALOG_FILES = catalog.xml
PY = python
XSLTPROC_OPTS := --stringparam toc.section.depth 2 --stringparam generate.section.toc.level 1
DBLATEX_OPTS  := -P latex.output.revhistory=0 -P doc.collab.show=1 -P toc.section.depth=2
A2X_OPTS      := -a toc -a icons -L -d article -v --xsltproc-opts "$(XSLTPROC_OPTS)" --dblatex-opts "$(DBLATEX_OPTS)"

%.html: %.txt anchors outputs
	asciidoc -a data-uri -v -b html -d article -n -a toc2 -a icons $<

# Ignore errors (dblatex may not be installed)
%.pdf: %.txt anchors outputs
	- a2x -f pdf $(A2X_OPTS) $<

# Ignore errors
%.epub: %.txt anchors outputs
	- a2x -f epub $(A2X_OPTS) $<

all: guide.html guide.pdf guide.epub

manpages:
	cd man; ./create_manpage;
	cd man; asciidoc -v -d manpage ./cxxtestgen.1.txt

html: guide.html

pdf: guide.pdf

epub: guide.epub

anchors: guide.txt
	$(PY) include_anchors.py $<

outputs:
	../bin/cxxtestgen -h > examples/cxxtestgen.out

clean:
	- \rm -f guide.xml
	- \rm -f examples/.*.py examples/.*.h examples/.*.cpp examples/.*.sh examples/runner examples/TEST*.xml examples/parsetab.py examples/*.orig examples/runner.cpp
