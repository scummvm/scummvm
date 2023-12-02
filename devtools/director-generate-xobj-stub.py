#!/usr/bin/env python

import argparse
import os
import re
import struct
from typing import BinaryIO

DIRECTOR_SRC_PATH = os.path.abspath(os.path.join(__file__, "..", "..", "engines", "director"))
MAKEFILE_PATH = os.path.join(DIRECTOR_SRC_PATH, "module.mk")
LINGO_XLIBS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xlibs")
LINGO_OBJECT_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "lingo-object.cpp")

TEMPLATE_H = """/* ScummVM - Graphic Adventure Engine
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
 */

#ifndef DIRECTOR_LINGO_XLIBS_{slug_upper}_H
#define DIRECTOR_LINGO_XLIBS_{slug_upper}_H

namespace Director {{

class {xobject_class} : public Object<{xobject_class}> {{
public:
	{xobject_class}(ObjectType objType);
}};

namespace {xobj_class} {{

extern const char *xlibName;
extern const char *fileNames[];

void open(int type);
void close(int type);

{methlist}

}} // End of namespace {xobj_class}

}} // End of namespace Director

#endif
"""

TEMPLATE_HEADER_METH = """void m_{methname}(int nargs);"""

TEMPLATE = """/* ScummVM - Graphic Adventure Engine
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
 */

#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/{slug}.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
{xmethtable}
 */

namespace Director {{

const char *{xobj_class}::xlibName = "{slug}";
const char *{xobj_class}::fileNames[] = {{
	"{slug}",
	nullptr
}};

static MethodProto xlibMethods[] = {{
{xlib_methods}
	{{ nullptr, nullptr, 0, 0, 0 }}
}};

{xobject_class}::{xobject_class}(ObjectType ObjectType) :Object<{xobject_class}>("{xobj_class}") {{
	_objType = ObjectType;
}}

void {xobj_class}::open(int type) {{
	if (type == kXObj) {{
		{xobject_class}::initMethods(xlibMethods);
		{xobject_class} *xobj = new {xobject_class}(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}} else if (type == kXtraObj) {{
		// TODO - Implement Xtra
	}}
}}

void {xobj_class}::close(int type) {{
	if (type == kXObj) {{
		{xobject_class}::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}} else if (type == kXtraObj) {{
		// TODO - Implement Xtra
	}}
}}

void {xobj_class}::m_new(int nargs) {{
	if (nargs != 0) {{
		warning("{xobj_class}::m_new: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}}
	g_lingo->push(g_lingo->_state->me);
}}

{xobj_stubs}

}}
"""
XLIB_METHOD_TEMPLATE = """	{{ "{methname}",				{xobj_class}::m_{methname},		 {arg_count}, {arg_count},	{director_version} }},"""

XOBJ_STUB_TEMPLATE = """XOBJSTUB({xobj_class}::m_{methname}, {default})"""

XOBJ_NR_STUB_TEMPLATE = """XOBJSTUBNR({xobj_class}::m_{methname})"""

def read_uint8(data: bytes) -> int:
	return struct.unpack('B', data)[0]

def read_uint16_le(data: bytes) -> int:
	return struct.unpack('<H', data)[0]

def read_uint16_be(data: bytes) -> int:
	return struct.unpack('>H', data)[0]

def read_uint32_le(data: bytes) -> int:
	return struct.unpack('<L', data)[0]

def read_uint32_be(data: bytes) -> int:
	return struct.unpack('>L', data)[0]

