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
 * Learning CorelDRAW 3
 *
 *************************************/
/*
 * -- JITDraw3 External Factory. 16Feb93 PTM
 * JITDraw3
 * I      mNew                --Creates a new instance of the XObject
 * X      mDispose            --Disposes of XObject instance
 * S      mName               --Returns the XObject name (JITDraw3)
 * I      mCheckRegistration
 * I      mCheckIfCDROM
 * S      mUserName
 * S      mUserCompany
 * IS      mSetSignature
 * S      mGetSignature
 * S      mPreRegister
 * II     mBookMark1
 * II     mBookMark2
 * II     mBookMark3
 * II     mBookMark4
 * II     mBookMark5
 * II     mBookMark6
 * I              mAddDrawButton
 * I              mRemoveDrawButton
 * I              mGotoDraw
 * I              mLoadDraw
 * I              mIsDrawLoaded
 * ISS    mMsgOkCancel
 * ISS    mMsgOk
 * ISS    mMsgYesNo
 * I    mOrder
 */

#include "common/system.h"
#include "common/translation.h"

#include "gui/message.h"

// #include "audio/mixer.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/jitdraw3.h"


namespace Director {

const char *const JITDraw3XObj::xlibName = "JITDraw3";
const XlibFileDesc JITDraw3XObj::fileNames[] = {
	{ "JITDraw3",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "New",				JITDraw3XObj::m_new,			  0, 0,	400 },	// D4
	{ "Dispose",			JITDraw3XObj::m_dispose,		  0, 0,	400 },	// D4
	{ "CheckIfCDROM",		JITDraw3XObj::m_checkifcdrom,	  0, 0,	400 },	// D4
	{ "MsgOkCancel",		JITDraw3XObj::m_msgokcancel,	  2, 2,	400 },	// D4
	{ "MsgOk",				JITDraw3XObj::m_msgok,			  2, 2,	400 },	// D4
	{ "MsgYesNo",			JITDraw3XObj::m_msgyesno,		  2, 2,	400 },	// D4
	{ "GotoDraw",			JITDraw3XObj::m_gotodraw,		  0, 0,	400 },	// D4
	{ "AddDrawButton",		JITDraw3XObj::m_adddrawbutton,	  0, 0,	400 },	// D4
	{ "RemoveDrawButton",	JITDraw3XObj::m_removedrawbutton, 0, 0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void JITDraw3XObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		JITDraw3XObject::initMethods(xlibMethods);
		JITDraw3XObject *xobj = new JITDraw3XObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void JITDraw3XObj::close(ObjectType type) {
	if (type == kXObj) {
		JITDraw3XObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


JITDraw3XObject::JITDraw3XObject(ObjectType ObjectType) :Object<JITDraw3XObject>("JITDraw3") {
	_objType = ObjectType;
}

void JITDraw3XObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(JITDraw3XObj::m_dispose)
XOBJSTUB(JITDraw3XObj::m_checkifcdrom, 0)

void JITDraw3XObj::m_msgokcancel(int nargs) {
	Common::U32String caption = g_lingo->pop().asString();  // Title of the message box
	Common::U32String message = g_lingo->pop().asString();
	GUI::MessageDialog dialog(message, _("Ok"), _("Cancel"));
	int result = dialog.runModal();
	g_lingo->push(Datum(result == GUI::kMessageOK ? 1 : 0));
}

void JITDraw3XObj::m_msgok(int nargs) {
	Common::U32String caption = g_lingo->pop().asString();  // Title of the message box
	Common::U32String message = g_lingo->pop().asString();
	GUI::MessageDialog dialog(message, _("Ok"));
	dialog.runModal();
	g_lingo->push(Datum());
}

void JITDraw3XObj::m_msgyesno(int nargs) {
	Common::U32String caption = g_lingo->pop().asString();  // Title of the message box
	Common::U32String message = g_lingo->pop().asString();
	GUI::MessageDialog dialog(message, _("Yes"), _("No"));
	int result = dialog.runModal();
	g_lingo->push(Datum(result == GUI::kMessageOK ? 1 : 0));
}

XOBJSTUB(JITDraw3XObj::m_gotodraw, 3)
XOBJSTUB(JITDraw3XObj::m_adddrawbutton, 0)
XOBJSTUB(JITDraw3XObj::m_removedrawbutton, 0)

} // End of namespace Director
