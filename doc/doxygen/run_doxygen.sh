#!/bin/bash
DOX_DIR="$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)"
if [ -z "$DOXYGEN_OUTPUT_DIRECTORY" ] ; then
	export DOXYGEN_OUTPUT_DIRECTORY=${DOX_DIR}/html
fi
echo "Generating reference documentation in ${DOXYGEN_OUTPUT_DIRECTORY}..."
cd ${DOX_DIR}
doxygen scummvm.doxyfile
