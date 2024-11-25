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

/*************************************
 *
 * USED IN:
 * Gahan Wilson's Ultimate Haunted House
 * Wellen, Wracks und Wassermaenner
 *
 *************************************/

/*
 * +Access INI Files (c) JourneyWare Media 1994
 * -- Access INI files
 * X      mDispose                                                  --Dispose of the XObject
 * ISSIS  mGetPrivateProfileInt, App, Key, Def, File                --GetPrivateProfileInt
 * SSSSIS mGetPrivateProfileString, App, Key, Default, Size, File   --GetPrivateProfileString
 * ISSI   mGetProfileInt, App, Key, Default                         --GetProfileInt
 * SSSSI  mGetProfileString, App, Key, Def, Size                    --GetProfileString
 * S      mName                                                     --Return XObject name
 * I      mNew                                                      --Create a new instance
 * ISSSS  mWritePrivateProfileString, App, Key, String, File        --WritePrivateProfileString
 * ISSS   mWriteProfileString, App, Key, String                     --WriteProfileString
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/jwxini.h"

namespace Director {

const char *const JourneyWareXINIXObj::xlibName = "INI";
const XlibFileDesc JourneyWareXINIXObj::fileNames[] = {
	{ "JWXINI",	nullptr },
	{ "INI",	"www" },	// "Wellen, Wracks und Wassermaenner" names it INI.DLL
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{"new", 						JourneyWareXINIXObj::m_new, 0, 0, 400}, // D4
	{"GetPrivateProfileInt", 		JourneyWareXINIXObj::m_GetPrivateProfileInt, 4, 4, 400}, // D4
	{"GetPrivateProfileString", 	JourneyWareXINIXObj::m_GetPrivateProfileString, 5, 5, 400}, // D4
	{"GetProfileInt", 				JourneyWareXINIXObj::m_GetProfileInt, 3, 3, 400}, // D4
	{"GetProfileString", 			JourneyWareXINIXObj::m_GetProfileString, 4, 4, 400}, // D4
	{"WritePrivateProfileString", 	JourneyWareXINIXObj::m_WritePrivateProfileString, 4, 4, 400}, // D4
	{"WriteProfileString", 			JourneyWareXINIXObj::m_WriteProfileString, 3, 3, 400}, // D4
	{nullptr, nullptr, 0, 0, 0}
};

void JourneyWareXINIXObj::open(ObjectType type, const Common::Path &path) {
   if (type == kXObj) {
	   JourneyWareXINIXObject::initMethods(xlibMethods);
	   JourneyWareXINIXObject *xobj = new JourneyWareXINIXObject(kXObj);
	   g_lingo->exposeXObject(xlibName, xobj);
   }
}

void JourneyWareXINIXObj::close(ObjectType type) {
   if (type == kXObj) {
	   JourneyWareXINIXObject::cleanupMethods();
	   g_lingo->_globalvars[xlibName] = Datum();
   }
}

JourneyWareXINIXObject::JourneyWareXINIXObject(ObjectType ObjectType) :Object<JourneyWareXINIXObject>("INI") {
   _objType = ObjectType;
}

void JourneyWareXINIXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("JWXIni::new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void JourneyWareXINIXObj::m_GetPrivateProfileInt(int nargs) {
	g_lingo->printSTUBWithArglist("JWXIni::GetPrivateProfileInt", nargs);
	/*auto file= */g_lingo->pop().asString();
	auto defaultValue = g_lingo->pop().asInt();
	/*auto key = */g_lingo->pop().asString();
	/*auto application = */g_lingo->pop().asString();
	g_lingo->push(Datum(defaultValue)); // TODO: We only return the default for now
}

void JourneyWareXINIXObj::m_GetPrivateProfileString(int nargs) {
	g_lingo->printSTUBWithArglist("JWXIni::GetPrivateProfileString", nargs);
	/*auto file = */g_lingo->pop().asString();
	/*auto size = */g_lingo->pop().asInt();
	auto defaultValue = g_lingo->pop().asString();
	/*auto key = */g_lingo->pop().asString();
	/*auto application = */g_lingo->pop().asString();
	g_lingo->push(Datum(defaultValue)); // TODO: We only return the default for now
}

void JourneyWareXINIXObj::m_GetProfileInt(int nargs) {
	g_lingo->printSTUBWithArglist("JWXIni::GetProfileInt", nargs);
	auto defaultValue = g_lingo->pop().asInt();
	/*auto key = */g_lingo->pop().asString();
	/*auto application = */g_lingo->pop().asString();
	g_lingo->push(Datum(defaultValue)); // TODO: We only return the default for now
}

void JourneyWareXINIXObj::m_GetProfileString(int nargs) {
	g_lingo->printSTUBWithArglist("JWXIni::GetProfileString", nargs);
	/*auto size = */g_lingo->pop().asInt();
	auto defaultValue = g_lingo->pop().asString();
	/*auto key = */g_lingo->pop().asString();
	/*auto application = */g_lingo->pop().asString();
	g_lingo->push(Datum(defaultValue)); // TODO: We only return the default for now
}

XOBJSTUB(JourneyWareXINIXObj::m_WritePrivateProfileString, 0)
XOBJSTUB(JourneyWareXINIXObj::m_WriteProfileString, 0)

} // End of namespace Director
