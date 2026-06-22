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
#include "director/lingo/xtras/a/aescrypt.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra AESCrypt
-- v0.1 (c) 2019 Valentin Schmidt
-- https://valentin.dasdeck.com

-- AESCrypt xtra implements strong AES-256 encryption. It's based on the source code of 
-- "AES Crypt" (https://www.aescrypt.com) and therefor compatible with all its other 
-- implementations/ports (win/mac/linux/android/python/java/c#).


-- Interface:

*encryptFile string infile, string outfile, string password -- Returns either TRUE for success or error message as string
*decryptFile string infile, string outfile, string password -- Returns either TRUE for success or error message as string

*encryptByteArray object inByteArray, string password -- Returns either encrypted byteArray or error message as string
*decryptByteArray object inByteArray, string password -- Returns either decrypted byteArray or error message as string
 */

namespace Director {

const char *AescryptXtra::xlibName = "Aescrypt";
const XlibFileDesc AescryptXtra::fileNames[] = {
	{ "aescrypt",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {


	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "encryptFile", AescryptXtra::m_encryptFile, 3, 3, 800, HBLTIN },
	{ "decryptFile", AescryptXtra::m_decryptFile, 3, 3, 800, HBLTIN },
	{ "encryptByteArray", AescryptXtra::m_encryptByteArray, 2, 2, 800, HBLTIN },
	{ "decryptByteArray", AescryptXtra::m_decryptByteArray, 2, 2, 800, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AescryptXtraObject::AescryptXtraObject(ObjectType ObjectType) :Object<AescryptXtraObject>("Aescrypt") {
	_objType = ObjectType;
}

bool AescryptXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AescryptXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AescryptXtra::xlibName);
	warning("AescryptXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AescryptXtra::open(ObjectType type, const Common::Path &path) {
    AescryptXtraObject::initMethods(xlibMethods);
    AescryptXtraObject *xobj = new AescryptXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AescryptXtra::close(ObjectType type) {
    AescryptXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AescryptXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AescryptXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AescryptXtra::m_encryptFile, 0)
XOBJSTUB(AescryptXtra::m_decryptFile, 0)
XOBJSTUB(AescryptXtra::m_encryptByteArray, 0)
XOBJSTUB(AescryptXtra::m_decryptByteArray, 0)

}
