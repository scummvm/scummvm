#!/bin/bash

usage() {
			cat << EOF
Usage: $0 [OPTIONS]...

Dumping Mac files into MacBinary format

There are 2 operation modes. Direct MacBinary encoding (Mac-only) and dumping ISO
contents with hfsutils.

Mode 1:
  $0 macbinary
      Operate in MacBinary encoding mode

Mode 2:
  $0 <file.iso>
      Operate in disk dumping mode

Miscellaneous:
  -h, --help   display this help and exit
EOF
}

path=

macbinarydump() {
	mypath=`realpath $0`

	for i in *
	do
		if test -d "$i" ; then
			cd "$i"
			if [ "$(ls -A .)" ] ; then # directory is not empty
				bash $mypath macbinary-phase2 "$path/$i"
			fi
			cd ..
		else
			echo -ne "$path/$i...                 \r"
			macbinary encode "$i"
			touch -r "$i" "$i.bin"
			mv "$i.bin" "$i"
		fi
	done
}

hfsdump() {
	IFS=$'\n'
	mypath=`realpath $0`

	for i in `hls -F1a`
	do
		if [[ "$i" =~ ":" ]] ; then
			dir="${i%?}"
			hcd "$dir"
			mkdir "$dir"
			cd "$dir"
			bash $mypath hfsutils-phase2 "$path:$i"
			hcd ::
			cd ..
		else
			echo -ne "$path$i...                 \r"
			# Executable files have star at their end. Strip it
			if [[ "$i" =~ \*$ ]] ; then
				file="${i%?}"
			else
				file="$i"
			fi
			fileunix="$file"
			# Files count contain stars
			file="${file//\*/\\*}"
			hcopy -m "$file" "./$fileunix"
		fi
	done
}

for parm in "$@" ; do
	if test "$parm" = "--help" || test "$parm" = "-help" || test "$parm" = "-h" ; then
		usage
		exit 0
	fi
done  # for parm in ...

if [[ $1 == "macbinary" ]] ; then
	if test ! `type macbinary >/dev/null 2>/dev/null` ; then
		echo "macbinary not found. Exiting"
		exit 1
	fi
	macbinarydump
	echo
	exit 0
fi

if [[ $1 == "macbinary-phase2" ]] ; then
	path=$2
	macbinarydump
	exit 0
fi

###########
# hfsutils mode
if  [ "$#" -lt 1 ] ; then
	usage
	exit 1
fi

if [[ $1 == "hfsutils-phase2" ]] ; then
	path=$2
	hfsdump
	exit 0
fi

if ! `type hmount >/dev/null 2>/dev/null` ; then
	echo "hfsutils not found. Exiting"
	exit 1
fi

isofile="$1"

echo -n "Mounting ISO..."
hmount "$isofile" >/dev/null 2>/dev/null
if test -z $? ; then
	echo error
	exit 1
fi

echo done

echo "Dumping..."

hfsdump
echo

echo -n "Unmounting ISO..."
humount >/dev/null 2>/dev/null
if test -z $? ; then
	echo error
	exit 1
fi

echo done
