#!/bin/bash
#
# Iterates over current directory, encodes all files with
# MacBinary but ensures that the dates are preserved

for i in *
do
	if test -d "$i" ; then
		cd "$i"
		bash $0 "$1/$i"
		cd ..
	else
		echo -n $1/$i ; echo -ne "... \r"
		macbinary encode "$i"
		touch -r "$i" "$i.bin"
		mv "$i.bin" "$i"
	fi
done

# on the top level we want to print a new line
if test -z $1 ; then
	echo
fi