def extract_xmethtable_macbinary(file: BinaryIO, resource_offset: int, xobj_id: int|None = None) -> tuple[str, str, list[str]]:
	file.seek(resource_offset)
	resource_data_offset = read_uint32_be(file.read(4))
	resource_map_offset = read_uint32_be(file.read(4))
	resource_data_size = read_uint32_be(file.read(4))
	resource_map_size = read_uint32_be(file.read(4))
	file.seek(resource_offset + resource_map_offset + 24)
	type_list_offset = read_uint16_be(file.read(2))
	name_list_offset = read_uint16_be(file.read(2))
	file.seek(resource_offset + resource_map_offset + type_list_offset)
	type_count = read_uint16_be(file.read(2))
	types = {}
	for _ in range(type_count + 1):
		key = file.read(4)
		types[key] = (read_uint16_be(file.read(2)) + 1, read_uint16_be(file.read(2)))
	if b'XCOD' in types:
		print('Found XCOD resources!')
		file.seek(resource_offset + resource_map_offset + type_list_offset + types[b'XCOD'][1])
		resources = []
		for _ in range(types[b'XCOD'][0]):
			id = read_uint16_be(file.read(2))
			name_offset = read_uint16_be(file.read(2))
			file.read(1)
			data_offset = (read_uint8(file.read(1)) << 16) + read_uint16_be(file.read(2))
			file.read(4)
			resources.append((id, data_offset, name_offset))
		xobj = {}
		for id, data_offset, name_offset in resources:
			xobj[id] = {}
			if name_offset != 0xffff:
				file.seek(resource_offset + resource_map_offset + name_list_offset + name_offset)
				name_size = read_uint8(file.read(1))
				xobj[id]["name"] = file.read(name_size).decode("macroman")
			else:
				xobj[id]["name"] = "<unknown>"
			file.seek(resource_offset + resource_data_offset + data_offset)
			size = read_uint32_be(file.read(4)) - 12
			file.read(12)
			xobj[id]["xmethtable"] = []
			while size > 0:
				count = read_uint8(file.read(1))
				if count == 0:
					break
				xobj[id]["xmethtable"].append(file.read(count).decode('macroman'))
				size -= 1 + count
		if xobj_id is None or xobj_id not in xobj:
			print("Please re-run with one of the following XOBJ resource IDs:")
			for id, data in xobj.items():
				print(f"{id} - {data['name']}")
			raise ValueError("Need to specify XOBJ resource ID")
		for entry in xobj[xobj_id]["xmethtable"]:
			print(entry)
		return (xobj[xobj_id]["name"], xobj[xobj_id]["name"].lower(), xobj[xobj_id]["xmethtable"])

	raise ValueError("No XCOD resources found!")

def extract_xmethtable_win16(file: BinaryIO, ne_offset: int) -> tuple[str, str, list[str]]:
	# get resource table
	file.seek(ne_offset + 0x24, os.SEEK_SET)
	restable_offset = read_uint16_le(file.read(0x2))
	resident_names_offset = read_uint16_le(file.read(0x2))
	file.seek(ne_offset + restable_offset)
	shift_count = read_uint16_le(file.read(0x2))
	# read each resource
	resources = []
	while file.tell() < ne_offset + resident_names_offset:
		type_id = read_uint16_le(file.read(0x2)) # should be 0x800a for XMETHTABLE
		if type_id == 0:
			break
		count = read_uint16_le(file.read(0x2))
		file.read(0x4) # reserved
		entries = []
		for i in range(count):
			file_offset = read_uint16_le(file.read(0x2))
			file_length = read_uint16_le(file.read(0x2))
			entries.append(dict(
				offset=file_offset << shift_count,
				length=file_length << shift_count
			))
			file.read(0x2) # flagword
			file.read(0x2) # resource_id
			file.read(0x2) # handle
			file.read(0x2) # usage
		resources.append(dict(
			type_id=type_id,
			entries=entries
		))
	resource_names = []
	while file.tell() < ne_offset + resident_names_offset:
		length = read_uint8(file.read(0x1))
		if length == 0:
			break
		resource_names.append(file.read(length).decode('ASCII'))

	print("Resources found:")
	print(resources, resource_names)

	xmethtable_exists = "XMETHTABLE" in resource_names
	file.seek(ne_offset + resident_names_offset)
	name_length = read_uint8(file.read(0x1))
	file_name = file.read(name_length).decode('ASCII')

	# Borland C++ can put the XMETHTABLE token into a weird nonstandard resource
	for x in filter(lambda d: d["type_id"] == 0x800f, resources):
		for y in x["entries"]:
			file.seek(y["offset"], os.SEEK_SET)
			data = file.read(y["length"])
			xmethtable_exists |= b"XMETHTABLE" in data

	if not xmethtable_exists:
		raise ValueError("XMETHTABLE not found!")


	resources = list(filter(lambda x: x["type_id"] == 0x800a, resources))
	if len(resources) != 1:
		raise ValueError("Expected a single matching resource type entry!")

	xmethtable_offset = resources[0]["entries"][0]["offset"]
	xmethtable_length = resources[0]["entries"][0]["length"]
	print(f"Found XMETHTABLE for XObject library {file_name}!")
	file.seek(xmethtable_offset, os.SEEK_SET)
	xmethtable_raw = file.read(xmethtable_length)
	xmethtable = [entry.decode('iso-8859-1') for entry in xmethtable_raw.strip(b"\x00").split(b"\x00")]
	for entry in xmethtable:
		print(entry)
	library_name = xmethtable[1]
	xmethtable[1] = "--" + library_name
	return library_name, file_name, xmethtable


