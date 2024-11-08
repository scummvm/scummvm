#!/usr/bin/env python

from __future__ import annotations

import argparse
import os
import re
import struct
from typing import Any, BinaryIO, Literal
from typing_extensions import TypedDict

XCodeType = Literal["XFCN", "XCMD", "XObject", "Xtra"]


class XCode(TypedDict):
    type: XCodeType
    name: str
    slug: str
    filename: str
    method_table: list[str]

class PESection(TypedDict):
    name: str
    virt_size: int
    virt_addr: int
    raw_size: int
    raw_ptr: int


DIRECTOR_SRC_PATH = os.path.abspath(
    os.path.join(__file__, "..", "..", "engines", "director")
)
MAKEFILE_PATH = os.path.join(DIRECTOR_SRC_PATH, "module.mk")
LINGO_XLIBS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xlibs")
LINGO_XTRAS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xtras")
LINGO_OBJECT_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "lingo-object.cpp")

LEGAL = """/* ScummVM - Graphic Adventure Engine
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
"""

TEMPLATE_H = (
    LEGAL
    + """
#ifndef DIRECTOR_LINGO_{base_upper}_{slug_upper}_H
#define DIRECTOR_LINGO_{base_upper}_{slug_upper}_H

namespace Director {{

class {xobject_class} : public Object<{xobject_class}> {{
public:
	{xobject_class}(ObjectType objType);{xtra_props_h}
}};

namespace {xobj_class} {{

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

{methlist}

}} // End of namespace {xobj_class}

}} // End of namespace Director

#endif
"""
)

TEMPLATE_HEADER_METH = """void m_{methname}(int nargs);"""

TEMPLATE = (
    LEGAL
    + """
#include "common/system.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/{base}/{slug}.h"

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

const char *{xobj_class}::xlibName = "{name}";
const XlibFileDesc {xobj_class}::fileNames[] = {{
	{{ "{filename}",   nullptr }},
	{{ nullptr,        nullptr }},
}};

static MethodProto xlibMethods[] = {{
{xlib_methods}
	{{ nullptr, nullptr, 0, 0, 0 }}
}};

static BuiltinProto xlibBuiltins[] = {{
{xlib_builtins}
	{{ nullptr, nullptr, 0, 0, 0, VOIDSYM }}
}};

{xobject_class}::{xobject_class}(ObjectType ObjectType) :Object<{xobject_class}>("{name}") {{
	_objType = ObjectType;
}}{xtra_props}

void {xobj_class}::open(ObjectType type, const Common::Path &path) {{
    {xobject_class}::initMethods(xlibMethods);
    {xobject_class} *xobj = new {xobject_class}(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}}

void {xobj_class}::close(ObjectType type) {{
    {xobject_class}::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}}

{xobj_new}

{xobj_stubs}

}}
"""
)
XLIB_METHOD_TEMPLATE = """	{{ "{methname}",				{xobj_class}::m_{methname},		 {min_args}, {max_args},	{director_version} }},"""
XLIB_NEW_TEMPLATE = """void {xobj_class}::m_new(int nargs) {{
	g_lingo->printSTUBWithArglist("{xobj_class}::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}}"""


# XTRA PROPS TEMPLATE and Header contains extra newline at the beginning.
# This keeps the newlines correct when `TEMPLATE` is used for xlibs.
XTRA_PROPS_TEMPLATE = """

bool {xobject_class}::hasProp(const Common::String &propName) {{
	return (propName == "name");
}}

Datum {xobject_class}::getProp(const Common::String &propName) {{
	if (propName == "name")
		return Datum({xobj_class}::xlibName);
	warning("{xobj_class}::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}}"""

XTRA_PROPS_H = """

	bool hasProp(const Common::String &propName) override;
	Datum getProp(const Common::String &propName) override;"""


