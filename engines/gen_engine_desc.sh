#!/bin/sh

# This file is meant to generate engine.desc files for all engines
# from the engines/configure.engines file.
# As such, it can be considered as a converse of the update_engines
# script, which regenerates engines/configure.engines and various
# other files from the engine.desc files

#
# Function to provide echo_n for bourne shells that don't have it
#
echo_n() {
	printf "$@"
}

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
	set_var _engine_${1}_base "${5}"
	set_var _engine_${1}_deps "${6}"
	for sub in ${4}; do
		set_var _engine_${sub}_sub "yes"
		set_var _engine_${sub}_parent "${1}"
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

# Get the dependencies
get_engine_dependencies() {
	get_var _engine_$1_deps
}

# Get the base engine game support description
get_engine_base() {
	get_var _engine_$1_base
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
#{
#	"description": "Mohawk",
#	"status": "stable",
#	"games": "Living Books",
#	"subengines": {
#		"cstime": {
#			"description": "Where in Time is Carmen Sandiego?",
#			"status": "unstable"
#		},
#		"myst": {
#			"description": "Myst",
#			"status": "unstable",
#			"dependencies": ["16bit"]
#		},
#		"riven": {
#			"description": "Riven: The Sequel to Myst",
#			"status": "unstable",
#			"dependencies": ["16bit"]
#		}
#	}
#}


for engine in $_engines; do
	if test "`get_engine_sub $engine`" = "yes" ; then
		continue;
	fi;

	echo $engine - `get_engine_name $engine`

	# Output filename
	fn=$engine/engine.desc

	echo '{' > $fn
	echo '	"description": "'`get_engine_name $engine`'",' >> $fn
	if test `get_engine_build $engine` = "no" ; then
		echo_n '	"status": "unstable"' >> $fn
	else
		echo_n '	"status": "stable"' >> $fn
	fi
	if test "`get_engine_base $engine`" != ""; then
		echo ',' >> $fn
		echo_n '	"games": "'"`get_engine_base $engine`"'"' >> $fn
	fi
	deps=`get_engine_dependencies $engine`
	if test "$deps" != "" ; then
		echo ',' >> $fn
		echo_n '	"dependencies": [' >> $fn
		first_dep=yes
		for dep in $deps; do
			if test $first_dep = yes; then
				first_dep=no
			else
				echo_n ', ' >> $fn
			fi
			echo_n '"'${dep}'"' >> $fn
		done
		echo_n ']' >> $fn
	fi
	subs=`get_engine_subengines $engine`
	if test "$subs" = "" ; then
		echo '' >> $fn
	else
		echo ',' >> $fn
		echo '	"subengines": {' >> $fn
		first=yes
		for sub in $subs; do
			if test $first = yes; then
				first=no
			else
				echo ',' >> $fn
			fi
			echo '		"'${sub}'": {' >> $fn
			echo '			"description": "'`get_engine_name $sub`'",' >> $fn
			if test `get_engine_build $sub` = "no" ; then
				echo_n '			"status": "unstable"' >> $fn
			else
				echo_n '			"status": "stable"' >> $fn
			fi

			deps=`get_engine_dependencies $sub`
			if test "$deps" != "" ; then
				echo ',' >> $fn
				echo_n '			"dependencies": [' >> $fn
				first_dep=yes
				for dep in $deps; do
					if test $first_dep = yes; then
						first_dep=no
					else
						echo_n ', ' >> $fn
					fi
					echo_n '"'${dep}'"' >> $fn
				done
				echo ']' >> $fn
			else
				echo '' >> $fn
			fi
			echo_n '		}' >> $fn
		done
		echo '' >> $fn
		echo '	}' >> $fn
	fi
	echo '}' >> $fn

done
