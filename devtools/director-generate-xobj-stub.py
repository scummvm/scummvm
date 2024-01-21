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


DIRECTOR_SRC_PATH = os.path.abspath(
    os.path.join(__file__, "..", "..", "engines", "director")
)
MAKEFILE_PATH = os.path.join(DIRECTOR_SRC_PATH, "module.mk")
LINGO_XLIBS_PATH = os.path.join(DIRECTOR_SRC_PATH, "lingo", "xlibs")
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

const char *{xobj_class}::xlibName = "{name}";
const char *{xobj_class}::fileNames[] = {{
	"{filename}",
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
	g_lingo->printSTUBWithArglist("{xobj_class}::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}}

{xobj_stubs}

}}
"""
)
XLIB_METHOD_TEMPLATE = """	{{ "{methname}",				{xobj_class}::m_{methname},		 {arg_count}, {arg_count},	{director_version} }},"""

XCMD_TEMPLATE_H = (
    LEGAL
    + """
#ifndef DIRECTOR_LINGO_XLIBS_{slug_upper}_H
#define DIRECTOR_LINGO_XLIBS_{slug_upper}_H

namespace Director {{

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
const char *{xobj_class}::fileNames[] = {{
	"{filename}",
	nullptr
}};

static BuiltinProto builtins[] = {{
{xlib_builtins}
	{{ nullptr, nullptr, 0, 0, 0, VOIDSYM }}
}};

void {xobj_class}::open(int type) {{
	g_lingo->initBuiltIns(builtins);
}}

void {xobj_class}::close(int type) {{
	g_lingo->cleanupBuiltIns(builtins);
}}

{xobj_stubs}

}}
"""
)

XCMD_BUILTIN_TEMPLATE = """	{{ "{name}", {xobj_class}::m_{name}, -1, 0, {director_version}, {methtype} }},"""

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


def inject_makefile(slug: str) -> None:
    make_contents = open(MAKEFILE_PATH, "r").readlines()
    expr = re.compile("^\tlingo/xlibs/([a-zA-Z0-9\\-]+).o( \\\\|)")
    for i in range(len(make_contents)):
        m = expr.match(make_contents[i])
        if m:
            if slug == m.group(1):
                # file already in makefile
                print(f"lingo/xlibs/{slug}.o already in {MAKEFILE_PATH}, skipping")
                return
            elif slug < m.group(1):
                make_contents.insert(i, f"\tlingo/xlibs/{slug}.o \\\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return
            elif m.group(2) == "":
                # final entry in the list
                make_contents[i] += " \\"
                make_contents.insert(i + 1, f"\tlingo/xlibs/{slug}.o\n")
                with open(MAKEFILE_PATH, "w") as f:
                    f.writelines(make_contents)
                return


def inject_lingo_object(slug: str, xobj_class: str, director_version: int) -> None:
    # write include statement for the object header
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    expr = re.compile('^#include "director/lingo/xlibs/([a-zA-Z0-9\\-]+)\\.h"')
    in_xlibs = False
    for i in range(len(lo_contents)):
        m = expr.match(lo_contents[i])
        if m:
            in_xlibs = True
            if slug == m.group(1):
                print(
                    f"lingo/xlibs/{slug}.h import already in {LINGO_OBJECT_PATH}, skipping"
                )
                break
            elif slug < m.group(1):
                lo_contents.insert(i, f'#include "director/lingo/xlibs/{slug}.h"\n')
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(i, f'#include "director/lingo/xlibs/{slug}.h"\n')
            with open(LINGO_OBJECT_PATH, "w") as f:
                f.writelines(lo_contents)
                break

    # write entry in the XLibProto table
    lo_contents = open(LINGO_OBJECT_PATH, "r").readlines()
    expr = re.compile("^\t\\{ ([a-zA-Z0-9_]+)::fileNames")
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
                    f"	{{ {xobj_class}::fileNames,			{xobj_class}::open,			{xobj_class}::close,		kXObj,					{director_version} }},	// D{director_version // 100}\n",
                )
                with open(LINGO_OBJECT_PATH, "w") as f:
                    f.writelines(lo_contents)
                break
        elif in_xlibs:
            # final entry in the list
            lo_contents.insert(
                i,
                f"	{{ {xobj_class}::fileNames,			{xobj_class}::open,			{xobj_class}::close,		kXObj,					{director_version} }},	// D{director_version // 100}\n",
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
    # get the .data section
    # find a string b"msgTable\x00", get the offset
    # get the .text section
    # find assembly:
    # 68 [ addr ] 6a 00 6a [ addr 2 ]
    # lookup addr2 in .data
    # get c string, split by \x0a

    return {"type": "Xtra", "name": "", "slug": "", "filename": "", "method_table": []}


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
                raise ValueError("No support yet for extracting from Win32 DLLs")
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
                arg_count=len(args),
                returnval=returnval,
                default='""' if returnval == "S" else "0",
            )
        )
    xobject_class = f"{name}XObject"
    xobj_class = f"{name}XObj"

    cpp_text = TEMPLATE.format(
        slug=slug,
        name=name,
        filename=filename,
        xmethtable="\n".join(xmethtable),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
        xlib_methods="\n".join(
            [
                XLIB_METHOD_TEMPLATE.format(
                    xobj_class=xobj_class, director_version=director_version, **x
                )
                for x in meths
            ]
        ),
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
        slug_upper=slug.upper(),
        xobject_class=xobject_class,
        xobj_class=xobj_class,
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
        inject_makefile(slug)
        inject_lingo_object(slug, xobj_class, director_version)


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
        xlib_builtins=XCMD_BUILTIN_TEMPLATE.format(
            name=name,
            xobj_class=xobj_class,
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
        inject_makefile(slug)
        inject_lingo_object(slug, xobj_class, director_version)


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
    elif xcode["type"] == "XFCN" or xcode["type"] == "XCMD":
        generate_xcmd_stubs(
            xcode["type"], slug, name, xcode["filename"], args.version, args.dry_run
        )


if __name__ == "__main__":
    main()
