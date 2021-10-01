#!/bin/sh
#
# This shell script is used to simplify building with the Docker images
# used by the buildbot.

DIR="$(cd "$(dirname "$0")/.." && pwd)"

_image=
_update=no

for ac_option in $@; do
	case "$ac_option" in
	--update)
		_update=yes
		;;
	--*)
		echo "Unknown argument '$ac_option'"
		exit 1
		;;
	*)
		if [ -n "$_image" ]; then
			echo "A toolchain image has already been specified"
			exit 1
		fi
		_image=$ac_option
		;;
	esac;
done;

if [ -z "$_image" ]; then
	echo "No toolchain image has been specified"
	exit 1
fi

if test "$_update" != yes ; then
	if ! docker image inspect $_image >/dev/null 2>&1 ; then
		echo "Image is not available"
		_update=yes
	fi
fi

if test "$_update" = yes ; then
	docker_url=`echo $_image | sed 's/\([^\/]*\)\/\([^\/]*\)/scummvm\/dockerized-\1:\2/'`
	(docker pull $docker_url && docker tag $docker_url $_image && docker rmi $docker_url) || exit 1
fi

docker run -v $DIR:/data/scummvm -w /data/scummvm -it $_image /bin/bash
