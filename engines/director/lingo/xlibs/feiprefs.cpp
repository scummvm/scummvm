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
* Dracula's Secret
*
*************************************/

/*
 * -- FEIPrefs External Factory.
 * FEIPrefs
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (FEIPref)
 * ISS    mPrefAppName        -- GroupName,AppName
 * ISSS    mPrefWriteString    -- Section,Entry,String
 * ISSI    mPrefWriteInteger    -- Section,Entry,Integer
 * ISSI    mPrefReadInteger    -- Section,Entry,Default
 * SSSS    mPrefReadString    -- Section,Entry,Default
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/feiprefs.h"


namespace Director {

const char *const FEIPrefsXObj::xlibName = "FEIPrefs";
const XlibFileDesc FEIPrefsXObj::fileNames[] = {
	{ "feiprefs",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",   FEIPrefsXObj::m_new,							0,	0,	400 },	// D4
	{ "PrefAppName",   FEIPrefsXObj::m_prefAppName,			2,	2,	400 },	// D4
	{ "PrefReadString",   FEIPrefsXObj::m_prefReadString,	3,	3,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void FEIPrefsXObj::open(ObjectType type, const Common::Path &path) {
   if (type == kXObj) {
		FEIPrefsXObject::initMethods(xlibMethods);
		FEIPrefsXObject *xobj = new FEIPrefsXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
   }
}

void FEIPrefsXObj::close(ObjectType type) {
   if (type == kXObj) {
		FEIPrefsXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
   }
}


FEIPrefsXObject::FEIPrefsXObject(ObjectType ObjectType) : Object<FEIPrefsXObject>("FEIPrefs") {
	_objType = ObjectType;
}

void FEIPrefsXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("FEIPrefsXObj::new", nargs);
	g_lingo->push(g_lingo->_state->me);
}

void FEIPrefsXObj::m_prefAppName(int nargs) {
	g_lingo->printSTUBWithArglist("FEIPrefsXObj::m_prefAppName", nargs);
	g_lingo->dropStack(nargs);
}

void FEIPrefsXObj::m_prefReadString(int nargs) {
	g_lingo->printSTUBWithArglist("FEIPrefsXObj::m_prefReadString", nargs);
	g_lingo->pop(); // TODO
	auto key = g_lingo->pop().asString();
	auto defaultValue = g_lingo->pop();
	g_lingo->push(defaultValue); // TODO: Just return the default for now.
}

} // End of namespace Director
