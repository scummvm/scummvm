#!/bin/sh

# This file is meant to generate engine.desc files for all engines
# from the engines/configure.engines file.
# As such, it can be considered as a converse of the update_engines
# script, which regenerates engines/configure.engines and various
# other files from the engine.desc files

set_var() {
	eval ${1}='${2}'
}

get_var() {
	eval echo \$${1}
}

# Add an engine: id name build subengines
add_engine() {
	_engines="${_engines} ${1}"
	if test "${3}" = "no" ; then
		set_var _wip_engines "${_wip_engines} ${1}"
	fi
	set_var _engine_${1}_name "${2}"
	set_var _engine_${1}_build "${3}"
	set_var _engine_${1}_build_default "${3}"
	set_var _engine_${1}_subengines "${4}"
	for sub in ${4}; do
		set_var _engine_${sub}_sub "yes"
	done
}

# Get the name of the engine
get_engine_name() {
	get_var _engine_$1_name
}

# Will this engine be built?
get_engine_build() {
	get_var _engine_$1_build
}

# Was this engine set to be built by default?
get_engine_build_default() {
	get_var _engine_$1_build_default
}

# Get the subengines
get_engine_subengines() {
	get_var _engine_$1_subengines
}

# Ask if this is a subengine
get_engine_sub() {
	sub=`get_var _engine_$1_sub`
	if test -z "$sub" ; then
		sub=no
	fi
	echo $sub
}

# Read list of engines
. configure.engines


# Sample output:
# {
#   "desc": "Legend of Kyrandia",
#   "status": "stable",
#   "subengines": {
#     "lol": {
#       "desc": "Lands of Lore"
#     },
#     "eob": {
#       "desc": "Eye of the Beholder",
#       "status": "wip"
#     }
#   }
# }


for engine in $_engines; do
	if test "`get_engine_sub $engine`" = "yes" ; then
		continue;
	fi;

	echo $engine - `get_engine_name $engine`

	# Output filename
	fn=$engine/engine.desc

	echo '{' > $fn
	echo '  "desc": "'`get_engine_name $engine`'",' >> $fn
	if test `get_engine_build $engine` = "no" ; then
		/bin/echo -n '  "status": "wip"' >> $fn
	else
		/bin/echo -n '  "status": "stable"' >> $fn
	fi
	subs=`get_engine_subengines $engine`
	if test "$subs" = "" ; then
		echo '' >> $fn
	else
		echo ',' >> $fn
		echo '  "subengines": {' >> $fn
		first=yes
		for sub in $subs; do
			if test $first = yes; then
				first=no
			else
				echo ',' >> $fn
			fi
			echo '    "'${sub}'": {' >> $fn
			echo '      "desc": "'`get_engine_name $sub`'",' >> $fn
			# TODO: Output subengine status only if it differs from parent?
			if test `get_engine_build $sub` = "no" ; then
				/bin/echo '      "status": "wip"' >> $fn
			else
				/bin/echo '      "status": "stable"' >> $fn
			fi
			/bin/echo -n '    }' >> $fn
		done
		echo '' >> $fn
		echo '  }' >> $fn
	fi
	echo '}' >> $fn

done
