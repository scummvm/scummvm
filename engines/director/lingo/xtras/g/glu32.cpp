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
#include "director/lingo/xtras/g/glu32.h"

/**************************************************
 *
 * USED IN:
 * Löwenzahn 4 (D7)(hashed), Löwenzahn 5 (D8) (hashed), Löwenzahn 7 (D8)(hashed), Löwenzahn 8 (D8.5)(hashed)
 * Löwenzahn Adventskalender (D8.5) (hashed), Löwenzahn Spielebox (D9)(optgraph.dll is a dummy)
 * TKKG 6 (disabled in start.dir), 7(hashed), 8(hashed), 9 (hashed)
 * Tiger Team 2 (detectionTable TBD)
 * Kommissar Kugelblitz - Vermisst am Missisippi (hashed)
 *
 **************************************************/

/*
-- xtra GLU32 -- Calls Win32 DLL Functions(v1.1)Ravi S. Singh Copyright(c) 1999
new object me
GLUNew object me,string DLLName,string FunctionName,string ReturnType,string Params,integer Vendor,integer CallingConvention -- Load DLL and Function
GLUCall object me, * -- Call the Function
GLUGetResults object me, * -- Get Call By Reference Results
GLUDispose object me, -- Unload DLL
GLUGetErrorString object me, integer code -- Retrieves a String for Given Error Code
GLUGetLastError object me, -- Gets Last Error Code
*GLURegister string SerialNumber, -- Unlock Function
GLUGetProp object me,string Property -- Gets a system property
GLUSetProp object me,string Property, * -- Sets a property
 */

namespace Director {

const char *GLU32Xtra::xlibName = "GLU32";
const XlibFileDesc GLU32Xtra::fileNames[] = {
	{ "glu32",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				GLU32Xtra::m_new,		 0, 0,	500 },
	{ "GLUNew",				GLU32Xtra::m_GLUNew,		 6, 6,	500 },
	{ "GLUCall",				GLU32Xtra::m_GLUCall,		 -1, 0,	500 },
	{ "GLUGetResults",				GLU32Xtra::m_GLUGetResults,		 -1, 0,	500 },
	{ "GLUDispose",				GLU32Xtra::m_GLUDispose,		 1, 1,	500 },
	{ "GLUGetErrorString",				GLU32Xtra::m_GLUGetErrorString,		 1, 1,	500 },
	{ "GLUGetLastError",				GLU32Xtra::m_GLUGetLastError,		 1, 1,	500 },
	{ "GLUGetProp",				GLU32Xtra::m_GLUGetProp,		 1, 1,	500 },
	{ "GLUSetProp",				GLU32Xtra::m_GLUSetProp,		 -1, 0,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "GLURegister", GLU32Xtra::m_GLURegister, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GLU32XtraObject::GLU32XtraObject(ObjectType ObjectType) :Object<GLU32XtraObject>("GLU32") {
	_objType = ObjectType;
}

bool GLU32XtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum GLU32XtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(GLU32Xtra::xlibName);
	warning("GLU32Xtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void GLU32Xtra::open(ObjectType type, const Common::Path &path) {
    GLU32XtraObject::initMethods(xlibMethods);
    GLU32XtraObject *xobj = new GLU32XtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GLU32Xtra::close(ObjectType type) {
    GLU32XtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GLU32Xtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GLU32Xtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void GLU32Xtra::m_GLUNew(int nargs) {
	g_lingo->printArgs("GLU32Xtra::m_GLUNew", nargs);
	GLU32XtraObject *me = (GLU32XtraObject *)g_lingo->_state->me.u.obj;

	Common::String dll, func;
	for (int i = nargs - 1; i >= 0; i--) {
		Datum d = g_lingo->pop();
		if (i == 0)
			dll = d.asString();
		else if (i == 1)
			func = d.asString();
	}
	me->_dll = dll;
	me->_func = func;

	g_lingo->push(Datum(0)); // 0 == no error
}

void GLU32Xtra::m_GLUCall(int nargs) {
	// We cannot run real DLLs, so all genuinely-uncallable bindings return 0;
	// except the Tivola/Terzio copy-protection probe optgraph.dll!initdisplay. The real
	// DLL returns a per-title magic which is compaired against a hard-coded constant.
	// We return the expected value per game.
	g_lingo->printArgs("GLU32Xtra::m_GLUCall", nargs);
	GLU32XtraObject *me = (GLU32XtraObject *)g_lingo->_state->me.u.obj;
	g_lingo->dropStack(nargs);

	Common::String dll = me->_dll;
	for (int i = (int)dll.size() - 1; i >= 0; i--) {
		if (dll[i] == '\\' || dll[i] == '/') {
			dll = dll.substr(i + 1);
			break;
		}
	}

	if (dll.equalsIgnoreCase("optgraph.dll") && me->_func.equalsIgnoreCase("initdisplay")) {
		Common::String gameId = g_director->getGameId();
		if (gameId == "tkkg7") {
			g_lingo->push(Datum((int)43123212)); //tested
			return;
		}
		if (gameId == "tkkg8") {
			g_lingo->push(Datum((int)16325536)); // TODO: Check with the game
			return;
		}
		if (gameId == "tkkg9") {
			g_lingo->push(Datum((int)16988788)); // TODO: Check with the game
			return;
		}
		if (gameId == "komissar1") {
			g_lingo->push(Datum(Common::String("37211457"))); // TODO: Check with the game
			return;
		}
		if (gameId == "loewe4") {
			g_lingo->push(Datum(Common::String("13665984"))); // TODO: Check with the game
			return;
		}
		if (gameId == "loewe5") {
			g_lingo->push(Datum(Common::String("22457882"))); // TODO: Check with the game
			return;
		}
		if (gameId == "loewe7") {
			g_lingo->push(Datum(Common::String("15225045"))); // TODO: Check with the game
			return;
		}
		if (gameId == "loewe8") {
			g_lingo->push(Datum(Common::String("32370583"))); // TODO: Check with the game
			return;
		}
		if (gameId == "loewecalendar") {
			g_lingo->push(Datum(Common::String("92263924"))); // TODO: Check with the game
			return;
		}
		if (gameId == "physicus" && g_director->getLanguage() == Common::EN_ANY) {
			g_lingo->push(Datum((int)43289555)); //tested
			return;
		}
		// TODO: Tiger Team 2: 32546872
		warning("GLU32Xtra::m_GLUCall: unhandled optgraph.dll copy protection for game '%s'", gameId.c_str());
	}

	g_lingo->push(Datum(0));
}

XOBJSTUB(GLU32Xtra::m_GLUGetResults, 0)
XOBJSTUB(GLU32Xtra::m_GLUDispose, 0)
XOBJSTUB(GLU32Xtra::m_GLUGetErrorString, 0)
XOBJSTUB(GLU32Xtra::m_GLUGetLastError, 0)
XOBJSTUB(GLU32Xtra::m_GLURegister, 0)
XOBJSTUB(GLU32Xtra::m_GLUGetProp, 0)
XOBJSTUB(GLU32Xtra::m_GLUSetProp, 0)

}
