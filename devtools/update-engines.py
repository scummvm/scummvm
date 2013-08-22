#!/usr/bin/python

import glob
import json
import sys
import re

# Show the tool usage and exit with error code
def show_usage():
	print("Usage:", sys.argv[0], "path/to/engines")
	exit(1)

# Custom string comparison routine for sorting the engines
def engine_name_key(engine):
	class sorter(object):
		NAME = "scumm"
		def __init__(self, obj):
			self.obj = obj

		def __lt__(self, other):
			if self.obj == self.NAME:
				return other != self.NAME
			else:
				return self.obj < other

		def __gt__(self, other):
			if self.obj == self.NAME:
				return False
			else:
				return self.obj > other

		def __eq__(self, other):
			return self.obj == other

		def __le__(self, other):
			if self.obj == self.NAME:
				return True
			else:
				return self.obj > other

		def __ge__(self, other):
			if other == self.NAME:
				return True
			else:
				self.obj >= other

		def __ne__(self, other):
			return self.obj != other
	return sorter(engine)

# Sort engines dictionary. This is a HACK due to difference in sorted between
# Python2 and Python3.
def sort_engines(engines):
	if sys.hexversion >= 0x03000000:
		return sorted(engines, key=engine_name_key)
	else:
		return sorted(engines, lambda x, y: (x == "scumm") and -1 or (y == "scumm") and 1 or cmp(x,y))

# Load a engine.desc file and add it to a dictionary of engines
def load_engine_desc(desc_filename, engines):
	# Extract the engine directory name. This name will be used for the
	# engine define.
	engine = desc_filename[str.rfind(desc_filename, "/", 0, -12)+1:-12]
	with open(desc_filename, "r") as f:
		engines[engine] = json.load(f)
	return engines

# Create an entry for the plugins_table.h file
def create_plugins_table_entry(name, engine):
	line =        "#if PLUGIN_ENABLED_STATIC(" + name.upper() + ")\n"
	line = line + "LINK_PLUGIN(" + name.upper() + ")\n"
	line = line + "#endif\n"
	return line

# Create the plugins_table.h file
def create_plugins_table(engines):
	lines =         "// This file was automatically generated and should NEVER be edited manually!\n"
	lines = lines + "// This file is being included by \"base/plugins.cpp\"\n"
	for engine in sort_engines(engines):
		lines = lines + create_plugins_table_entry(engine, engines[engine])
	return lines

# Create an entry for the engines.mk file
def create_engines_mk_entry(name, engine):
	line =        "ifdef ENABLE_" + name.upper() + "\n"
	line = line + "DEFINES += -DENABLE_" + name.upper() + "=$(ENABLE_" + name.upper() + ")\n"
	line = line + "MODULES += " + engines_path + name + "\n"
	if "subengines" in engine:
		for subengine in sorted(engine["subengines"]):
			line = line + "\n"
			line = line + "ifdef ENABLE_" + subengine.upper() + "\n"
			line = line + "DEFINES += -DENABLE_" + subengine.upper() + "\n"
			line = line + "endif\n"
	line = line + "endif\n"
	return line

# Create the engines.mk file
def create_engines_mk(engines):
	lines = "# This file was automatically generated and should NEVER be edited manually!\n"
	for engine in sort_engines(engines):
		lines = lines + "\n"
		lines = lines + create_engines_mk_entry(engine, engines[engine])
	return lines

# Create an entry for the configure.engines file
def create_configure_engines_entry(name, engine):
	# Add the basic engine name and the description
	line = "add_engine " + name + " \"" + engine["description"] + "\" "
	# Add whether the engine should be build by default or not
	if engine["status"] == "stable":
		line = line + "yes"
	else:
		line = line + "no"
	line = line + " "
	# Add possible subengine names
	if "subengines" in engine:
		line = line + "\"" + " ".join(iter(sorted(engine["subengines"].keys()))) + "\""
	else:
		line = line + "\"\""
	line = line + " "
	# Add possible base games description
	if "games" in engine:
		line = line + "\"" + engine["games"] + "\""
	else:
		line = line + "\"\""
	line = line + " "
	# Add possible dependencies
	if "dependencies" in engine:
		line = line + "\"" + " ".join(engine["dependencies"]) + "\""
	else:
		line = line + "\"\""
	line = line + "\n"
	# Last but not least add all subengines now
	if "subengines" in engine:
		for subengine in sorted(engine["subengines"]):
			line = line + create_configure_engines_entry(subengine, engine["subengines"][subengine])
	return line

# Create the configure.engines file
def create_configure_engines(engines):
	lines =         "# This file was automatically generated and should NEVER be edited manually!\n"
	lines = lines + "# This file is included from the main \"configure\" script\n"
	lines = lines + "# add_engine [name] [desc] [build-by-default] [subengines] [base games] [deps]\n"
	for engine in sort_engines(engines):
		lines = lines + create_configure_engines_entry(engine, engines[engine])
	return lines

