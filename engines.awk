#!/bin/awk -f

function get_values(var, arr) {
	return split(ENVIRON[var], arr, " ")
}

# Add a line of data to config.mk.
function add_line_to_config_mk(line) {
	print(line) >> config_mk
}

function add_to_config_h_if_yes(value, define) {
	if (value == "yes")
		print(define) >> config_h
	else
		print("/* " define " */") >> config_h
}

#
# Feature handling functions
#

# Get the name of the feature
function get_feature_name(feature) {
	return ENVIRON["_feature_" feature "_name"]
}

# Check whether the feature is enabled
function get_feature_state(feature) {
	get_values("_feature_" feature "_settings", settings)
	for (i in settings) {
		if (ENVIRON[settings[i]] == "yes")
			return "yes"
	}
	return "no"
}


#
# Engine handling functions
#

# Get the name of the engine
function get_engine_name(engine) {
	return ENVIRON["_engine_" engine "_name"]
}

# Will this engine be built?
function get_engine_build(engine) {
	return ENVIRON["_engine_" engine "_build"]
}

# Was this engine set to be built by default?
function get_engine_build_default(engine) {
	return ENVIRON["_engine_" engine "_build_default"]
}

# Get the subengines
function get_engine_subengines(engine, subengines) {
	return get_values("_engine_" engine "_subengines", subengines)
}

# Get the dependencies
function get_engine_dependencies(engine, deps) {
	return get_values("_engine_" engine "_deps", deps)
}

# Get the base engine game support description
function get_engine_base(engine) {
	return ENVIRON["_engine_" engine "_base"]
}

# Ask if this is a subengine
function get_engine_sub(engine) {
	subeng = ENVIRON["_engine_" engine "_sub"]
	if (subeng == "")
		subeng = "no"
	return subeng
}

# Get a subengine's parent (undefined for non-subengines)
function get_subengine_parent(engine) {
	return ENVIRON["_engine_" engine "_parent"]
}

function disable_engine(engine) {
	ENVIRON["_engine_" engine "_build"] = "no"
}

function check_engine_deps(engine) {
	unmet_deps = ""

	# Check whether the engine is enabled
	if (get_engine_build(engine) != "no") {
		# Collect unmet dependencies
		depcount = get_engine_dependencies(engine, deps)
		for (d = 1; d <= depcount; d++) {
			if (get_feature_state(deps[d]) == "no")
				unmet_deps = unmet_deps get_feature_name(deps[d]) " "
		}

		# Check whether there is any unmet dependency
		if (unmet_deps) {
			print("WARNING: Disabling engine " get_engine_name(engine) " because the following dependencies are unmet: " unmet_deps)
			disable_engine(engine)
		}
	}
}

# Prepare the strings about the engines to build
function prepare_engine_build_strings(engine) {
	if (string = get_engine_build_string(engine, "static"))
		_engines_built_static[++_static] = string

	if (string = get_engine_build_string(engine, "dynamic"))
		_engines_built_dynamic[++_dynamic] = string

	if (string = get_engine_build_string(engine, "no"))
		_engines_skipped[++_skipped] = string

	if (string = get_engine_build_string(engine, "wip"))
		_engines_built_wip[++_wip] = string
}

# Get the string about building an engine
function get_engine_build_string(engine, request_status) {
	engine_build = get_engine_build(engine)
	engine_build_default = get_engine_build_default(engine)
	show = 0

	# Convert static/dynamic to yes to ease the check of subengines
	if (engine_build == "no")
		subengine_filter = "no"
	else
		subengine_filter = "yes"

	# Check if the current engine should be shown for the current status
	if (engine_build == request_status) {
		show = 1
	} else {
		# Test for disabled sub-engines
		if (request_status == "no") {
			get_engine_subengines(engine, subengines)
			for (subeng in subengines) {
				if (get_engine_build(subengines[subeng]) == "no") {
					# In this case we to display _disabled_ subengines
					subengine_filter = "no"
					show = 1
					break
				}
			}
		}
		# Test for enabled wip sub-engines
		if (request_status == "wip") {
			get_engine_subengines(engine, subengines)
			for (subeng in subengines) {
				if (get_engine_build(subengines[subeng]) != "no" && get_engine_build_default(subengines[subeng]) == "no") {
					show = 1
					break
				}
			}
		}
	}


	# Check if it is a wip engine
	if (request_status == "wip" && engine_build != "no" && engine_build_default == "no")
		show = 1

	# The engine should be shown, build the string
	if (show)
		return get_engine_name(engine) " " get_subengines_build_string(engine, subengine_filter, request_status)
}

