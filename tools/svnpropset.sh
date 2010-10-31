#!/bin/sh

# This script adds common svn properties to files

if [ $# -eq 0 ]; then
        echo "Usage: $0 [FILE]..."
        exit 1
fi

for filename in $@; do
        if [ -f $filename ]; then
                svn propset svn:mime-type text/plain $filename
                svn propset svn:keywords "Date Rev Author URL Id" $filename
                svn propset svn:eol-style native $filename
        elif [ -e $filename ]; then
                echo "$0: error: '$filename' is not a regular file"
        else
                echo "$0: error: '$filename' does not exist"
        fi
done
