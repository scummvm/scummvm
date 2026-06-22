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
#include "director/lingo/xtras/b/budweb.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra BudWeb
new object me

* bwDownloadFile string URL, string File, string Overwrite, list Dialog, list Options -- downloads URL to file
* bwDownloadFolder string URL, string Folder, string FileSpec, string Overwrite, int getSubFolders, list Dialog, list Options -- downloads folder
* bwDownloadFileList list URLs, string Folder, string Overwrite, list Dialog, list Options -- downloads files
* bwUploadFile string File, string URL, string Overwrite, list Dialog, list Options -- uploads File to URL
* bwUploadFolder string Folder, string URL, string FileSpec, string Overwrite, integer SubFolders, list Dialog, list Options -- uploads Folder to URL
* bwUploadFileList list Files, string URL, string Overwrite, list Dialog, list Options -- uploads Files to URL

* bwFileExists string URL, list Options -- returns 1 if URL exists
* bwFileSize string URL, list Options -- returns size of URL
* bwFileAge string URL, list Options -- returns age of URL
* bwFolderExists string URL, list Options -- returns 1 if URL exists and is a folder
* bwFileList string Folder, string fileSpec, list Options -- return list of Urls in folder
* bwFolderList string Folder, list Options -- return list of folders
* bwCreateFolder string Folder, list Options -- creates folder
* bwDeleteFile string URL, list Options -- deletes file
* bwRenameFile string URL, string NewFile, list Options -- renames file
* bwWriteList string URL, list List, string Key, list Dialog, list Options -- saves list to file
* bwReadList string URL, string Key, list Dialog, list Options -- reads list from URL
* bwWriteFile string URL, string Data, string Key, list Dialog, list Options -- saves text file
* bwReadFile string URL, string Key, list Dialog, list Options -- reads text file

* bwGetFile string URL, string Mode, list Options -- opens file for reading or writing
* bwWriteText integer fileNum, string Text -- writes text to file
* bwReadText integer fileNum, integer Characters -- reads text from file
* bwCloseFile integer fileNum -- closes file
* bwCloseSession -- closes file and session

* bwPing string host, * -- returns time to ping host in milliseconds, with optional timeout
* bwUploadedFile -- returns name of last file uploaded by user
* bwWebResult -- returns last result
* bwSetOption string Option, any Value -- sets dialog or URL option

* bwEncryptText string Text, string Key -- encrypt Text with Key
* bwDecryptText string Text, string Key -- decrypt Text with Key

* bwMsgBox string Msg, string Caption, string Buttons, string Icon, integer DefaultButton -- show message box
* bwMsgBoxEx string Msg, string Caption, string Button1, string Button2, string Button3, string Icon, integer DefButton, string Alignment, string FontName, int FontSize, int FontWeight, int xPos, int yPos  -- shows custom message box
* bwPrompt string Caption, string Instruction, string DefaultText, integer Flags, integer X, integer Y  -- asks for string input
* bwLogIn string Caption, string Instruction, string UserName, string Password, integer Flags, string NameCaption, string PasswordCaption, integer X, integer Y -- asks for Log In with user name and password
* bwBusyDlg string Caption, string Text, integer Flags, integer X, integer Y -- shows a busy dialog
* bwCloseBusyDlg -- closes the busy dialog
* bwProgressDlg string Caption, string Text, integer Flags, integer X, integer Y -- shows a progress dialog
* bwUpdateProgressDlg integer Percent, string Text -- updates progress dialog
* bwCloseProgressDlg -- closes progress dialog
* bwMenuAt list Item, integer Flags, int X, int Y -- shows simple menu at X-Y

* bwRegister string Name, integer Number -- registers Buddy Web

-- Buddy Web Version 1.0
-- Copyright Magic Modules Pty Ltd 2005
-- www.mods.com.au    buddy@mods.com.au
"
 */

