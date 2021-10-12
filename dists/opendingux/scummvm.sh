#!/bin/sh

cd `dirname $0`

if [ ! -f "$1" ]; then
	exec ./scummvm 2>&1 >/var/tmp/scummvm.log
fi

path=`dirname "$1"`
file=`basename "$1"`

if [ "$file" == "detect.svm" ]; then
	./scummvm --path="$path" --add --recursive 2>&1 >/var/tmp/scummvm.log
	./scummvm --list-targets |
	sed 's/\//-/g' | sed 's/://g' |
	while IFS=' ' read id title; do
		case $id in
			Target) ;;
			---*) ;;
			*) echo "$id" > "$path/$title.svm" ;;
		esac
	done
else
	gameid=`cat "$1"`
	exec ./scummvm $gameid 2>&1 >/var/tmp/scummvm.log
fi
