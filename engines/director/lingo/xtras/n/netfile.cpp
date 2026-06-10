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
#include "director/lingo/xtras/n/netfile.h"

/**************************************************
 *
 * USED IN:
 * Loewenzahn 1 / 2 / 3 / 4 / 8 / Adventskalender
 * TKKG 4 / 7 / 8 / 9 / 10 / 11 / 13 / 14
 * Oscar the Balloonist Discovers the Sea
 *
 **************************************************/

/*
 * Macromedia NetFile Xtra (NETFILE.X32 / NETFILE.X16), part of the NetLingo
 * family. It builds its msgTable at runtime, so the method table could not be
 * extracted statically with devtools/director-generate-xobj-stub.py. The
 * Loewenzahn titles auto-load it but call none of its handlers, so this is an
 * empty stub to register an opener and make the Xtra exist. To fill in the real
 * handlers, load it in real Director, run `put mMessageList(xtra("NetFile"))`,
 * save the output and re-run the generator.
 */

namespace Director {

const char *NetFileXtra::xlibName = "NetFile";
const XlibFileDesc NetFileXtra::fileNames[] = {
	{ "NETFILE",	nullptr },
	{ nullptr,		nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",	NetFileXtra::m_new,	0, 0, 500 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	// postNetText(url, propList [, platform]) is the network-send used by the
	// "e-mail/postcard versenden" and highscore-upload features. Registered so
	// those scripts resolve the handler instead of crashing; see the stub below
	// for why it is intentionally a no-op.
	{ "postNetText",	NetFileXtra::m_postNetText,	-1, 0,	500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

NetFileXtraObject::NetFileXtraObject(ObjectType ObjectType) :Object<NetFileXtraObject>("NetFile") {
	_objType = ObjectType;
}

bool NetFileXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum NetFileXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(NetFileXtra::xlibName);
	warning("NetFileXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void NetFileXtra::open(ObjectType type, const Common::Path &path) {
	NetFileXtraObject::initMethods(xlibMethods);
	NetFileXtraObject *xobj = new NetFileXtraObject(type);
	if (type == kXtraObj) {
		g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
	g_lingo->exposeXObject(xlibName, xobj);
	g_lingo->initBuiltIns(xlibBuiltins);
}

void NetFileXtra::close(ObjectType type) {
	NetFileXtraObject::cleanupMethods();
	g_lingo->cleanupBuiltIns(xlibBuiltins);
	g_lingo->_globalvars[xlibName] = Datum();
}

void NetFileXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("NetFileXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void NetFileXtra::m_postNetText(int nargs) {
	// postNetText(url, propList [, platform]) would HTTP-POST user-entered data
	// (e-mail/postcard text, highscores) to a remote server. We deliberately do
	// NOT perform any networking or transmit data anywhere: just drop the
	// arguments and hand back a dummy network ID so the polling wait-loop
	// (netDone/netError/netTextResult) completes gracefully instead of crashing.
	g_lingo->printSTUBWithArglist("NetFileXtra::m_postNetText", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

} // End of namespace Director
