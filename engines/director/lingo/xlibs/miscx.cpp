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
#include "director/lingo/xlibs/miscx.h"

/**************************************************
 *
 * USED IN:
 * Pippi
 * AMBER: Journeys Beyond
 * Total Distortion
 * Backpacker
 *
 *************************************/

/*
 * -- Misc_X, Misc Utilities XObject v2.0
 * Misc_X
 * I      mNew
 * X      mDispose
 * S      mBootName                     -- get the name of the boot disk
 * S      mWindowsDirectory             -- get the path to the Windows directory
 * IS     mFileExists, filePath             -- return 1 if file is there, otherwise 0
 * ISS    mCopyFile, sourcePath, destPath   -- return 1 if successful, otherwise 0
 * IS     mFolderExists, folderPath         -- return 1 if folder exists, otherwise 0
 * IS     mInsureFolder, folderPath         -- create a full path to the folder, return possible error
 * XS     mDeleteFolder, folderPath         -- delete folder along with contents
 * SS     mFileList, fontainerPath          -- return list of files in the folder
 * SSSSS  mAsk, query, default, btnOK, btnCancel  -- dialog box that accepts user input
 * SSSSS  mAnswer, query, btnL, btnM, btnR        -- alert type dialog box with variable number of buttons
 * IS     mSpaceOnVol, volumeName     -- return the number of free bytes on volume
 * IS     mDeleteGroup, groupName     -- delete program group under Windows
 * ----
 * -- Copyright 1994 Sanctuary Woods --
 * --    written by Bob McKay and Brian Parkinson
 * --
 * -- Append folder name in mInsureFolder with a colon
 * --   gXObject(mInsureFolder, the pathName & "bands:genesis:"
 * --
 * -- The mAsk dialog box returns the text field if btnOk clicked, otherwise the btnCancel text
 * -- The mAnswer dialog box returns the text of the button clicked
 * --
 * -- For mAnswer dialog box, the number of buttons is variable
 * --   gXObject(mAnswer, "Are you sure ?", "", "Yes", "No")  -- two buttons
 * --   gXObject(mAnswer, "A simple alert box", "", "", "Ok") -- one button
 * ----
 */

namespace Director {

const char *const MiscX::xlibName = "Misc_X";
const XlibFileDesc MiscX::fileNames[] = {
	{ "MISC_X",		nullptr },
	{ "sharCOPY",	nullptr }, // TD loads this up using openXLib("@:sharCOPY.DLL")
	{ "BPXLIB",		nullptr }, // Backpacker
	{ nullptr, 		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					MiscX::m_new,				0,	0,	400 },	// D4
	{ "dispose",				MiscX::m_dispose,			0,	0,	400 },	// D4
	{ "bootName",				MiscX::m_bootName,			0,	0,	400 },	// D4
	{ "windowsDirectory",		MiscX::m_windowsDirectory,	0,	0,	400 },	// D4
	{ "fileExists",				MiscX::m_fileExists,		1,	1,	400 },	// D4
	{ "copyFile",				MiscX::m_copyFile,			2,	2,	400 },	// D4
	{ "folderExists",			MiscX::m_folderExists,		1,	1,	400 },	// D4
	{ "insureFolder",			MiscX::m_insureFolder,		1,	1,	400 },	// D4
	{ "prefsFolder",			MiscX::m_prefsFolder,		0,	0,	400 },	// D4
	{ "deleteFolder",			MiscX::m_deleteFolder,		1,	1,	400 },	// D4
	{ "fileList",				MiscX::m_fileList,			1,	1,	400 },	// D4
	{ "ask",					MiscX::m_ask,				4,	4,	400 },	// D4
	{ "answer",					MiscX::m_answer,			4,	4,	400 },	// D4
	{ "spaceOnVol",				MiscX::m_spaceOnVol,		1,	1,	400 },	// D4
	{ "deleteGroup",			MiscX::m_deleteGroup,		1,	1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

MiscXObject::MiscXObject(ObjectType ObjectType) :Object<MiscXObject>("MiscX") {
	_objType = ObjectType;
}

void MiscX::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MiscXObject::initMethods(xlibMethods);
		MiscXObject *xobj = new MiscXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MiscX::close(ObjectType type) {
	if (type == kXObj) {
		MiscXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MiscX::m_new(int nargs) {
	if (nargs != 0) {
		warning("MiscX::m_new: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MiscX::m_dispose)
XOBJSTUB(MiscX::m_bootName, "")
XOBJSTUB(MiscX::m_windowsDirectory, "")
XOBJSTUB(MiscX::m_fileExists, 0)
XOBJSTUB(MiscX::m_copyFile, 0)
XOBJSTUB(MiscX::m_folderExists, 0)
XOBJSTUB(MiscX::m_insureFolder, 0)
XOBJSTUB(MiscX::m_prefsFolder, "")
XOBJSTUBNR(MiscX::m_deleteFolder)
XOBJSTUB(MiscX::m_fileList, "")
XOBJSTUB(MiscX::m_ask, "")
XOBJSTUB(MiscX::m_answer, "")
XOBJSTUB(MiscX::m_spaceOnVol, 0)
XOBJSTUB(MiscX::m_deleteGroup, 0)

}
