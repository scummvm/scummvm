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
#include "director/lingo/xlibs/dirutil.h"

/*
-- DIRUTIL External Factory. 16Feb93 PTM
DIRUTIL
I      mNew                    --Creates a new instance of the XObject
X      mDispose                --Disposes of XObject instance
S      mName                   --Returns the XObject name (dirutil)
I      mStatus                 --Returns an integer status code
SI     mError, code            --Returns an error string
S      mLastError              --Returns last error string
ISI    mSetAttrib,filename,attrib --Sets the Attibute of attr
IS     mGetAttrib,filename     --Gets the Attibute of filename
II     mGetDriveType,drivenumber --Gets the type of a drive selected
I      mQTVersion              --Returns an integer status code
S      mQTVersionStr           --Returns version as a string
II     mIsCdrom,drivenumber    --Return true if drive CDROM
S      mAuthors                --Information on authors
XI     mSetErrorMode,mode      --sets windoze error mode
 */

namespace Director {

const char *const DirUtilXObj::xlibName = "DirUtil";
const XlibFileDesc DirUtilXObj::fileNames[] = {
	{ "dirutil",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				DirUtilXObj::m_new,		 0, 0,	400 },	// D4
	{ "dispose",				DirUtilXObj::m_dispose,		 0, 0,	400 },	// D4
	{ "name",				DirUtilXObj::m_name,		 0, 0,	400 },	// D4
	{ "status",				DirUtilXObj::m_status,		 0, 0,	400 },	// D4
	{ "error",				DirUtilXObj::m_error,		 1, 1,	400 },	// D4
	{ "lastError",				DirUtilXObj::m_lastError,		 0, 0,	400 },	// D4
	{ "setAttrib",				DirUtilXObj::m_setAttrib,		 2, 2,	400 },	// D4
	{ "getAttrib",				DirUtilXObj::m_getAttrib,		 1, 1,	400 },	// D4
	{ "getDriveType",				DirUtilXObj::m_getDriveType,		 1, 1,	400 },	// D4
	{ "qTVersion",				DirUtilXObj::m_qTVersion,		 0, 0,	400 },	// D4
	{ "qTVersionStr",				DirUtilXObj::m_qTVersionStr,		 0, 0,	400 },	// D4
	{ "isCdrom",				DirUtilXObj::m_isCdrom,		 1, 1,	400 },	// D4
	{ "authors",				DirUtilXObj::m_authors,		 0, 0,	400 },	// D4
	{ "setErrorMode",				DirUtilXObj::m_setErrorMode,		 1, 1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

DirUtilXObject::DirUtilXObject(ObjectType ObjectType) :Object<DirUtilXObject>("DirUtil") {
	_objType = ObjectType;
}

void DirUtilXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		DirUtilXObject::initMethods(xlibMethods);
		DirUtilXObject *xobj = new DirUtilXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void DirUtilXObj::close(ObjectType type) {
	if (type == kXObj) {
		DirUtilXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void DirUtilXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DirUtilXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(DirUtilXObj::m_dispose)
XOBJSTUB(DirUtilXObj::m_name, "")
XOBJSTUB(DirUtilXObj::m_status, 0)
XOBJSTUB(DirUtilXObj::m_error, "")
XOBJSTUB(DirUtilXObj::m_lastError, "")
XOBJSTUB(DirUtilXObj::m_setAttrib, 0)
XOBJSTUB(DirUtilXObj::m_getAttrib, 0)
XOBJSTUB(DirUtilXObj::m_getDriveType, 0)
XOBJSTUB(DirUtilXObj::m_qTVersion, 0)
XOBJSTUB(DirUtilXObj::m_qTVersionStr, "2.03.51")
XOBJSTUB(DirUtilXObj::m_isCdrom, 0)
XOBJSTUB(DirUtilXObj::m_authors, "")
XOBJSTUBNR(DirUtilXObj::m_setErrorMode)

}