XCMD_TEMPLATE_H = (
    LEGAL
    + """
#ifndef DIRECTOR_LINGO_XLIBS_{slug_upper}_H
#define DIRECTOR_LINGO_XLIBS_{slug_upper}_H

namespace Director {{

namespace {xobj_class} {{

extern const char *xlibName;
extern const XlibFileDesc fileNames[];

void open(ObjectType type, const Common::Path &path);
void close(ObjectType type);

{methlist}

}} // End of namespace {xobj_class}

}} // End of namespace Director

#endif
"""
)

XCMD_TEMPLATE = (
    LEGAL
    + """
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

namespace Director {{

const char *{xobj_class}::xlibName = "{name}";
const XlibFileDesc {xobj_class}::fileNames[] = {{
	{{ "{filename}", nullptr }},
	{{ nullptr, nullptr }}
}};

static BuiltinProto builtins[] = {{
{xlib_builtins}
	{{ nullptr, nullptr, 0, 0, 0, VOIDSYM }}
}};

void {xobj_class}::open(ObjectType type, const Common::Path &path) {{
	g_lingo->initBuiltIns(builtins);
}}

void {xobj_class}::close(ObjectType type) {{
	g_lingo->cleanupBuiltIns(builtins);
}}

{xobj_stubs}

}}
"""
)

BUILTIN_TEMPLATE = """	{{ "{name}", {xobj_class}::m_{name}, {min_args}, {max_args}, {director_version}, {methtype} }},"""

XOBJ_STUB_TEMPLATE = """XOBJSTUB({xobj_class}::m_{methname}, {default})"""

XOBJ_NR_STUB_TEMPLATE = """XOBJSTUBNR({xobj_class}::m_{methname})"""


def read_uint8(data: bytes) -> int:
    return struct.unpack("B", data)[0]


def read_uint16_le(data: bytes) -> int:
    return struct.unpack("<H", data)[0]


def read_uint16_be(data: bytes) -> int:
    return struct.unpack(">H", data)[0]


def read_uint32_le(data: bytes) -> int:
    return struct.unpack("<L", data)[0]


def read_uint32_be(data: bytes) -> int:
    return struct.unpack(">L", data)[0]


