#!/bin/sh

# Uncomment this to debug the script
# set -x

DIR=$1

help()
{
	echo "Scummvm wrapper by Bastien Nocera <hadess@hadess.net>"
	echo "Usage:"
	echo "         `basename $0` [dir|file]"
	echo "Options:"
	echo "         dir   path to a directory containing a supported game"
	echo "         file  one of the file in a directory containing a supported game"
	echo
	exit
}

if [ $# -ne 1 ] ; then help ; fi
if [ $1 == "-h" ] ; then help ; fi
if [ ! -d $1 ] ; then\
	if [ -e $1 ] ; then
		DIR=`dirname $1`
	else
		help
	fi
fi

# Here we check for which game we're gonna be running
if [ -e $DIR/*.000 ] ; then
	GAME=`basename $DIR/*.000 | sed 's,.000$,,g' | grep -v '*'`
	if [ x$GAME = 'x' ] ; then
		echo "*** Game not found ***"
		help
	fi
fi


# Check for the game, see if it's supported
if [ -z $GAME ] ; then
	echo "*** Unsupported game ***"
	help
fi

# Go to the game's dir
cd $DIR

# Launch scummvm with the game name
/usr/lib/scummvm/scummvm $GAME

