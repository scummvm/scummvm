#!/usr/bin/env python

import argparse
import os
import re
import sys

HELPTEXT = \
'''A script to add a new C++ class to an engine.  Creates the .cpp and .h
with copyright notices, header guards, and namespace, and updates module.mk.'''

# This copyright notice is used by the script but also applies to this file.
COPYRIGHT = \
'''/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */'''

def get_engine_path(engname):
	engname = engname.lower()
	curpath = os.path.realpath(os.path.curdir)
	pathelems = curpath.split(os.path.sep)

	result = None

	if 'engines' in pathelems:
		# already in engines
		result = os.path.join(*(pathelems[:pathelems.index('engines') + 1] + [engname]))
	elif 'devtools' in pathelems:
		result = os.path.join(*(pathelems[:pathelems.index('devtools')] + ['engines', engname]))
	elif 'scummvm' in pathelems:
		result = os.path.join(*(pathelems[:pathelems.index('scummvm') + 1] + ['engines', engname]))

	if result and curpath.startswith('/'):
		# put the / back on the start
		result = '/' + result

	if not result or not os.path.isdir(result):
		result = None

	return result

CPP_TEMPLATE = \
'''{copyright}

#include "{headerpath}/{headerfilename}"

namespace {namespace} {{

{classname}::{classname}() {{
}}

// TODO: Add more functions here.

}} // end namespace {namespace}
'''

H_TEMPLATE = \
'''{copyright}

#ifndef {headername}
#define {headername}

namespace {namespace} {{

class {classname} {{
public:
	{classname}();

	// TODO add public members

private:
	// TODO add private members

}};

}} // end namespace {namespace}

#endif // {headername}
'''

def class_to_fbase(classname):
	'''Create class_file_pattern from camelcase classname'''
	return re.sub('([A-Z])', '_\\1', classname).lower()[1:]


def create_files(destpath, classname, namespace, enginename):
	'''Create the cpp and h files from the templates'''
	fbase = class_to_fbase(classname)
	fname_cpp = os.path.join(destpath, fbase + '.cpp')
	fname_h = os.path.join(destpath, fbase + '.h')
	headerpath = destpath[destpath.index(enginename):]

	headername = headerpath.upper().replace('/', '_') + '_' + fbase.upper() + '_H'

	if os.path.exists(fname_cpp):
		print('ERROR: cpp file %s already exists' % fname_cpp, file=sys.stderr)
		return False
	if os.path.exists(fname_h):
		print('ERROR: header file %s already exists' % fname_h, file=sys.stderr)
		return False

	cppstr = CPP_TEMPLATE.format(copyright=COPYRIGHT, classname=classname,
				namespace=namespace, headerpath=headerpath,
				headerfilename=(fbase+'.h'))

	hstr = H_TEMPLATE.format(copyright=COPYRIGHT, classname=classname,
				namespace=namespace, headername=headername)

	print("Creating %s" % fname_cpp)
	with open(fname_cpp, 'w') as cppfile:
		cppfile.write(cppstr)
	print("Creating %s" % fname_h)
	with open(fname_h, 'w') as hfile:
		hfile.write(hstr)

	return True

high_sort_key = chr(0x7e)

def module_sort_key(mod):
	'''
	Sort key for .o files in module.mk

	Files in a directory always before sub-dir contents.
	Alphabetical order otherwise.
	'''
	parts = mod.split('/')
	nparts = len(parts)
	key = ''
	for i in range(nparts):
		if i < nparts - 1:
			key += high_sort_key + parts[i] + '/'
		else:
			key += parts[i]
	return key

def update_module_mk(module_mk_path, classname, subpath):
	'''Add the new object file to module.mk's obj file list'''
	fbase = class_to_fbase(classname)

	o_file_path = fbase + '.o'
	if subpath != '.':
		o_file_path = os.path.join(subpath, o_file_path)

	module_mk_contents = open(module_mk_path).read()
	module_mk_contents = module_mk_contents.split('\n')

	mod_obs_start = -1
	mod_obs_end = -1
	for lineno, line in enumerate(module_mk_contents):
		if line.startswith('MODULE_OBJS := '):
			mod_obs_start = lineno
			break
	if mod_obs_start == -1:
		print('ERROR: MODULE_OBJS line not found in %s?' % module_mk_path)
		return False

	for lineno, line in enumerate(module_mk_contents[mod_obs_start:]):
		if not line:
			mod_obs_end = lineno
			break
	if mod_obs_end == -1:
		print('ERROR: MODULE_OBJS never ends in %s?' % module_mk_path)
		return False

	before = module_mk_contents[:mod_obs_start+1]
	modules = module_mk_contents[mod_obs_start+1:mod_obs_end]
	after = module_mk_contents[mod_obs_end:]

	modules.append('\t%s \\' % o_file_path)
	modules.sort(key=module_sort_key)

	print("Adding %s to module.mk" % o_file_path)
	open(module_mk_path, 'w').write('\n'.join(before + modules + after))

	return True


def main():
	parser = argparse.ArgumentParser(description=HELPTEXT)
	parser.add_argument('engine', help='name of the engine')
	parser.add_argument('subpath', help='subpath within the engine (use \'.\' to add to the engine root)')
	parser.add_argument('classname', help='CamelCase name of the class to add')
	parser.add_argument('--namespace', '-n', help='Namespace tp use (default is name of engine with a capital letter start)')

	args = parser.parse_args()

	engpath = get_engine_path(args.engine)
	if not engpath:
		parser.error("Can't find path for engine %s. Run inside scummvm checkout and ensure engine exists" % (args.engine,))

	module_mk = os.path.join(engpath, 'module.mk')
	if not os.path.exists(module_mk):
		parser.error("Can't find module.mk inside engine %s, help!" % (args.engine,))

	destpath = engpath
	if not args.subpath == '.':
		destpath = destpath + os.path.sep + args.subpath
	if not os.path.isdir(destpath):
		parser.error("Subdir %s doesn't exist in engine %s, try making it first." % (args.subpath, args.engine,))

	if not args.classname[0].isupper():
		parser.error("Class name %s should be CamelCase" % (args.classname,))

	namespace = args.namespace or args.engine.capitalize()
	if not create_files(destpath, args.classname, namespace, args.engine):
		return 1

	if not update_module_mk(module_mk, args.classname, args.subpath):
		return 1

	return 0

if __name__ == '__main__':
	sys.exit(main())