# Get the string about building subengines
function get_subengines_build_string(parent_engine, subengine_filter, request_status) {
	parent_engine_build_default = get_engine_build_default(parent_engine)
	subengine_string = ""

	# If the base engine isn't built at all, no need to list subengines
	# in any of the possible categories.
	if (get_engine_build(parent_engine) == "no")
		return

	all = 1
	subeng_count = get_engine_subengines(parent_engine, subengines)
	# If there are no subengines, never display "[all games]" (for brevity).
	if (!subeng_count)
		all = 0
	# If the base engine does not fit the category we're displaying here
	# (WIP or Skipped), we should never show "[all games]"
	if (request_status == "wip" && parent_engine_build_default == "yes")
		all = 0
	if (request_status == "no") {
		# If we're here, the parent engine is built, so no need to check that.
		all = 0
	}


	# In the static/dynamic categories, also display the engine's base games.
	if (subeng_count && request_status != "no" && request_status != "wip")
		subengine_string = "[" get_engine_base(parent_engine) "]"

	for (s = 1; s <= subeng_count; s++) {
		subeng = subengines[s]
		subengine_build = get_engine_build(subeng)
		subengine_build_default = get_engine_build_default(subeng)

		# Display this subengine if it matches the filter, unless it is
		# a stable subengine in the WIP request.
		if ((subengine_build == subengine_filter) && !(request_status == "wip" && subengine_build_default == "yes")) {
			name = "[" get_engine_name(subeng) "]"
			if (subengine_string)
				subengine_string = subengine_string " " name
			else
				subengine_string = name
		} else {
			all = 0
		}
	}

	# Summarize the full list, where applicable
	if (all)
		subengine_string = "[all games]"

	return subengine_string
}

function print_engines(headline, engines, count) {
	if (!count)
		return
	print("\n" headline)
	for (e = 1; e <= count; e++)
		print("    " engines[e])
}

BEGIN {
	config_mk = "config.mk.engines"
	config_h = "config.h.engines"
	# Clear previous contents if any
	printf("") > config_h
	printf("") > config_mk
}

