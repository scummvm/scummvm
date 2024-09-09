#!/bin/bash
#
# opk_make.sh
#
# This script is meant to ease generation of a opk file. Please consult the output
# when running --help for a list of available parameters and an explanation of
# those.
#
# Required tools when running the script:
# bash
# echo, cat, mv, rm, mksquashfs

check_for_tool()
{
	which $1 &> /dev/null
	if [ "$?" -ne "0" ];
	then
		cecho "ERROR: Could not find the program '$1'. Please make sure
that it is available in your PATH since it is required to complete your request." $red
		exit 1
	fi
}

print_help()
{
	cat << EOSTREAM
opk_make.sh - A script to package "something" into a OPK.

Usage:
  $(basename ${0}) {--directory|-d} <folder> {--opk|-o} <file> [{--help|-h}]


Switches:
  --directory / -d          Sets the folder that is to be used for the resulting opk
                            to <folder>. This option is mandatory for the script to
                            function correctly.

  --help / -h               Displays this help text.

  --opkname / -o            Sets the output filename of the resulting opk to <file>.
                            This option is mandatory for the script to function
                            correctly.

A version >=4.0 of squashfs is required to be available in your PATH.
EOSTREAM
}


# Parse command line parameters
while [ "${1}" != "" ]; do
	if [ "${1}" = "--directory" ] || [ "${1}" = "-d" ];
	then
		FOLDER=$2
		shift 2
	elif [ "${1}" = "--help" ] || [ "${1}" = "-h" ];
	then
		print_help
		exit 0
	elif [ "${1}" = "--opkname" ] || [ "${1}" = "-o" ];
	then
		OPKNAME=$2
		shift 2
	else
		echo "ERROR: '$1' is not a known argument. Printing --help and aborting."
		print_help
		exit 1
	fi
done


# Probe if required variables were set
echo "Checking if all required variables were set."
if [ ! $OPKNAME ] || [ ! $FOLDER ];
then
	echo "ERROR: Not all required options were set! Please see the --help information below."
	print_help
	exit 1
else
	echo "OPKNAME set to '$OPKNAME'."
fi
# Check if the selected folder actually exists
if [ ! -d $FOLDER ];
then
	echo "ERROR: '$FOLDER' doesn't exist or is not a folder."
	exit 1
else
	echo "FOLDER set to '$FOLDER'."
fi

# Make iso from folder
echo "Creating an iso file based on '$FOLDER'."

check_for_tool mksquashfs
if [ $(mksquashfs -version | awk 'BEGIN{r=0} $3>=4{r=1} END{print r}') -eq 0 ];
then
	echo "ERROR: Your squashfs version is older then version 4, please upgrade to 4.0 or later"
	exit 1
fi
mksquashfs $FOLDER $OPKNAME.opk -noappend -no-exports -no-xattrs

# Final message
if [ -f $OPKNAME ];
then
	echo "Successfully finished creating the opk '$OPKNAME'."
else
	echo "There seems to have been a problem and '$OPKNAME' was not created. Please check
the output above for any error messages. A possible cause for this is that there was
not enough space available."
	exit 1
fi

