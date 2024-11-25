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
 * Secrets of the Pyramids (Win)
 * Mindscape's Wonder Land (Win)
 * Grackon's Curse (Win)
 * Return to Jurassic (Win)
 *
 *************************************/

/*
 * -- Ednox External Factory. 22Nov93 PTM
 * ednox
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (Widget)
 * I      mStatus             --Returns an integer status code
 * SI     mError, code        --Returns an error string
 * S      mLastError          --Returns last error string
 * SSI    playSoundX       --Play an external sound, return status string
 * S      clearSoundX      --Stop an external sound, return status string
 * S      checkSoundX      --check an external sound,see if it is still playing
 * SS      drawBkgndX,hBkgnd   --draw background image, from resource bitmap
 * SS     SaveX, hStrIn    --Save string into a file(file name will be prompted
 * SSSI    GetPathX,hSection,hStrIn,mMacMode, Retrieve the installed path name from the win.ini and return
 * S      RestoreX     --Restore string from a file(file name will be prompted
 * SS      SetDriveX,hStrIn    --Set the current path name
 * SSS    IsCDX,hDrive,hStrIn --Check for the correct CD
 * S      EnableTaskSwitch    --enable task switch
 * S      DisableTaskSwitch   --Disable task switch
 * SSS    getDocumentName,hdir,hext --Get strings of file in specific directory
 * SSS    getDocumentFile,hDir,hFile --Get string from a file
 * SSSS   saveDocumentFile,hDir,hFile,hStrIn --save string into a file
 * SSS    deleteDocumentFile,hDir,hFile  -- delete a file from the directory
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/ednox.h"


namespace Director {

const char *const Ednox::xlibName = "Ednox";
const XlibFileDesc Ednox::fileNames[] = {
	{ "Ednox",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "mNew",				Ednox::m_new,					0,	0,	300 },	// d3
	{ "mDispose",			Ednox::m_dispose,				0,	0,	300 },	// d3
	{ "mName",				Ednox::m_name,					0,	0,	300 },	// d3
	{ "mStatus",			Ednox::m_status,				0,	0,	300 },	// d3
	{ "mError",				Ednox::m_error,					1,	1,	300 },	// d3
	{ "mLastError",			Ednox::m_lasterror,				0,	0,	300 },	// d3
	{ "playSoundX",			Ednox::m_playsoundx,			2,	2,	300 },	// d3
	{ "clearSoundX",		Ednox::m_clearsoundx,			0,	0,	300 },	// d3
	{ "checkSoundX",		Ednox::m_checksoundx,			0,	0,	300 },	// d3
	{ "drawBkgndX",			Ednox::m_drawbkgndx,			1,	1,	300 },	// d3
	{ "SaveX",				Ednox::m_savex,					1,	1,	300 },	// d3
	{ "GetPathX",			Ednox::m_getpathx,				3,	3,	300 },	// d3
	{ "RestoreX",			Ednox::m_restorex,				0,	0,	300 },	// d3
	{ "SetDriveX",			Ednox::m_setdrivex,				1,	1,	300 },	// d3
	{ "IsCDX",				Ednox::m_iscdx,					2,	2,	300 },	// d3
	{ "EnableTaskSwitch",	Ednox::m_enabletaskswitch,		0,	0,	300 },	// d3
	{ "DisableTaskSwitch",	Ednox::m_disabletaskswitch,		0,	0,	300 },	// d3
	{ "getDocumentName",	Ednox::m_getdocumentname,		2,	2,	300 },	// d3
	{ "getDocumentFile",	Ednox::m_getdocumentfile,		2,	2,	300 },	// d3
	{ "saveDocumentFile",	Ednox::m_savedocumentfile,		3,	3,	300 },	// d3
	{ "deleteDocumentFile",	Ednox::m_deletedocumentfile,	2,	2,	300 },	// d3
	{ nullptr, nullptr, 0, 0, 0 }
};

void Ednox::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		EdnoxObject::initMethods(xlibMethods);
		EdnoxObject *xobj = new EdnoxObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void Ednox::close(ObjectType type) {
	if (type == kXObj) {
		EdnoxObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


EdnoxObject::EdnoxObject(ObjectType ObjectType) :Object<EdnoxObject>("Ednox") {
	_objType = ObjectType;
}

void Ednox::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(Ednox::m_dispose)

void Ednox::m_getdocumentfile(int nargs) {
	// Common::U32String hFile = g_lingo->pop().asString();
	// Common::U32String hDir = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_getdocumentfile", nargs);
	g_lingo->dropStack(nargs);
}

void Ednox::m_getpathx(int nargs) {
	/* int mMacMode = */ g_lingo->pop().asInt();
	Common::U32String hStrIn = g_lingo->pop().asString();
	/* Common::U32String hSection = */ g_lingo->pop().asString();
	hStrIn.toLowercase();
	if (hStrIn == "cdpath"){
		g_lingo->push(Datum("d:\\"));
	} else {
		g_lingo->push(Datum(""));
	}
}

void Ednox::m_iscdx(int nargs) {
	/* Common::U32String hStrIn = */ g_lingo->pop().asString();
	Common::U32String hDrive = g_lingo->pop().asString();
	// g_lingo->printSTUBWithArglist("Ednox::m_iscdx", nargs);
	if (hDrive == "d:\\"){
		g_lingo->push(Datum(0));
	} else {
		g_lingo->push(Datum(-1));
	}
}

void Ednox::m_savedocumentfile(int nargs) {
	// Common::U32String hStrIn = g_lingo->pop().asString();
	// Common::U32String hFile = g_lingo->pop().asString();
	// Common::U32String hDir = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_savedocumentfile", nargs);
	g_lingo->dropStack(nargs);
}

void Ednox::m_setdrivex(int nargs) {
	// Common::U32String hStrIn = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_setdrivex", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

XOBJSTUB(Ednox::m_checksoundx, "")
XOBJSTUB(Ednox::m_clearsoundx, "")

void Ednox::m_deletedocumentfile(int nargs) {
	// Common::U32String hFile = g_lingo->pop().asString();
	// Common::U32String hDir = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_deletedocumentfile", nargs);
	g_lingo->dropStack(nargs);
}

XOBJSTUB(Ednox::m_enabletaskswitch, "")
XOBJSTUB(Ednox::m_disabletaskswitch, "")

void Ednox::m_drawbkgndx(int nargs) {
	// Common::U32String hBkgnd = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_drawbkgndx", nargs);
}

void Ednox::m_getdocumentname(int nargs) {
	// Common::U32String hExt = g_lingo->pop().asString();
	// Common::U32String hDir = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_getdocumentname", nargs);
	g_lingo->dropStack(nargs);
}

void Ednox::m_error(int nargs) {
	// int code = g_lingo->pop().asInt();
	g_lingo->printSTUBWithArglist("Ednox::m_error", nargs);
	g_lingo->dropStack(nargs);
}

XOBJSTUB(Ednox::m_lasterror, "")

void Ednox::m_name(int nargs) {
	g_lingo->push(Datum("ednox"));
}

XOBJSTUB(Ednox::m_status, 0)
XOBJSTUB(Ednox::m_playsoundx, "")
XOBJSTUB(Ednox::m_restorex, "")

void Ednox::m_savex(int nargs) {
	// Common::U32String hStrIn = g_lingo->pop().asString();
	g_lingo->printSTUBWithArglist("Ednox::m_savex", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