END {
	engine_count = get_values("_engines", engines)
	for (e = 1; e <= engine_count; e++) {
		engine = engines[e]
		check_engine_deps(engine)
		if (get_engine_sub(engine) == "no") {
			# It's a main engine
			if (get_engine_build(engine) == "no") {
				isbuilt = "no"
			} else {
				# If dynamic plugins aren't supported, mark
				# all the engines as static
				if (ENVIRON["_dynamic_modules"] == "no") {
					ENVIRON["_engine_" engine "_build"] = "static"
				} else {
					# If it wasn't explicitly marked as static or
					# dynamic, use the configured default
					if (get_engine_build(engine) == "yes")
						ENVIRON["_engine_" engine "_build"] = ENVIRON["_plugins_default"]
				}

				# Prepare the defines
				if (get_engine_build(engine) == "dynamic") {
					isbuilt = "DYNAMIC_PLUGIN"
				} else {
					ENVIRON["_engine_" engine "_build"] = "static"
					isbuilt = "STATIC_PLUGIN"
				}
			}
		} else {
			# It's a subengine, just say yes or no
			if (get_engine_build(engine) == "no")
				isbuilt = "no"
			else
				isbuilt = "1"
		}

		# Save the settings
		defname = "ENABLE_" toupper(engine)
		if (isbuilt == "no")
			add_line_to_config_mk("# " defname)
		else
			add_line_to_config_mk(defname " = " isbuilt)
	}

	# Sort engines to place our headline engine at start...
	# No technical reason, just historical convention
	headline_engine = "scumm"
	sorted_engines[++sorted] = headline_engine
	for (e = 1; e <= engine_count; e++) {
		if (engines[e] != headline_engine)
			sorted_engines[++sorted] = engines[e]
	}

	# Prepare the information to be shown
	for (e = 1; e <= engine_count; e++) {
		engine = sorted_engines[e]
		if (get_engine_sub(engine) == "no") {
			# It's a main engine
			prepare_engine_build_strings(engine)
		}
	}

	#
	# Detection of WIP/unstable engines
	#
	for (e in engines) {
		engine = engines[e]
		if (get_engine_build(engine) != "no" && get_engine_build_default(engine) == "no") {
			_tainted_build = "yes"
			break
		}
	}

	add_to_config_h_if_yes(_tainted_build, "#define TAINTED_BUILD")
	print_engines("Engines (builtin):", _engines_built_static, _static)
	print_engines("Engines (plugins):", _engines_built_dynamic, _dynamic)
	print_engines("Engines Skipped:", _engines_skipped, _skipped)
	print_engines("WARNING: This ScummVM build contains the following UNSTABLE engines:", _engines_built_wip, _wip)

	# Ensure engines folder exists prior to trying to generate
	# files into it (used for out-of-tree-builds)
	system("mkdir -p engines")

	print("")
	print("Creating engines/engines.mk")
	engines_mk = "engines/engines.mk.new"
	print("# This file is automatically generated by configure\n" \
	      "# DO NOT EDIT MANUALLY\n" \
	      "# This file is being included by \"Makefile.common\"") > engines_mk

	for (e = 1; e <= engine_count; e++) {
		engine = sorted_engines[e]
		j = toupper(engine)
		if (get_engine_sub(engine) == "no") {
			# main engine
			print("\n" \
			      "ifdef ENABLE_" j "\n" \
			      "DEFINES += -DENABLE_" j "=$(ENABLE_" j ")\n" \
			      "MODULES += engines/" engine) >> engines_mk

			subeng_count = get_engine_subengines(engine, subengines)
			for (s = 1; s <= subeng_count; s++) {
				k = toupper(subengines[s])
				print("\n" \
				      "ifdef ENABLE_" k "\n" \
				      "DEFINES += -DENABLE_" k "\n" \
				      "endif") >> engines_mk
			}

			print("endif") >> engines_mk
		}
	}

	# Name which is suffixed to each detection plugin
	detectId = "_DETECTION"
	det_table = "engines/detection_table.h.new"

	print("Creating engines/detection_table.h")
	print("/* This file is automatically generated by configure */\n" \
	      "/* DO NOT EDIT MANUALLY */\n" \
	      "// This file is being included by \"base/plugins.cpp\"") > det_table

	for (e = 1; e <= engine_count; e++) {
		engine = sorted_engines[e]
		if (get_engine_sub(engine) == "no") {
			j = toupper(engine)
			detectEngine = j detectId
			print("#if defined(ENABLE_" j ") || defined(DETECTION_FULL)\n" \
			      "LINK_PLUGIN(" j detectId ")\n" \
			      "#endif") >> det_table
		}
	}

	plugins_table = "engines/plugins_table.h.new"
	print("Creating engines/plugins_table.h")
	print("/* This file is automatically generated by configure */\n" \
	      "/* DO NOT EDIT MANUALLY */\n" \
	      "// This file is being included by \"base/plugins.cpp\"") > plugins_table

	for (e = 1; e <= engine_count; e++) {
		engine = sorted_engines[e]
		if (get_engine_sub(engine) == "no") {
			j = toupper(engine)
			print("#if PLUGIN_ENABLED_STATIC(" j ")\n" \
			      "LINK_PLUGIN(" j ")\n" \
			      "#endif") >> plugins_table
		}
	}
}
