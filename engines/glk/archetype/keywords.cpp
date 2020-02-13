/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/archetype/keywords.h"
#include "glk/archetype/archetype.h"
#include "glk/archetype/misc.h"

namespace Glk {
namespace Archetype {

LookupType Reserved_Wds = {
	nullptr,
	"ABSENT",
	"FALSE",
	"TRUE",
	"UNDEFINED",
	"based",
	"break",
	"case",
	"create",
	"default",
	"destroy",
	"do",
	"each",
	"else",
	"end",
	"for",
	"if",
	"include",
	"key",
	"keyword",
	"message",
	"methods",
	"named",
	"null",
	"of",
	"on",
	"read",
	"self",
	"sender",
	"stop",
	"then",
	"type",
	"while",
	"write",
	"writes"
};

LookupType Operators = {
	 nullptr,
	"&",
	"&:=",
	"*",
	"*:=",
	"+",
	"+:=",
	"-",
	"-->",
	"-:=",
	"->",
	".",
	"/",
	"/:=",
	":=",
	"<",
	"<=",
	"=",
	">",
	">=",
	"?",
	"^",
	"and",
	"chs",
	"leftfrom",
	"length",
	"not",
	"numeric",
	"or",
	"rightfrom",
	"string",
	"within",
	"~=",
	nullptr,
	nullptr
};

void load_text_list(Common::ReadStream *fIn, XArrayType &the_list) {
	int i, n;
	String s;

	new_xarray(the_list);
	n = fIn->readUint16LE();

	for (i = 0; i < n; ++i) {
		load_string(fIn, s);
		append_to_xarray(the_list, NewConstStr(s));
	}
}

void dump_text_list(Common::WriteStream *fOut, XArrayType &the_list) {
	void *p;

	fOut->writeUint16LE(the_list.size());
	for (uint i = 1; i <= the_list.size(); ++i) {
		if (index_xarray(the_list, i, p))
			dump_string(fOut, *(StringPtr)(p));
	}
}

void dispose_text_list(XArrayType &the_list) {
	void *p;

	for (uint i = 1; i <= the_list.size(); ++i) {
		if (index_xarray(the_list, i, p))
			delete (StringPtr)p;
	}

	dispose_xarray(the_list);
}

void load_id_info(Common::ReadStream *bfile) {
	load_text_list(bfile, g_vm->Type_ID_List);
	load_text_list(bfile, g_vm->Object_ID_List);
	load_text_list(bfile, g_vm->Attribute_ID_List);
}

void dump_id_info(Common::WriteStream *bfile) {
	dump_text_list(bfile, g_vm->Type_ID_List);
	dump_text_list(bfile, g_vm->Object_ID_List);
	dump_text_list(bfile, g_vm->Attribute_ID_List);
}

} // End of namespace Archetype
} // End of namespace Glk
