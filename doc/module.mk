DOXYGEN_OUTPUT_DIRECTORY := html
export DOXYGEN_OUTPUT_DIRECTORY

doxygen:
	@echo "Generating reference documentation in $(srcdir)/doc/doxygen/${DOXYGEN_OUTPUT_DIRECTORY}..."
	@cd $(srcdir)/doc/doxygen ; doxygen scummvm.doxyfile

clean-doxygen:
	rm -rf $(srcdir)/doc/doxygen/html
	rm -f $(srcdir)/doc/doxygen/doxygen_warnings.txt

doc: doxygen

clean-doc: clean-doxygen