def extract_xmethtable_win32(file: BinaryIO, pe_offset: int) -> tuple[str, list[str]]:
	# get the .data section
	# find a string b"msgTable\x00", get the offset
	# get the .text section
	# find assembly:
	# 68 [ addr ] 6a 00 6a [ addr 2 ]
	# lookup addr2 in .data
	# get c string, split by \x0a

	return ("", [])

def extract_xmethtable(path: str, resid: int) -> tuple[str, str, list[str]]:
	with open(path, 'rb') as file:
		magic = file.read(0x2)
		if magic == b'MZ':
			file.seek(0x3c, os.SEEK_SET)
			header_offset = read_uint16_le(file.read(0x2))
			file.seek(header_offset, os.SEEK_SET)
			magic = file.read(0x2)
			if magic == b'NE':
				print("Found Win16 NE DLL!")
				return extract_xmethtable_win16(file, header_offset)
			elif magic == b'PE':
				raise ValueError("No support yet for extracting from Win32 DLLs")
		file.seek(0)
		header = file.read(124)
		if len(header) == 124 and header[0] == 0 and header[74] == 0 and header[82] == 0 and header[122] in [129, 130] and header[123] in [129, 130]:
			print("Found MacBinary!")

			data_size = read_uint32_be(header[83:87])
			resource_size = read_uint32_be(header[87:91])
			resource_offset = 128 + data_size + ((128 - (data_size % 128)) if (data_size % 128) else 0)
			print(f"resource offset: {resource_offset}")
			return extract_xmethtable_macbinary(file, resource_offset, resid)

	raise ValueError("Unknown filetype")


def generate_stubs(xmethtable: list[str], slug: str, name: str, director_version: int=400, dry_run=False) -> None:
	meths = []
	for e in xmethtable:
		if not e.strip():
			break
		elems = e.split()
		if not elems or elems[0].startswith('--'):
			continue
		returnval = elems[0][0]
		args = elems[0][1:]
		methname = elems[1].split(",")[0]
		if methname.startswith('+'):
			methname = methname[1:]
		if methname.startswith('m'):
			methname = methname[1].lower() + methname[2:]
		meths.append(dict(
			methname=methname,
			args=args,
			arg_count=len(args),
			returnval=returnval,
			default= '""' if returnval == "S" else "0"
		))

	xobject_class = f"{name}XObject"
	xobj_class = f"{name}XObj"

	cpp_text = TEMPLATE.format(
		slug=slug,
		xmethtable="\n".join(xmethtable),

		xobject_class=xobject_class,
		xobj_class=xobj_class,
		xlib_methods="\n".join([XLIB_METHOD_TEMPLATE.format(
			xobj_class=xobj_class,
			director_version=director_version,
			**x
		) for x in meths]),
		xobj_stubs="\n".join([
			XOBJ_NR_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
			if x["returnval"] == "X" else
			XOBJ_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
			for x in meths if x["methname"] != "new"
		]),
	)
	if dry_run:
		print("C++ output:")
		print(cpp_text)
		print()
	else:
		with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.cpp"), "w") as cpp:
			cpp.write(cpp_text)

	header_text = TEMPLATE_H.format(
		slug_upper=slug.upper(),
		xobject_class=f"{name}XObject",
		xobj_class=f"{name}XObj",
		methlist="\n".join([TEMPLATE_HEADER_METH.format(**x) for x in meths]),
	)
	if dry_run:
		print("Header output:")
		print(header_text)
		print()
	else:
		with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.h"), "w") as header:
			header.write(header_text)


def main() -> None:
	parser = argparse.ArgumentParser(
		description="Extract the method table from a Macromedia Director XObject/XLib and generate method stubs."
	)
	parser.add_argument("XOBJ_FILE", help="XObject/XLib file to test")
	parser.add_argument("--resid", help="XOBJ resource ID (for MacBinary)", type=int, default=None)
	parser.add_argument("--slug", help="Slug to use for files (e.g. {slug}.cpp, {slug}.h)")
	parser.add_argument("--name", help="Base name to use for classes (e.g. {name}XObj, {name}XObject)")
	parser.add_argument("--version", metavar="VER", help="Minimum Director version (default: 400)", default="400")
	parser.add_argument("--dry-run", help="Test only, don't write files", action='store_true')
	args = parser.parse_args()

	library_name, file_name, xmethtable = extract_xmethtable(args.XOBJ_FILE, args.resid)
	slug = args.slug or file_name
	name = args.name or library_name
	generate_stubs(xmethtable, slug, name, args.version, args.dry_run)


if __name__ == "__main__":
	main()
