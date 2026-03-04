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
 * The Legend of Lotus Spring
 *
 *************************************/

/*
* From "Director in a Nutshell":
+----------------------------------------------------------------------------------------------------------+--------+--------+--------+
| Command                                                                                                  | System | Member | Sprite |
+==========================================================================================================+========+========+========+
| canUseQuicktimeStreaming()                                                                               | ✓      |        |        |
| isUsingQuicktimeStreaming()                                                                              | ✓      |        |        |
| interface (xtra "QuickTimeSupport")                                                                      | ✓      |        |        |
| mMessageList (xtra "QuickTimeSupport")                                                                   | ✓      |        |        |
| new (#quickTimeMedia)                                                                                    | ✓      |        |        |
| qtRegisterAccessKey (category, key)                                                                      | ✓      |        |        |
| qtUnRegisterAccessKey (category, key)                                                                    | ✓      |        |        |
| quickTimeVersion()                                                                                       | ✓      |        |        |
| setTrackEnabled (sprite qtSprite, track)                                                                 |        |        | ✓      |
| trackCount (member qtMember); trackCount (sprite qtSprite)                                               |        | ✓      | ✓      |
| trackEnabled (sprite qtSprite, track)                                                                    |        |        | ✓      |
| trackText (sprite qtSprite, track)                                                                       |        |        | ✓      |
| trackType (member qtMember, track); trackType (sprite qtSprite, track)                                   |        | ✓      | ✓      |
| useQuickTimeStreaming (TRUE | FALSE)                                                                     | ✓      |        |        |
| VREnableHotSpot (sprite qtvrSprite, hotSpotID, TRUE | FALSE)                                             |        |        | ✓      |
| VRGetHotSpotRect (sprite qtvrSprite, hotSpotID)                                                          |        |        | ✓      |
| VRNudge (sprite qtvrSprite, #left | #upLeft | #up | #upRight | #right | #downRight | #down | #downLeft)  |        |        | ✓      |
| VRPtToHotSpotID (sprite qtvrSprite, point(the mouseH, the mouseV))                                       |        |        | ✓      |
| VRswing (sprite qtvrSprite, pan, tilt, fieldOfView)                                                      |        |        | ✓      |
+----------------------------------------------------------------------------------------------------------+--------+--------+--------+
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/q/qtsupport.h"


namespace Director {

const char *QTSupport::xlibName = "QuickTimeSupport";
const XlibFileDesc QTSupport::fileNames[] = {
	{ "QuickTime Asset",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",						QTSupport::m_new,						1, 1,  600 },
	{ "CanUseQuickTimeStreaming",	QTSupport::m_canUseQuicktimeStreaming,	0, 0,  600 },
	{ "UseQuickTimeStreaming",		QTSupport::m_useQuickTimeStreaming,		1, 1,  600 },
	{ "IsUsingQuickTimeStreaming",	QTSupport::m_isUsingQuicktimeStreaming,	0, 0,  600 },
	// { "interface",					QTSupport::m_interface,					1, 1,  600 },
	// { "mMessageList",				QTSupport::m_mMessageList,				1, 1,  600 },
	// { "qtRegisterAccessKey",		QTSupport::m_qtRegisterAccessKey,		2, 2,  600 },
	// { "qtUnRegisterAccessKey",		QTSupport::m_qtUnRegisterAccessKey,		2, 2,  600 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "QuickTimeVersion",			QTSupport::m_quickTimeVersion,			0, 0,  600, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

QTSupportXObject::QTSupportXObject(ObjectType ObjectType) :Object<QTSupportXObject>("RegistryReader") {
	_objType = ObjectType;
}

bool QTSupportXObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum QTSupportXObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(QTSupport::xlibName);
	warning("QTSupport::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void QTSupport::open(ObjectType type, const Common::Path &path) {
    QTSupportXObject::initMethods(xlibMethods);
    QTSupportXObject *xobj = new QTSupportXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void QTSupport::close(ObjectType type) {
    QTSupportXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();
}


XOBJSTUB(QTSupport::m_new, "")
XOBJSTUB(QTSupport::m_canUseQuicktimeStreaming, "")
XOBJSTUB(QTSupport::m_isUsingQuicktimeStreaming, "")
XOBJSTUB(QTSupport::m_interface, "")
XOBJSTUB(QTSupport::m_mMessageList, "")
XOBJSTUB(QTSupport::m_qtRegisterAccessKey, "")
XOBJSTUB(QTSupport::m_qtUnRegisterAccessKey, "")
XOBJSTUB(QTSupport::m_quickTimeVersion, 4)
XOBJSTUB(QTSupport::m_useQuickTimeStreaming, "")
} // End of namespace Director
