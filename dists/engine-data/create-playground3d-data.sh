#!/bin/sh

# Create the directory structure
# Avoided bash shortcuts / file-separators in interest of portability

if [ -e playground3d ]; then
	echo "Game-data already present as playground3d/"
	echo "To regenerate, remove and rerun"
	exit 0
fi

mkdir playground3d

cd playground3d

# For game detection
echo "ScummVM rocks!" > PLAYGROUND3D

# back to the top
cd ..

echo "Game data created"
