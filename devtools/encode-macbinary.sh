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
  $0 [jap] <file.iso>
      Operate in disk dumping mode
	  Optionally specify 'jap' for using 'recode' for converting Japanese file names

Miscellaneous:
  -h, --help   display this help and exit
EOF
}

path=
jap=

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
	mypath=`realpath $0`

	if [[ jap == "jap" ]] ; then
		flist=`hls -F1a|recode SJIS..utf-8`
	else
		flist=`hls -F1a`
	fi

	echo "$flist" | while read i ; do
		if [[ jap == "jap" ]] ; then
			macname=`echo "$i"|recode utf-8..SJIS`
		else
			macname="$i"
		fi

		# Guard empty directories
		if [[ "$i" == "" ]] ; then
			continue
		fi

		if [[ "$i" =~ ":" ]] ; then
			dir="${i%?}"	# Strip trailing ':'
			dir="${dir//\//:}"	# Files could contain '/', replace those with ':'
			macdir="${macname%?}"
			hcd "$macdir"
			mkdir -- "$dir"
			cd -- "$dir"
			bash $mypath $jap hfsutils-phase2 "$path:$i"
			hcd ::
			cd ..
		else
			echo -ne "$path$i...                 \r"
			# Executable files have star at their end. Strip it
			if [[ "$i" =~ \*$ ]] ; then
				macfile="${macname%?}"
				file="${i%?}"
			else
				macfile="$macname"
				file="$i"
			fi
			fileunix="${file//\//:}"	# Files could contain '/', replace those with ':'

			# Files could contain '*', '{', so backslash them to avoid globbing
			macfile="${macfile//\*/\\*}"
			macfile="${macfile//{/\\{}"
			hcopy -m -- "$macfile" "./$fileunix"
		fi
	done
}

for parm in "$@" ; do
	if test "$parm" = "--help" || test "$parm" = "-help" || test "$parm" = "-h" ; then
		usage
		exit 0
	fi
done  # for parm in ...

if [[ $# -eq 0 ]] ; then
	usage
	exit 0
fi

if [[ $1 == "macbinary" ]] ; then
	if ! `command -v macbinary >/dev/null 2>/dev/null` ; then
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

if [[ $1 == "jap" ]] ; then
	if ! `command -v recode >/dev/null 2>/dev/null` ; then
		echo "recode not found. Exiting"
		exit 1
	fi

	jap=jap
	shift
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