def inject_makefile(slug: str, xcode_type: XCodeType) -> None:
    make_contents = open(MAKEFILE_PATH, "r").readlines()
    storage_path = "lingo/xtras" if xcode_type == "Xtra" else "lingo/xlibs"
    expr = re.compile(f"^\t{storage_path}/([a-zA-Z0-9\\-]+).o( \\\\|)")
    for i in range(len(make_contents)):
        m = expr.match(make_contents[i])
        if m:
            if slug == m.group(1):
                # file already in makefile
                print(f"{storage_path}/{slug}.o already in {MAKEFILE_PATH}, skipping")
                return
            elif slug < m.group(1):
                make_contents.insert(i, f"\t{storage_path}/{slug}.o \\\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return
            elif m.group(2) == "":
                # final entry in the list
                make_contents[i] += " \\"
                make_contents.insert(i + 1, f"\t{storage_path}/{slug}.o\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return


def inject_lingo_object(slug: str, xobj_class: str, director_version: int, xcode_type: XCodeType) -> None:
    # write include statement for the object header
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    storage_path = "director/lingo/xtras" if xcode_type == "Xtra" else "director/lingo/xlibs"
    obj_type = "kXtraObj" if xcode_type == "Xtra" else "kXObj"
    expr = re.compile(f'^#include "{storage_path}/([a-zA-Z0-9\\-]+)\\.h"')
    in_xlibs = False
    for i in range(len(lo_contents)):
        m = expr.match(lo_contents[i])
        if m:
            in_xlibs = True
            if slug == m.group(1):
                print(
                    f"{storage_path}/{slug}.h import already in {LINGO_OBJECT_PATH}, skipping"
                )
                break
            elif slug < m.group(1):
                lo_contents.insert(i, f'#include "{storage_path}/{slug}.h"\n')
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(i, f'#include "{storage_path}/{slug}.h"\n')
            with open(LINGO_OBJECT_PATH, "w") as f:
                f.writelines(lo_contents)
                break

    # write entry in the XLibProto table
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    expr = re.compile("^\tXLIBDEF\\(([a-zA-Z0-9_]+),")
    in_xlibs = False
    for i in range(len(lo_contents)):
        m = expr.match(lo_contents[i])
        if m:
            in_xlibs = True
            if xobj_class == m.group(1):
                print(
                    f"{xobj_class} proto import already in {LINGO_OBJECT_PATH}, skipping"
                )
                break
            elif xobj_class < m.group(1):
                lo_contents.insert(
                    i,
                    f"	XLIBDEF({xobj_class},			{obj_type},					{director_version}),	// D{director_version // 100}\n",
                )
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(
                i,
                f"	XLIBDEF({xobj_class},			{obj_type},					{director_version}),	// D{director_version // 100}\n",
            )
            with open(LINGO_OBJECT_PATH, "w") as f:
                f.writelines(lo_contents)
                break


def extract_xcode_macbinary(
    file: BinaryIO, resource_offset: int, xobj_id: str | None = None
) -> XCode:
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
    xobj: dict[str, dict[str, Any]] = {}
    for chunk_type in [b"XCOD", b"XFCN", b"XCMD"]:
        if chunk_type in types:
            print(f"Found {chunk_type.decode('utf8')} resources!")
            file.seek(
                resource_offset
                + resource_map_offset
                + type_list_offset
                + types[chunk_type][1]
            )
            resources: list[tuple[str, int, int]] = []
            for _ in range(types[chunk_type][0]):
                id = f"{chunk_type.decode('utf8')}_{read_uint16_be(file.read(2))}"
                name_offset = read_uint16_be(file.read(2))
                file.read(1)
                data_offset = (read_uint8(file.read(1)) << 16) + read_uint16_be(
                    file.read(2)
                )
                file.read(4)
                resources.append((id, data_offset, name_offset))
            for id, data_offset, name_offset in resources:
                xobj[id] = {}
                if name_offset != 0xFFFF:
                    file.seek(
                        resource_offset
                        + resource_map_offset
                        + name_list_offset
                        + name_offset
                    )
                    name_size = read_uint8(file.read(1))
                    xobj[id]["name"] = file.read(name_size).decode("macroman")
                else:
                    xobj[id]["name"] = "<unknown>"
                file.seek(resource_offset + resource_data_offset + data_offset)
                xobj[id]["dump"] = file.read(read_uint32_be(file.read(4)) - 4)
                file.seek(resource_offset + resource_data_offset + data_offset)
                size = read_uint32_be(file.read(4)) - 12
                file.read(12)
                xobj[id]["xmethtable"] = []
                while size > 0:
                    count = read_uint8(file.read(1))
                    if count == 0:
                        break
                    xobj[id]["xmethtable"].append(file.read(count).decode("macroman"))
                    size -= 1 + count
    if not xobj:
        raise ValueError("No extension resources found!")

    if xobj_id is None or xobj_id not in xobj:
        print("Please re-run with one of the following resource IDs:")
        for id, data in xobj.items():
            print(f"{id} - {data['name']}")
        raise ValueError("Need to specify resource ID")
    type: XCodeType = (
        "XFCN"
        if xobj_id.startswith("XFCN_")
        else "XCMD"
        if xobj_id.startswith("XCMD_")
        else "XObject"
    )
    if type == "XObject":
        for entry in xobj[xobj_id]["xmethtable"]:
            print(entry)
    slug = xobj[xobj_id]["name"].lower()
    if type in ["XFCN", "XCMD"]:
        slug += type.lower()
    return {
        "type": type,
        "name": xobj[xobj_id]["name"],
        "slug": slug,
        "filename": xobj[xobj_id]["name"],
        "method_table": xobj[xobj_id]["xmethtable"],
    }


def extract_xcode_win16(file: BinaryIO, ne_offset: int) -> XCode:
    # get resource table
    file.seek(ne_offset + 0x24, os.SEEK_SET)
    restable_offset = read_uint16_le(file.read(0x2))
    resident_names_offset = read_uint16_le(file.read(0x2))
    file.seek(ne_offset + restable_offset)
    shift_count = read_uint16_le(file.read(0x2))
    # read each resource
    resources: list[dict[str, Any]] = []
    while file.tell() < ne_offset + resident_names_offset:
        type_id = read_uint16_le(file.read(0x2))  # should be 0x800a for XMETHTABLE
        if type_id == 0:
            break
        count = read_uint16_le(file.read(0x2))
        file.read(0x4)  # reserved
        entries = []
        for i in range(count):
            file_offset = read_uint16_le(file.read(0x2))
            file_length = read_uint16_le(file.read(0x2))
            entries.append(
                dict(
                    offset=file_offset << shift_count, length=file_length << shift_count
                )
            )
            file.read(0x2)  # flagword
            file.read(0x2)  # resource_id
            file.read(0x2)  # handle
            file.read(0x2)  # usage
        resources.append(dict(type_id=type_id, entries=entries))
    resource_names = []
    while file.tell() < ne_offset + resident_names_offset:
        length = read_uint8(file.read(0x1))
        if length == 0:
            break
        resource_names.append(file.read(length).decode("ASCII"))

    print("Resources found:")
    print(resources, resource_names)

    xmethtable_exists = "XMETHTABLE" in resource_names
    file.seek(ne_offset + resident_names_offset)
    name_length = read_uint8(file.read(0x1))
    file_name = file.read(name_length).decode("ASCII")

    # Borland C++ can put the XMETHTABLE token into a weird nonstandard resource
    for x in filter(lambda d: d["type_id"] == 0x800F, resources):
        for y in x["entries"]:
            file.seek(y["offset"], os.SEEK_SET)
            data = file.read(y["length"])
            xmethtable_exists |= b"XMETHTABLE" in data

    if not xmethtable_exists:
        raise ValueError("XMETHTABLE not found!")

    resources = list(filter(lambda x: x["type_id"] == 0x800A, resources))
    if len(resources) != 1:
        raise ValueError("Expected a single matching resource type entry!")

    xmethtable_offset = resources[0]["entries"][0]["offset"]
    xmethtable_length = resources[0]["entries"][0]["length"]
    print(f"Found XMETHTABLE for XObject library {file_name}!")
    file.seek(xmethtable_offset, os.SEEK_SET)
    xmethtable_raw = file.read(xmethtable_length)
    xmethtable = [
        entry.decode("iso-8859-1")
        for entry in xmethtable_raw.strip(b"\x00").split(b"\x00")
    ]
    for entry in xmethtable:
        print(entry)
    library_name = xmethtable[1]
    xmethtable[1] = "--" + library_name
    return {
        "type": "XObject",
        "name": library_name,
        "slug": file_name.lower(),
        "filename": file_name,
        "method_table": xmethtable,
    }


def extract_xcode_win32(file: BinaryIO, pe_offset: int) -> XCode:
    file.seek(pe_offset + 4)

    # read the COFF Header, perform basic sanity checks
    machine_type = read_uint16_le(file.read(0x2))
    if machine_type != 0x14c:
        raise ValueError(f"PE file is not 32-bit Intel x86")
    section_count = read_uint16_le(file.read(0x2))
    file.seek(12, os.SEEK_CUR)
    optional_size = read_uint16_le(file.read(0x2))
    characteristics = read_uint16_le(file.read(0x2))
    if not (characteristics & 0x2000):
        raise ValueError("DLL flag not set")
    if not (characteristics & 0x0100):
        raise ValueError("32-bit flag not set")

    # read the Optional Header to get the image base address
    optional = file.read(optional_size)
    image_base = 0
    if read_uint16_le(optional[0:2]) == 0x10b:
        image_base = read_uint32_le(optional[28:32])
        print(f"Found PE32, image base {image_base:08x}")
    elif read_uint16_le(optional[0:2]) == 0x20b:
        raise ValueError("PE32+ not supported")
    else:
        raise ValueError("Unknown optional header magic number")

    # read each Section Header from the Section Table
    sections: dict[str, PESection] = {}
    for i in range(section_count):
        segment: PESection = {
            "name": file.read(0x8).strip(b'\x00').decode('utf8'),
            "virt_size": read_uint32_le(file.read(0x4)),
            "virt_addr": read_uint32_le(file.read(0x4)),
            "raw_size": read_uint32_le(file.read(0x4)),
            "raw_ptr": read_uint32_le(file.read(0x4)),
        }
        file.seek(16, os.SEEK_CUR)
        sections[segment["name"]] = segment
        print(f"{segment['name']}: {segment['virt_addr']:08x} {segment['virt_size']:08x}")

    # grab the .text section; this contains the program instructions
    if ".text" not in sections:
        raise ValueError(".text section not found")
    file.seek(sections[".text"]["raw_ptr"])
    code = file.read(sections[".text"]["raw_size"])

    # Lingo Xtras are COM libraries with a generic calling API.
    # Director discovers what functions are available by requesting
    # a msgTable, which unfortunately for us is done with code.
    # We need to find the following x86 assembly:
    # 68 [ u32 addr 1 ]  ; push offset "msgTable"
    # 6a 00              ; push 0
    # 68 [ u32 addr 2 ]  ; push offset xtra_methtable
    # 6a 09              ; push 9

    instr = re.compile(rb"\x68(.{4})\x6a\x00\x68(.{4})\x6a\x09", flags=re.DOTALL)
    methtable_found = False
    methtable = []
    for msgtable_raw, methtable_raw in instr.findall(code):
        # should be the offset to the string "msgTable"
        msgtable_offset = read_uint32_le(msgtable_raw) - image_base
        # should be the offset to the full method table
        methtable_offset = read_uint32_le(methtable_raw) - image_base
        msgtable_found = False
        for s in sections.values():
            if msgtable_offset in range(s["virt_addr"], s["virt_addr"]+s["virt_size"]):
                file.seek(s["raw_ptr"])
                data = file.read(s["raw_size"])
                start = msgtable_offset - s["virt_addr"]
                end = data.find(b"\x00", start)
                if data[start:end] != b"msgTable":
                    continue
                print(f"Found msgTable!")
                msgtable_found = True
        if not msgtable_found:
            continue
        # If we found the text "msgTable" at the first address, we know we've found the right call.
        for s in sections.values():
            if methtable_offset in range(s["virt_addr"], s["virt_addr"]+s["virt_size"]):
                file.seek(s["raw_ptr"])
                data = file.read(s["raw_size"])
                start = methtable_offset - s["virt_addr"]
                end = data.find(b"\x00", start)
                methtable_found = True
                methtable = data[start:end].decode('iso-8859-1').split('\n')
    if not methtable_found:
        raise ValueError("Could not find msgTable!")

    for entry in methtable:
        print(entry)

    library_name = methtable[0].split()[1].capitalize()
    methtable[0] = "-- " + methtable[0]

    return {
        "type": "Xtra",
        "name": library_name,
        "slug": library_name.lower(),
        "filename": library_name.lower(),
        "method_table": methtable
    }


def extract_xcode(path: str, resid: str) -> XCode:
    with open(path, "rb") as file:
        magic = file.read(0x2)
        if magic == b"MZ":
            file.seek(0x3C, os.SEEK_SET)
            header_offset = read_uint16_le(file.read(0x2))
            file.seek(header_offset, os.SEEK_SET)
            magic = file.read(0x2)
            if magic == b"NE":
                print("Found Win16 NE DLL!")
                return extract_xcode_win16(file, header_offset)
            elif magic == b"PE":
                print("Found Win32 PE DLL!")
                return extract_xcode_win32(file, header_offset)
        file.seek(0)
        header = file.read(124)
        if (
            len(header) == 124
            and header[0] == 0
            and header[74] == 0
            and header[82] == 0
            and header[122] in [129, 130]
            and header[123] in [129, 130]
        ):
            print("Found MacBinary!")

            data_size = read_uint32_be(header[83:87])
            resource_size = read_uint32_be(header[87:91])
            resource_offset = (
                128
                + data_size
                + ((128 - (data_size % 128)) if (data_size % 128) else 0)
            )
            print(f"resource offset: {resource_offset}")
            return extract_xcode_macbinary(file, resource_offset, resid)

    raise ValueError("Unknown filetype")


def generate_xobject_stubs(
    xmethtable: list[str],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 400,
    dry_run: bool = False,
) -> None:
    meths = []
    for e in xmethtable:
        if not e.strip():
            break
        elems = e.split()
        if not elems or elems[0].startswith("--"):
            continue

        first = elems[0]
        if first.startswith("/"):
            first = first[1:]
        returnval = first[0]
        args = first[1:]
        methname = elems[1].split(",")[0]
        if methname.startswith("+"):
            methname = methname[1:]
        if methname.startswith("m"):
            methname = methname[1].lower() + methname[2:]
        meths.append(
            dict(
                methname=methname,
                args=args,
                min_args=len(args),
                max_args=len(args),
                returnval=returnval,
                default='""' if returnval == "S" else "0",
            )
        )
    xobject_class = f"{name}XObject"
    xobj_class = f"{name}XObj"

    cpp_text = TEMPLATE.format(
        base="xlibs",
        slug=slug,
        name=name,
        filename=filename,
        xmethtable="\n".join(xmethtable),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xlib_builtins="",
        xlib_toplevels="",
        xlib_methods="\n".join(
            [
                XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
                )
                for x in meths
            ]
        ),
        xtra_props="",
		xobj_new=XLIB_NEW_TEMPLATE.format(xobj_class=xobj_class),
        xobj_stubs="\n".join(
            [
                XOBJ_NR_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                if x["returnval"] == "X"
                else XOBJ_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                for x in meths
                if x["methname"] != "new"
            ]
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = TEMPLATE_H.format(
        base_upper="XLIBS",
        slug_upper=slug.upper(),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xtra_props_h="",
        methlist="\n".join([TEMPLATE_HEADER_METH.format(**x) for x in meths]),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, "XObject")
        inject_lingo_object(slug, xobj_class, director_version, "XObject")


def generate_xcmd_stubs(
    type: Literal["XCMD", "XFCN"],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 400,
    dry_run: bool = False,
) -> None:
    xobj_class = f"{name}{type}"
    methtype = "CBLTIN" if type == "XCMD" else "HBLTIN"
    cpp_text = XCMD_TEMPLATE.format(
        slug=slug,
        name=name,
        filename=filename,
        xobj_class=xobj_class,
        xlib_builtins=BUILTIN_TEMPLATE.format(
            name=name,
            xobj_class=xobj_class,
            min_args=-1,
            max_args=0,
            director_version=director_version,
            methtype=methtype,
        ),
        xobj_stubs=XOBJ_STUB_TEMPLATE.format(
            xobj_class=xobj_class, methname=name, default=0
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = XCMD_TEMPLATE_H.format(
        slug_upper=slug.upper(),
        xobj_class=xobj_class,
        methlist=TEMPLATE_HEADER_METH.format(methname=name),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        with open(os.path.join(LINGO_XLIBS_PATH, f"{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, type)
        inject_lingo_object(slug, xobj_class, director_version, "XObject")

def generate_xtra_stubs(
    msgtable: list[str],
    slug: str,
    name: str,
    filename: str,
    director_version: int = 500,
    dry_run: bool = False,
) -> None:
    meths = []
    for e in msgtable:
        elem = e.split("--", 1)[0].strip()
        if not elem:
            continue
        functype = "method"
        if elem.startswith("+"):
            elem = elem[1:].strip()
            functype = "toplevel"
        elif elem.startswith("*"):
            elem = elem[1:].strip()
            functype = "global"
        if " " not in elem:
            methname, argv = elem, []
        else:
            methname, args = elem.split(" ", 1)
            argv = args.split(",")
        min_args = len(argv)
        max_args = len(argv)
        if argv and argv[-1].strip() == "*":
            min_args = -1
            max_args = 0
        elif functype == "method":
            min_args -= 1
            max_args = 0

        meths.append(
            dict(
                functype=functype,
                methname=methname,
                args=argv,
                min_args=min_args,
                max_args=max_args,
                default="0",
            )
        )
    xobject_class = f"{name}XtraObject"
    xobj_class = f"{name}Xtra"

    cpp_text = TEMPLATE.format(
        base="xtras",
        slug=slug,
        name=name,
        filename=filename,
        xmethtable="\n".join(msgtable),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xlib_methods="\n".join(
            [
                XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
                )
                for x in meths if x["functype"] == "method"
            ]
        ),
        xlib_builtins="\n".join([BUILTIN_TEMPLATE.format(
            name=x["methname"],
            xobj_class=xobj_class,
            min_args=x["min_args"],
            max_args=x["max_args"],
            director_version=director_version,
            methtype="HBLTIN",
        ) for x in meths if x["functype"] == "global"]),
        xlib_toplevels="\n".join([BUILTIN_TEMPLATE.format(
            name=x["methname"],
            xobj_class=xobj_class,
            min_args=x["min_args"],
            max_args=x["max_args"],
            director_version=director_version,
            methtype="HBLTIN",
        ) for x in meths if x["functype"] == "toplevel"]),
        xtra_props=XTRA_PROPS_TEMPLATE.format(xobj_class=xobj_class,
                                              xobject_class=xobject_class),
		xobj_new=XLIB_NEW_TEMPLATE.format(xobj_class=xobj_class),
        xobj_stubs="\n".join(
            [
                XOBJ_STUB_TEMPLATE.format(xobj_class=xobj_class, **x)
                for x in meths
                if x["methname"] != "new"
            ]
        ),
    )
    if dry_run:
        print("C++ output:")
        print(cpp_text)
        print()
    else:
        with open(os.path.join(LINGO_XTRAS_PATH, f"{slug}.cpp"), "w") as cpp:
            cpp.write(cpp_text)

    header_text = TEMPLATE_H.format(
        base_upper="XTRAS",
        slug_upper=slug.upper(),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xtra_props_h=XTRA_PROPS_H,
        methlist="\n".join([TEMPLATE_HEADER_METH.format(**x) for x in meths]),
    )
    if dry_run:
        print("Header output:")
        print(header_text)
        print()
    else:
        with open(os.path.join(LINGO_XTRAS_PATH, f"{slug}.h"), "w") as header:
            header.write(header_text)

    if not dry_run:
        inject_makefile(slug, "Xtra")
        inject_lingo_object(slug, xobj_class, director_version, "Xtra")



def main() -> None:
    parser = argparse.ArgumentParser(
        description="Extract the method table from a Macromedia Director XObject/XLib and generate method stubs."
    )
    parser.add_argument("XOBJ_FILE", help="XObject/XLib file to test")
    parser.add_argument(
        "--resid", help="Resource ID (for MacBinary)", type=str, default=None
    )
    parser.add_argument(
        "--slug", help="Slug to use for files (e.g. {slug}.cpp, {slug}.h)"
    )
    parser.add_argument(
        "--name", help="Base name to use for classes (e.g. {name}XObj, {name}XObject)"
    )
    parser.add_argument(
        "--version",
        metavar="VER",
        help="Minimum Director version (default: 400)",
        type=int,
        default=400,
    )
    parser.add_argument(
        "--write",
        help="Write generated stubs to the source tree",
        dest="dry_run",
        action="store_false",
    )
    args = parser.parse_args()

    xcode = extract_xcode(args.XOBJ_FILE, args.resid)
    slug = args.slug or xcode["slug"]
    name = args.name or xcode["name"]
    if xcode["type"] == "XObject":
        generate_xobject_stubs(
            xcode["method_table"],
            slug,
            name,
            xcode["filename"],
            args.version,
            args.dry_run,
        )
    elif xcode["type"] == "Xtra":
        generate_xtra_stubs(
            xcode["method_table"], slug, name, xcode["filename"], args.version, args.dry_run
        )
    elif xcode["type"] == "XFCN" or xcode["type"] == "XCMD":
        generate_xcmd_stubs(
            xcode["type"], slug, name, xcode["filename"], args.version, args.dry_run
        )


if __name__ == "__main__":
    main()
