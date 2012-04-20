#!/usr/bin/env python2.7

# This scripts reads all the engine.desc files in JSON format, and then
# (re)generates various files from that:
# - configure.engines
# - plugins_table.h
# - engines.mk

import glob
import json


# read all engines
engines=dict()
for edesc_name in glob.iglob('*/engine.desc'):
	engine = edesc_name[0:-12]
	with open(edesc_name, 'r') as f:
		engines[engine] = json.load(f)

# HACK: In order to sort SCUMM first, we pass a key filter to sorted()
def scumm_first_key(engine):
	if engine == "scumm":
		return "_scumm"
	return engine

# Regenerate plugins_table.h
print "Regenerating plugins_table.h..."
with open("plugins_table.h", 'w') as f:
	f.write("// This file was automatically generated and should NEVER be edited manually!\n")
	f.write("// To regenerate it, run the update-engines.py script\n")
	for engine in sorted(engines, key=scumm_first_key):
		# 
		f.write("#if PLUGIN_ENABLED_STATIC(" + engine.upper() + ")\n")
		f.write("LINK_PLUGIN(" + engine.upper() + ")\n")
		f.write("#endif\n")

# Regenerate engines.mk
print "Regenerating engines.mk..."
with open("engines.mk", 'w') as f:
	f.write("# This file was automatically generated and should NEVER be edited manually!\n")
	f.write("# To regenerate it, run the update-engines.py script\n")
	for engine in sorted(engines, key=scumm_first_key):
		# 
		f.write("\n")
		f.write("ifdef ENABLE_" + engine.upper() + "\n")
		f.write("DEFINES += -DENABLE_" + engine.upper() + "=$(ENABLE_" + engine.upper() + ")\n")
		f.write("MODULES += engines/" + engine + "\n")
		for sub in sorted(engines[engine].get("subengines", dict())):
			f.write("\n")
			f.write("ifdef ENABLE_" + sub.upper() + "\n")
			f.write("DEFINES += -DENABLE_" + sub.upper() + "\n")
			f.write("endif\n")
		f.write("endif\n")
