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
#include "director/lingo/xtras/a/asudp.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra asUDP -- An Xtra for Macromedia Director that does UDP datagram send and receive
-- Version 4.1
-- http://www.schmittmachine.com
new object me, *
free object me -- returns 0 if error
getLocalIP object me -- returns the first local IP, ex: '192.168.0.1', returns 0 if error. Forbidden in Shockwave.
getLocalIPs object me -- returns the local IPs list, ex: ['192.168.0.1', '192.168.0.2'], returns 0 if error. Forbidden in Shockwave.
startListen object me, integer localPort, * -- last parameters is string localIP and is optional, by default uses the first local IP. returns 0 if error
stopListen object me -- returns 0 if error
bytesReceived object me -- returns the number of bytes received
getBytes object me -- receive bytes, returns 0 if error
getBytesWithSenderInfo object me -- receive bytes and sender information, returns 0 if error
sendBytes object me, bytes buffer, string destIP, integer destPort  -- send buffer, returns 0 if error
setSendingPort object me, integer sendingPort -- sets the sending port, VOID means random, which is default
getError object me
"
 */

namespace Director {

const char *AsudpXtra::xlibName = "Asudp";
const XlibFileDesc AsudpXtra::fileNames[] = {
	{ "asudp",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AsudpXtra::m_new,		 -1, 0,	1000 },
	{ "free",				AsudpXtra::m_free,		 0, 0,	1000 },
	{ "getLocalIP",				AsudpXtra::m_getLocalIP,		 0, 0,	1000 },
	{ "getLocalIPs",				AsudpXtra::m_getLocalIPs,		 0, 0,	1000 },
	{ "startListen",				AsudpXtra::m_startListen,		 -1, 0,	1000 },
	{ "stopListen",				AsudpXtra::m_stopListen,		 0, 0,	1000 },
	{ "bytesReceived",				AsudpXtra::m_bytesReceived,		 0, 0,	1000 },
	{ "getBytes",				AsudpXtra::m_getBytes,		 0, 0,	1000 },
	{ "getBytesWithSenderInfo",				AsudpXtra::m_getBytesWithSenderInfo,		 0, 0,	1000 },
	{ "sendBytes",				AsudpXtra::m_sendBytes,		 3, 3,	1000 },
	{ "setSendingPort",				AsudpXtra::m_setSendingPort,		 1, 1,	1000 },
	{ "getError",				AsudpXtra::m_getError,		 0, 0,	1000 },
	{ """,				AsudpXtra::m_",		 -1, -1,	1000 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AsudpXtraObject::AsudpXtraObject(ObjectType ObjectType) :Object<AsudpXtraObject>("Asudp") {
	_objType = ObjectType;
}

bool AsudpXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AsudpXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AsudpXtra::xlibName);
	warning("AsudpXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AsudpXtra::open(ObjectType type, const Common::Path &path) {
    AsudpXtraObject::initMethods(xlibMethods);
    AsudpXtraObject *xobj = new AsudpXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AsudpXtra::close(ObjectType type) {
    AsudpXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AsudpXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AsudpXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AsudpXtra::m_free, 0)
XOBJSTUB(AsudpXtra::m_getLocalIP, 0)
XOBJSTUB(AsudpXtra::m_getLocalIPs, 0)
XOBJSTUB(AsudpXtra::m_startListen, 0)
XOBJSTUB(AsudpXtra::m_stopListen, 0)
XOBJSTUB(AsudpXtra::m_bytesReceived, 0)
XOBJSTUB(AsudpXtra::m_getBytes, 0)
XOBJSTUB(AsudpXtra::m_getBytesWithSenderInfo, 0)
XOBJSTUB(AsudpXtra::m_sendBytes, 0)
XOBJSTUB(AsudpXtra::m_setSendingPort, 0)
XOBJSTUB(AsudpXtra::m_getError, 0)
XOBJSTUB(AsudpXtra::m_", 0)

}
