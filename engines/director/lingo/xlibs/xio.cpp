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
 * מיץ פטל (Mitz Petel)
 *
 *************************************/

/*
 * -- Xio - Functions for file manipulation 10Aug96 Moty Aharobnovitz
 * xio
 * I	mNew			--Creates a new instance of the XObject
 * X	mDispose		--Disposes of XObject instance
 * IS  mUnlock    --Given a correct code, enables usage of the Xobject
 * IS  mDeleteFile  --Removes a file from the file system
 * ISS  mCopyFile   --Copies a file from source to dest
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/xio.h"


namespace Director {

const char *const XioXObj::xlibName = "Xio";
const XlibFileDesc XioXObj::fileNames[] = {
	{ "XIO",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",		XioXObj::m_new,			0,	0,	400 },	// D4
	{ "Dispose",	XioXObj::m_dispose,		0,	0,	400 },	// D4
	{ "Unlock",		XioXObj::m_unlock,		1,	1,	400 },	// D4
	{ "DeleteFile",	XioXObj::m_deleteFile,	1,	1,	400 },	// D4
	{ "CopyFile",	XioXObj::m_copyFile,	2,	2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void XioXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		XioXObject::initMethods(xlibMethods);
		XioXObject *xobj = new XioXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void XioXObj::close(ObjectType type) {
	if (type == kXObj) {
		XioXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


XioXObject::XioXObject(ObjectType ObjectType) :Object<XioXObject>("Xio") {
	_objType = ObjectType;
}

void XioXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void XioXObj::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("XioXObj::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void XioXObj::m_unlock(int nargs) {
	Common::String unlockPassword = g_lingo->pop().asString();
	warning("XioXObj::m_unlock: Unlocked with password \"%s\"", unlockPassword.c_str());
	g_lingo->push(Datum(1));
}

XOBJSTUB(XioXObj::m_deleteFile, 1)
XOBJSTUB(XioXObj::m_copyFile, 1)

} // End of namespace Director
