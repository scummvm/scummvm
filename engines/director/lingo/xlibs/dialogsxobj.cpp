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
* Total Distortion
*
*************************************/

/*
 * -- Dialogs XObject. Written by Scott Kildall. Copyright (c) Red Eye  Software, December 11th, 1994.
 * Compuserve:72703,451 AOL:KILDALL  APPLELINK:S.KILDALL
 * Internet:72703.451@compuserve.com
 * Licensed for the MediaBook CD for Director
 * Published by gray matter design (415) 243-0394
 *
 * Dialogs
 * X        mNew          --Creates a new instance of the XObject
 * SSSS     mPutFile, Dialog Title, Default Name, Default Extension       --Displays a save dialog box
 * SSSS     mGetFile, Dialog Title, Default Name, Default Extension       --Displays an open dialog box
 *
 * -- How it is called:
 * set GameFileFULLPATH to gDialogsObj(mPutFile, "Save your Total Distortion Game File!", "TDGame", "TDG")
 * set GameFileFULLPATH to gDialogsObj(mGetFile, "Find your Total Distortion Game File!", "*.TDG", "TDG")
*/

#include "gui/filebrowser-dialog.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/dialogsxobj.h"

namespace Director {

const char *const DialogsXObj::xlibNames[] = {
	"DialogS",
	nullptr
};

const XlibFileDesc DialogsXObj::fileNames[] = {
	{ "DialogS",	nullptr },
	{ "shaREQUE",	nullptr }, // TD loads this up using openXLib("@:shaREQUE.DLL")
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",               DialogsXObj::m_new,              0, 0,  400 },  // D4
	{ "GetFile",           DialogsXObj::m_getFile,          3, 3,  400 },  // D4
	{ "PutFile",           DialogsXObj::m_putFile,          3, 3,  400 },  // D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void DialogsXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		DialogsXObject::initMethods(xlibMethods);
		DialogsXObject *xobj = new DialogsXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->exposeXObject(xlibNames[i], xobj);
		}
	}
}

void DialogsXObj::close(ObjectType type) {
	if (type == kXObj) {
		DialogsXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->_globalvars[xlibNames[i]] = Datum();
		}
	}
}

DialogsXObject::DialogsXObject(ObjectType ObjectType) : Object<DialogsXObject>("DialogS") {
	_objType = ObjectType;
}

void DialogsXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void DialogsXObj::m_putFile(int nargs) {
	Common::String extn = g_lingo->pop().asString();
	Common::String name = g_lingo->pop().asString();
	Common::String title = g_lingo->pop().asString();

	Common::String prefix = savePrefix();
	Common::String mask = prefix + "*." + extn + ".txt";
	Common::String filename = name;

	GUI::FileBrowserDialog browser(title.c_str(), "txt", GUI::kFBModeSave, mask.c_str());
	if (browser.runModal() > 0) {
		filename = browser.getResult();
		if (!filename.matchString(mask)) // If user choose to create new file (rather than overwriting existing one)
			filename = Common::String::format("C:\\%s%s", prefix.c_str(), filename.c_str());
		else {
			// Here user chose existing save file to be overwritten, so format it before sending! (To prevent duplicate extensions)
			const Common::String suffx = "." + extn;
			Common::replace(filename, suffx, "");
		}
	}
	warning("DialogsXObj::m_putFile return filename: %s", filename.c_str());
	g_lingo->push(Datum(filename));
}

void DialogsXObj::m_getFile(int nargs) {
	Common::String extn = g_lingo->pop().asString();
	Common::String name = g_lingo->pop().asString();
	Common::String title = g_lingo->pop().asString();

	Common::String prefix = savePrefix();
	Common::String mask = prefix + "*." + extn + ".txt";
	Common::String fileName = name;

	GUI::FileBrowserDialog browser(title.c_str(), "txt", GUI::kFBModeLoad, mask.c_str());
	if (browser.runModal() > 0) {
		Common::String path = browser.getResult();
		fileName = Common::String::format("C:\\%s", path.c_str()); // Create fullpath from this name, as: C:\filename.TDG.txt!
	}
	warning("DialogsXObj::m_getFile return filename: %s", fileName.c_str());
	g_lingo->push(Datum(fileName));
}

} // End of namespace Director
