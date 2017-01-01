#!/bin/bash
#
# Iterates over current directory, encodes all files with
# MacBinary but ensures that the dates are preserved

for i in *
do
	macbinary encode "$i"
	touch -r "$i" "$i.bin"
	mv "$i.bin" "$i"
done
