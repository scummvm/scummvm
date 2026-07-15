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
#include "director/lingo/xtras/n/netlingo.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 1 / 2 / 3 / 4 / 5 / 6 / 7 / 8 / Adventskalender / Spielebox
 * TKKG 4 / 7 / 8 / 9 / 10 / 11 / 13 / 14
 * Oscar the Balloonist Discovers the Sea
 * Bioscopia / Biolab
 *
 **************************************************/

/*
-- xtra NetLingo 
* netStatus * --  
* getNetText  * -- 
* gotoNetMovie * -- 
* preloadNetThing  * -- 
* netAbort * -- 
* gotoNetPage * -- 
* getLatestNetID * -- 
* netError * -- 
* netDone * -- 
* netTextResult * -- 
* netMime * -- 
* netLastModDate * -- 
* externalEvent * -- 
* netPresent * -- 
* downloadNetThing  * -- 
* clearCache * -- 
* cacheSize * -- 
* cacheDocVerify * -- 
* proxyServer * -- 
* browserName * -- 
* tellStreamStatus * -- 

 */

namespace Director {

const char *NetLingoXtra::xlibName = "NetLingo";
const XlibFileDesc NetLingoXtra::fileNames[] = {
	{ "netlingo",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {


	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "netStatus", NetLingoXtra::m_netStatus, -1, 0, 500, HBLTIN },
	{ "getNetText", NetLingoXtra::m_getNetText, -1, 0, 500, HBLTIN },
	{ "gotoNetMovie", NetLingoXtra::m_gotoNetMovie, -1, 0, 500, HBLTIN },
	{ "preloadNetThing", NetLingoXtra::m_preloadNetThing, -1, 0, 500, HBLTIN },
	{ "netAbort", NetLingoXtra::m_netAbort, -1, 0, 500, HBLTIN },
	{ "gotoNetPage", NetLingoXtra::m_gotoNetPage, -1, 0, 500, HBLTIN },
	{ "getLatestNetID", NetLingoXtra::m_getLatestNetID, -1, 0, 500, HBLTIN },
	{ "netError", NetLingoXtra::m_netError, -1, 0, 500, HBLTIN },
	{ "netDone", NetLingoXtra::m_netDone, -1, 0, 500, HBLTIN },
	{ "netTextResult", NetLingoXtra::m_netTextResult, -1, 0, 500, HBLTIN },
	{ "netMime", NetLingoXtra::m_netMime, -1, 0, 500, HBLTIN },
	{ "netLastModDate", NetLingoXtra::m_netLastModDate, -1, 0, 500, HBLTIN },
	{ "externalEvent", NetLingoXtra::m_externalEvent, -1, 0, 500, HBLTIN },
	{ "netPresent", NetLingoXtra::m_netPresent, -1, 0, 500, HBLTIN },
	{ "downloadNetThing", NetLingoXtra::m_downloadNetThing, -1, 0, 500, HBLTIN },
	{ "clearCache", NetLingoXtra::m_clearCache, -1, 0, 500, HBLTIN },
	{ "cacheSize", NetLingoXtra::m_cacheSize, -1, 0, 500, HBLTIN },
	{ "cacheDocVerify", NetLingoXtra::m_cacheDocVerify, -1, 0, 500, HBLTIN },
	{ "proxyServer", NetLingoXtra::m_proxyServer, -1, 0, 500, HBLTIN },
	{ "browserName", NetLingoXtra::m_browserName, -1, 0, 500, HBLTIN },
	{ "tellStreamStatus", NetLingoXtra::m_tellStreamStatus, -1, 0, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

NetLingoXtraObject::NetLingoXtraObject(ObjectType ObjectType) :Object<NetLingoXtraObject>("NetLingo") {
	_objType = ObjectType;
}

bool NetLingoXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum NetLingoXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(NetLingoXtra::xlibName);
	warning("NetLingoXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void NetLingoXtra::open(ObjectType type, const Common::Path &path) {
    NetLingoXtraObject::initMethods(xlibMethods);
    NetLingoXtraObject *xobj = new NetLingoXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void NetLingoXtra::close(ObjectType type) {
    NetLingoXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void NetLingoXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("NetLingoXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(NetLingoXtra::m_netStatus, 0)
XOBJSTUB(NetLingoXtra::m_getNetText, 0)
XOBJSTUB(NetLingoXtra::m_gotoNetMovie, 0)
XOBJSTUB(NetLingoXtra::m_preloadNetThing, 0)
XOBJSTUB(NetLingoXtra::m_netAbort, 0)
XOBJSTUB(NetLingoXtra::m_gotoNetPage, 0)
XOBJSTUB(NetLingoXtra::m_getLatestNetID, 0)
XOBJSTUB(NetLingoXtra::m_netError, "OK")
XOBJSTUB(NetLingoXtra::m_netDone, 1)
XOBJSTUB(NetLingoXtra::m_netTextResult, "")
XOBJSTUB(NetLingoXtra::m_netMime, 0)
XOBJSTUB(NetLingoXtra::m_netLastModDate, 0)
XOBJSTUB(NetLingoXtra::m_externalEvent, 0)
XOBJSTUB(NetLingoXtra::m_netPresent, 0)
XOBJSTUB(NetLingoXtra::m_downloadNetThing, 0)
XOBJSTUB(NetLingoXtra::m_clearCache, 0)
XOBJSTUB(NetLingoXtra::m_cacheSize, 0)
XOBJSTUB(NetLingoXtra::m_cacheDocVerify, 0)
XOBJSTUB(NetLingoXtra::m_proxyServer, 0)
XOBJSTUB(NetLingoXtra::m_browserName, "")
XOBJSTUB(NetLingoXtra::m_tellStreamStatus, 0)

}
