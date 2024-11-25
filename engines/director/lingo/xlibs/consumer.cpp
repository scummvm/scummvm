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
#include "director/lingo/xlibs/consumer.h"

/*
 * Used in: Bob Winkle Solves Life's Greatest Mysteries
 *
-- Consumer dialog xobject. 02Aug95 JA
Consumer
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
S      mName               --Returns the XObject name (Consumer)
SII        mSettingsDlg            --Shows the settings dialog and returns results
ISIS       mPopHotword         --Pops up a hot word window
SSSSS      mExplore                        --Shows the Eplore dialog
SSSS   mPrefSetValue               --Set a preference file entry
SSS    mPrefGetValue       --Get a preference file entry
SSSSSS mContentLock                --Shows the content lock dialog
SSI        mGetRecord                      --Retrieves a data file record
SSS        mGetWinIniEntry         --Retrieves an entry from the WIN.INI
I      mAboutDlg                   --Displays the About Box
IS     mRegisterDlg                --Displays the Registration Dialog
I      mLockedDlg                  --Displays the Term Locked Dialog
II         mCheckCD                        --Checks for the presence of the CD
I          mHackMenu                       --Hack to destroy menu
 */

namespace Director {

const char *const ConsumerXObj::xlibName = "Consumer";
const XlibFileDesc ConsumerXObj::fileNames[] = {
	{ "consumer",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					ConsumerXObj::m_new,			 0, 0,	400 },	// D4
	{ "dispose",				ConsumerXObj::m_dispose,		 0, 0,	400 },	// D4
	{ "name",					ConsumerXObj::m_name,			 0, 0,	400 },	// D4
	{ "settingsDlg",			ConsumerXObj::m_settingsDlg,	 2, 2,	400 },	// D4
	{ "popHotword",				ConsumerXObj::m_popHotword,		 3, 3,	400 },	// D4
	{ "explore",				ConsumerXObj::m_explore,		 4, 4,	400 },	// D4
	{ "prefSetValue",			ConsumerXObj::m_prefSetValue,	 3, 3,	400 },	// D4
	{ "prefGetValue",			ConsumerXObj::m_prefGetValue,	 2, 2,	400 },	// D4
	{ "contentLock",			ConsumerXObj::m_contentLock,	 5, 5,	400 },	// D4
	{ "getRecord",				ConsumerXObj::m_getRecord,		 2, 2,	400 },	// D4
	{ "getWinIniEntry",			ConsumerXObj::m_getWinIniEntry,	 2, 2,	400 },	// D4
	{ "aboutDlg",				ConsumerXObj::m_aboutDlg,		 0, 0,	400 },	// D4
	{ "registerDlg",			ConsumerXObj::m_registerDlg,	 1, 1,	400 },	// D4
	{ "lockedDlg",				ConsumerXObj::m_lockedDlg,		 0, 0,	400 },	// D4
	{ "checkCD",				ConsumerXObj::m_checkCD,		 1, 1,	400 },	// D4
	{ "hackMenu",				ConsumerXObj::m_hackMenu,		 0, 0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

ConsumerXObject::ConsumerXObject(ObjectType ObjectType) :Object<ConsumerXObject>("Consumer") {
	_objType = ObjectType;
}

void ConsumerXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		ConsumerXObject::initMethods(xlibMethods);
		ConsumerXObject *xobj = new ConsumerXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void ConsumerXObj::close(ObjectType type) {
	if (type == kXObj) {
		ConsumerXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void ConsumerXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ConsumerXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ConsumerXObj::m_dispose)
XOBJSTUB(ConsumerXObj::m_name, "Consumer")
XOBJSTUB(ConsumerXObj::m_settingsDlg, "")
XOBJSTUB(ConsumerXObj::m_popHotword, 0)
XOBJSTUB(ConsumerXObj::m_explore, "")
XOBJSTUB(ConsumerXObj::m_prefSetValue, "")
XOBJSTUB(ConsumerXObj::m_prefGetValue, "")
XOBJSTUB(ConsumerXObj::m_contentLock, "")
XOBJSTUB(ConsumerXObj::m_getRecord, "0,0,0") // used to bypass the locked question check
XOBJSTUB(ConsumerXObj::m_getWinIniEntry, "")
XOBJSTUB(ConsumerXObj::m_aboutDlg, 0)
XOBJSTUB(ConsumerXObj::m_registerDlg, 0)
XOBJSTUB(ConsumerXObj::m_lockedDlg, 0)
XOBJSTUB(ConsumerXObj::m_checkCD, 1)
XOBJSTUB(ConsumerXObj::m_hackMenu, 0)

}