namespace Director {

const char *BudwebXtra::xlibName = "Budweb";
const XlibFileDesc BudwebXtra::fileNames[] = {
	{ "budweb",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BudwebXtra::m_new,		 0, 0,	500 },
	{ """,				BudwebXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bwDownloadFile", BudwebXtra::m_bwDownloadFile, 5, 5, 500, HBLTIN },
	{ "bwDownloadFolder", BudwebXtra::m_bwDownloadFolder, 7, 7, 500, HBLTIN },
	{ "bwDownloadFileList", BudwebXtra::m_bwDownloadFileList, 5, 5, 500, HBLTIN },
	{ "bwUploadFile", BudwebXtra::m_bwUploadFile, 5, 5, 500, HBLTIN },
	{ "bwUploadFolder", BudwebXtra::m_bwUploadFolder, 7, 7, 500, HBLTIN },
	{ "bwUploadFileList", BudwebXtra::m_bwUploadFileList, 5, 5, 500, HBLTIN },
	{ "bwFileExists", BudwebXtra::m_bwFileExists, 2, 2, 500, HBLTIN },
	{ "bwFileSize", BudwebXtra::m_bwFileSize, 2, 2, 500, HBLTIN },
	{ "bwFileAge", BudwebXtra::m_bwFileAge, 2, 2, 500, HBLTIN },
	{ "bwFolderExists", BudwebXtra::m_bwFolderExists, 2, 2, 500, HBLTIN },
	{ "bwFileList", BudwebXtra::m_bwFileList, 3, 3, 500, HBLTIN },
	{ "bwFolderList", BudwebXtra::m_bwFolderList, 2, 2, 500, HBLTIN },
	{ "bwCreateFolder", BudwebXtra::m_bwCreateFolder, 2, 2, 500, HBLTIN },
	{ "bwDeleteFile", BudwebXtra::m_bwDeleteFile, 2, 2, 500, HBLTIN },
	{ "bwRenameFile", BudwebXtra::m_bwRenameFile, 3, 3, 500, HBLTIN },
	{ "bwWriteList", BudwebXtra::m_bwWriteList, 5, 5, 500, HBLTIN },
	{ "bwReadList", BudwebXtra::m_bwReadList, 4, 4, 500, HBLTIN },
	{ "bwWriteFile", BudwebXtra::m_bwWriteFile, 5, 5, 500, HBLTIN },
	{ "bwReadFile", BudwebXtra::m_bwReadFile, 4, 4, 500, HBLTIN },
	{ "bwGetFile", BudwebXtra::m_bwGetFile, 3, 3, 500, HBLTIN },
	{ "bwWriteText", BudwebXtra::m_bwWriteText, 2, 2, 500, HBLTIN },
	{ "bwReadText", BudwebXtra::m_bwReadText, 2, 2, 500, HBLTIN },
	{ "bwCloseFile", BudwebXtra::m_bwCloseFile, 1, 1, 500, HBLTIN },
	{ "bwCloseSession", BudwebXtra::m_bwCloseSession, 0, 0, 500, HBLTIN },
	{ "bwPing", BudwebXtra::m_bwPing, -1, 0, 500, HBLTIN },
	{ "bwUploadedFile", BudwebXtra::m_bwUploadedFile, 0, 0, 500, HBLTIN },
	{ "bwWebResult", BudwebXtra::m_bwWebResult, 0, 0, 500, HBLTIN },
	{ "bwSetOption", BudwebXtra::m_bwSetOption, 2, 2, 500, HBLTIN },
	{ "bwEncryptText", BudwebXtra::m_bwEncryptText, 2, 2, 500, HBLTIN },
	{ "bwDecryptText", BudwebXtra::m_bwDecryptText, 2, 2, 500, HBLTIN },
	{ "bwMsgBox", BudwebXtra::m_bwMsgBox, 5, 5, 500, HBLTIN },
	{ "bwMsgBoxEx", BudwebXtra::m_bwMsgBoxEx, 13, 13, 500, HBLTIN },
	{ "bwPrompt", BudwebXtra::m_bwPrompt, 6, 6, 500, HBLTIN },
	{ "bwLogIn", BudwebXtra::m_bwLogIn, 9, 9, 500, HBLTIN },
	{ "bwBusyDlg", BudwebXtra::m_bwBusyDlg, 5, 5, 500, HBLTIN },
	{ "bwCloseBusyDlg", BudwebXtra::m_bwCloseBusyDlg, 0, 0, 500, HBLTIN },
	{ "bwProgressDlg", BudwebXtra::m_bwProgressDlg, 5, 5, 500, HBLTIN },
	{ "bwUpdateProgressDlg", BudwebXtra::m_bwUpdateProgressDlg, 2, 2, 500, HBLTIN },
	{ "bwCloseProgressDlg", BudwebXtra::m_bwCloseProgressDlg, 0, 0, 500, HBLTIN },
	{ "bwMenuAt", BudwebXtra::m_bwMenuAt, 4, 4, 500, HBLTIN },
	{ "bwRegister", BudwebXtra::m_bwRegister, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BudwebXtraObject::BudwebXtraObject(ObjectType ObjectType) :Object<BudwebXtraObject>("Budweb") {
	_objType = ObjectType;
}

bool BudwebXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BudwebXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BudwebXtra::xlibName);
	warning("BudwebXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BudwebXtra::open(ObjectType type, const Common::Path &path) {
    BudwebXtraObject::initMethods(xlibMethods);
    BudwebXtraObject *xobj = new BudwebXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BudwebXtra::close(ObjectType type) {
    BudwebXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BudwebXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BudwebXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BudwebXtra::m_bwDownloadFile, 0)
XOBJSTUB(BudwebXtra::m_bwDownloadFolder, 0)
XOBJSTUB(BudwebXtra::m_bwDownloadFileList, 0)
XOBJSTUB(BudwebXtra::m_bwUploadFile, 0)
XOBJSTUB(BudwebXtra::m_bwUploadFolder, 0)
XOBJSTUB(BudwebXtra::m_bwUploadFileList, 0)
XOBJSTUB(BudwebXtra::m_bwFileExists, 0)
XOBJSTUB(BudwebXtra::m_bwFileSize, 0)
XOBJSTUB(BudwebXtra::m_bwFileAge, 0)
XOBJSTUB(BudwebXtra::m_bwFolderExists, 0)
XOBJSTUB(BudwebXtra::m_bwFileList, 0)
XOBJSTUB(BudwebXtra::m_bwFolderList, 0)
XOBJSTUB(BudwebXtra::m_bwCreateFolder, 0)
XOBJSTUB(BudwebXtra::m_bwDeleteFile, 0)
XOBJSTUB(BudwebXtra::m_bwRenameFile, 0)
XOBJSTUB(BudwebXtra::m_bwWriteList, 0)
XOBJSTUB(BudwebXtra::m_bwReadList, 0)
XOBJSTUB(BudwebXtra::m_bwWriteFile, 0)
XOBJSTUB(BudwebXtra::m_bwReadFile, 0)
XOBJSTUB(BudwebXtra::m_bwGetFile, 0)
XOBJSTUB(BudwebXtra::m_bwWriteText, 0)
XOBJSTUB(BudwebXtra::m_bwReadText, 0)
XOBJSTUB(BudwebXtra::m_bwCloseFile, 0)
XOBJSTUB(BudwebXtra::m_bwCloseSession, 0)
XOBJSTUB(BudwebXtra::m_bwPing, 0)
XOBJSTUB(BudwebXtra::m_bwUploadedFile, 0)
XOBJSTUB(BudwebXtra::m_bwWebResult, 0)
XOBJSTUB(BudwebXtra::m_bwSetOption, 0)
XOBJSTUB(BudwebXtra::m_bwEncryptText, 0)
XOBJSTUB(BudwebXtra::m_bwDecryptText, 0)
XOBJSTUB(BudwebXtra::m_bwMsgBox, 0)
XOBJSTUB(BudwebXtra::m_bwMsgBoxEx, 0)
XOBJSTUB(BudwebXtra::m_bwPrompt, 0)
XOBJSTUB(BudwebXtra::m_bwLogIn, 0)
XOBJSTUB(BudwebXtra::m_bwBusyDlg, 0)
XOBJSTUB(BudwebXtra::m_bwCloseBusyDlg, 0)
XOBJSTUB(BudwebXtra::m_bwProgressDlg, 0)
XOBJSTUB(BudwebXtra::m_bwUpdateProgressDlg, 0)
XOBJSTUB(BudwebXtra::m_bwCloseProgressDlg, 0)
XOBJSTUB(BudwebXtra::m_bwMenuAt, 0)
XOBJSTUB(BudwebXtra::m_bwRegister, 0)
XOBJSTUB(BudwebXtra::m_", 0)

}
