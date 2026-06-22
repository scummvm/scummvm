/* ScummVM - Graphic Adventure Engine
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
#include "director/lingo/xtras/b/binbin.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BinBin -- Are you writing script, and have you never thought "If XOR of value is removed here."? So, this. "Bin Bin Xtra". values are compared, and AND, OR, XOR can be taken, and even which of 10 suspected binary numbers, 2 suspected binary numbers can receive the result further. Futhermore, it lets me do the bit shift of value. Do you try to be absorbed in bit operation from today, too?
-- http://xtradesign.net/
new object me
-------------------------------------
*Bin2Dec string -- Conbert Bin To Dec
*Dec2Bin integer -- Conbert Dec To Bin
-------------------------------------
*BinBin string, string, integer, integer -- and, or, xor  
*DecBin integer, string, integer, integer -- and, or, xor  
*DecDec integer, integer, integer, integer -- and, or, xor  
*BinDec string, integer, integer, integer -- and, or, xor  
-------------------------------------
*BinShift string, integer, integer -- Bit Shift  
*DecShift integer, integer, integer -- Bit Shift  
*GetCarry -- get carry flag  
*ClearCarry -- clear carry flag  
"

 */

namespace Director {

const char *BinbinXtra::xlibName = "Binbin";
const XlibFileDesc BinbinXtra::fileNames[] = {
	{ "binbin",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BinbinXtra::m_new,		 0, 0,	600 },
	{ """,				BinbinXtra::m_",		 -1, -1,	600 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "Bin2Dec", BinbinXtra::m_Bin2Dec, 1, 1, 600, HBLTIN },
	{ "Dec2Bin", BinbinXtra::m_Dec2Bin, 1, 1, 600, HBLTIN },
	{ "BinBin", BinbinXtra::m_BinBin, 4, 4, 600, HBLTIN },
	{ "DecBin", BinbinXtra::m_DecBin, 4, 4, 600, HBLTIN },
	{ "DecDec", BinbinXtra::m_DecDec, 4, 4, 600, HBLTIN },
	{ "BinDec", BinbinXtra::m_BinDec, 4, 4, 600, HBLTIN },
	{ "BinShift", BinbinXtra::m_BinShift, 3, 3, 600, HBLTIN },
	{ "DecShift", BinbinXtra::m_DecShift, 3, 3, 600, HBLTIN },
	{ "GetCarry", BinbinXtra::m_GetCarry, 0, 0, 600, HBLTIN },
	{ "ClearCarry", BinbinXtra::m_ClearCarry, 0, 0, 600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BinbinXtraObject::BinbinXtraObject(ObjectType ObjectType) :Object<BinbinXtraObject>("Binbin") {
	_objType = ObjectType;
}

bool BinbinXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BinbinXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BinbinXtra::xlibName);
	warning("BinbinXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BinbinXtra::open(ObjectType type, const Common::Path &path) {
    BinbinXtraObject::initMethods(xlibMethods);
    BinbinXtraObject *xobj = new BinbinXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BinbinXtra::close(ObjectType type) {
    BinbinXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BinbinXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BinbinXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BinbinXtra::m_Bin2Dec, 0)
XOBJSTUB(BinbinXtra::m_Dec2Bin, 0)
XOBJSTUB(BinbinXtra::m_BinBin, 0)
XOBJSTUB(BinbinXtra::m_DecBin, 0)
XOBJSTUB(BinbinXtra::m_DecDec, 0)
XOBJSTUB(BinbinXtra::m_BinDec, 0)
XOBJSTUB(BinbinXtra::m_BinShift, 0)
XOBJSTUB(BinbinXtra::m_DecShift, 0)
XOBJSTUB(BinbinXtra::m_GetCarry, 0)
XOBJSTUB(BinbinXtra::m_ClearCarry, 0)
XOBJSTUB(BinbinXtra::m_", 0)

}