# Create translation file list for a specific engine
def create_translation_files_entry(name, engine):
	# In case no translation files are given do nothing
	if not ("translation" in engine):
		return ""
	# Otherwise add all the files given in their proper path
	lines = ""
	for filename in engine["translation"]:
		lines = lines + "engines/" + name + "/" + filename + "\n"
	return lines

# Create the translation file list
def create_translation_files(engines):
	lines = ""
	for engine in sort_engines(engines):
		lines = lines + create_translation_files_entry(engine, engines[engine])
	return lines

# Create the authors section for a specific engine (and its subengines)
def create_credits_team_entry(engine):
	# There might not be special credits in case of sub-engines.
	if "credits" not in engine:
		return ""
	credits = engine["credits"]
	lines = ""
	lines = lines + "\t\t\tbegin_section(\"" + credits["name"] + "\");\n"
	for author in credits["authors"]:
		lines = lines + "\t\t\t\tadd_person(\"" + author["name"] + "\", \"" + author["nick"] + "\", \"" + author["desc"] + "\");\n"
	lines = lines + "\t\t\tend_section();\n"
	lines = lines + "\n"
	# Last but not least add all subengines now
	if "subengines" in engine:
		for subengine in sorted(engine["subengines"]):
			lines = lines + create_credits_team_entry(engine["subengines"][subengine])
	return lines

# Create the authors section for all engines
def create_credits_team(engines):
	lines = ""
	for engine in sort_engines(engines):
		lines = lines + create_credits_team_entry(engines[engine])
	return lines

# Create special thanks paragraph for a specific engine
def create_credits_thanks_entry(engine):
	# There might not be special credits in case of sub-engines.
	if "credits" not in engine:
		return ""
	credits = engine["credits"]
	# There might not be a special thanks section in every engine.
	if "thanks" not in credits:
		return ""
	lines = ""
	lines = lines + "\tadd_paragraph(\n"
	first = True
	for line in credits["thanks"]:
		if first:
			first = False
		else:
			lines = lines + ".\n"
		lines = lines + "\t"
		lines = lines + "\"" + line + "\""
	lines = lines + ");\n"
	lines = lines + "\n"
	# Last but not least add all subengines now
	if "subengines" in engine:
		for subengine in sorted(engine["subengines"]):
			lines = lines + create_credits_thanks_entry(engine["subengines"][subengine])
	return lines

# Create the special thanks paragraphs for all engines
def create_credits_thanks(engines):
	lines = ""
	for engine in sort_engines(engines):
		lines = lines + create_credits_thanks_entry(engines[engine])
	return lines

if len(sys.argv) != 2:
	show_usage()

# Load all engine descriptions present in the engine directory given.
engines = dict()
# Make sure the path always has a '/' at the end
engines_path = sys.argv[1]
if engines_path[-1] != '/':
	engines_path = engines_path + '/'
# Finally load all the engine description files
for desc_filename in glob.iglob(engines_path + "*/engine.desc"):
	engines = load_engine_desc(desc_filename, engines)

# Then create all the files needed by the build system
print("Creating plugins_table.h...")
lines = create_plugins_table(engines)
with open(engines_path + "plugins_table.h", "w") as f:
	f.write(lines)

print("Creating engines.mk...")
lines = create_engines_mk(engines)
with open(engines_path + "engines.mk", "w") as f:
	f.write(lines)

print("Creating configure.engines...")
lines = create_configure_engines(engines)
with open(engines_path + "configure.engines", "w") as f:
	f.write(lines)

# Do replacement of all the misc utilitiy files which depend on engine
# specific data.

# Create the proper POTFILES file
print("Creating po/POTFILES...")
lines = create_translation_files(engines)
with open("po/POTFILES.in", "r") as f:
	p = re.compile("@ENGINE_TRANSLATION_FILES@\n")
	lines = p.sub(lines, f.read())
with open("po/POTFILES", "w") as f:
	f.write("# This file was automatically generated and should NEVER be edited manually!\n")
	f.write(lines)

# Create the credits.pl tool
print("Creating devtools/credits.pl...")
teams = create_credits_team(engines)
thanks = create_credits_thanks(engines)
with open("devtools/credits.pl.in", "r") as f:
	p = re.compile("@ENGINE_TEAMS_CREDITS@\n")
	lines = p.sub(teams, f.read())
	p = re.compile("@ENGINE_THANKS@\n")
	lines = p.sub(thanks, lines)
with open("devtools/credits.pl", "w") as f:
	first = True
	for line in lines.splitlines(True):
		if first:
			f.write(line)
			f.write("# This file was automatically generated and should NEVER be edited manually!\n")
			first = False
		else:
			f.write(line)
