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
#include "director/types.h"
#include "gui/message.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/askuser.h"

namespace Director {

/**************************************************
 *
 * USED IN:
 * DEVO Presents: Adventures of the Smart Patrol
 *
 **************************************************/

/*
 * -- AskUser XObject. Copyright 1996 Inscape v1.0 24May96 BDL
 * AskUser
 * I      mNew                                                                --Creates a new instance
 * X      mDispose                                                            --Disposes XObject instance
 * SSSS               mAsk                                                                                          --Data to display in the message box.
*/

const char *const AskUser::xlibName = "AskUser";
const XlibFileDesc AskUser::fileNames[] = {
	{ "AskUser",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					AskUser::m_new,			 0, 0,	400 },	// D4
	{ "ask",					AskUser::m_ask,			 3, 3,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

AskUserXObject::AskUserXObject(ObjectType ObjectType) :Object<AskUserXObject>("AskUser") {
	_objType = ObjectType;
}

void AskUser::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		AskUserXObject::initMethods(xlibMethods);
		AskUserXObject *xobj = new AskUserXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void AskUser::close(ObjectType type) {
	if (type == kXObj) {
		AskUserXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void AskUser::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AskUser::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void AskUser::m_ask(int nargs) {
	if (nargs != 3) {
		warning("AskUser::m_ask: expected 3 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum("Ok"));
		return;
	}
	Datum text = g_lingo->pop();
	Datum caption = g_lingo->pop(); // ScummVM doesn't have a title for message boxes, not used
	Datum mbType = g_lingo->pop();
	if (text.type != STRING) {
		warning("AskUser::m_ask: expected text to be a string, not %s", text.type2str());
		g_lingo->push(Datum("Ok"));
		return;
	}
	if (mbType.type != STRING) {
		warning("AskUser::m_ask: expected mbType to be a string, not %s", mbType.type2str());
		g_lingo->push(Datum("Ok"));
		return;
	}

	Common::U32String defaultButton;
	Common::U32StringArray altButtons;

	if (mbType.u.s->equals("YesNoCancel")) {
		defaultButton = Common::U32String("Yes");
		altButtons.push_back(Common::U32String("No"));
		altButtons.push_back(Common::U32String("Cancel"));
	} else if (mbType.u.s->equals("YesNo")) {
		defaultButton = Common::U32String("Yes");
		altButtons.push_back(Common::U32String("No"));
	} else if (mbType.u.s->equals("OkCancel")) {
		defaultButton = Common::U32String("OK");
		altButtons.push_back(Common::U32String("Cancel"));
	} else if (mbType.u.s->equals("Ok")) {
		defaultButton = Common::U32String("OK");
	} else {
		warning("AskUser::m_ask: unhandled mbType %s, falling back to Ok", mbType.u.s->c_str());
		defaultButton = Common::U32String("OK");
	}

	g_director->_wm->clearHandlingWidgets();
	GUI::MessageDialog dialog(Common::U32String(text.u.s->c_str()), defaultButton, altButtons);
	int result = dialog.runModal();

	if (result == GUI::kMessageOK) {
		g_lingo->push(Datum(Common::String(defaultButton)));
	} else if ((result >= GUI::kMessageAlt) && (result < GUI::kMessageAlt + (int)altButtons.size())) {
		g_lingo->push(Datum(Common::String(altButtons[result - GUI::kMessageAlt])));
	} else {
		warning("AskUser::m_ask: got unexpected dialog result of %d", result);
		g_lingo->push(Datum("Ok"));
	}
}

}
