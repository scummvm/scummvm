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
 * Plates are People too!
 *
 *************************************/
/* --  PrintOMatic XObject
--  Version 1.1.4, August 8, 1995
--  ©1994-95 Electronic Ink
--
--  STANDARD METHODS
I      mNew
X      mDispose
--
--  DOCUMENT ATTRIBUTES
X      mReset
I      mNewPage
XI     mSetPage, pageNum
IIIII  mSetMargins, left, top, right, bottom
X      mSetPrintableMargins
I      mGetPageWidth
I      mGetPageHeight
--
--  GRAPHICS STATE
XIII   mSetColor, r, g, b    (scale = 0-255)
XI     mSetGray, grayLevel   (scale = 0-100)
XII    mSetPenSize, h, v
XS     mSetTextFont, fontName
XI     mSetTextSize, fontSize
XS     mSetTextStyle, styleNames
XS     mSetTextJust, [ right | left | centered ]
--
--  TEXT ELEMENTS
XIIIII mTextBox, left, top, right, bottom, linkedToPrevious
V      mSetText, textString [, autoAppend]
V      mAppendText, textString [, autoAppend]
V      mAppendTextFile, fileName [, autoAppend]
V      mAppendTextResource, id | name [, autoAppend]
ISII   mDrawText, text, startH, startV
S      mGetInsertionPoint
--
--  MASTER FLOWS
XIIII  mMasterTextBox, left, top, right, bottom
V      mAppendMasterText, textString
V      mAppendMasterTextFiles, fileName
XIIII  mMasterPictBox, left, top, right, bottom
V      mAppendMasterPict, pict | pictFile | pictFolder | pictResID
X      mStageToMasterPict
--
--  GRAPHIC ELEMENTS
XIIII  mStrokedRect, left, top, right, bottom
XIIII  mFilledRect, left, top, right, bottom
XIIIII mStrokedRoundRect, left, top, right, bottom, cornerRadius
XIIIII mFilledRoundRect, left, top, right, bottom, cornerRadius
V      mStrokedOval [, left, top, right, bottom | , centerH, centerV, radius ]
V      mFilledOval [, left, top, right, bottom | , centerH, centerV, radius ]
XIIII  mLine, startH, startV, endH, endV
V      mPicture, pict | pictFile | pictResID, left, top [ , right, bottom ]
V      mStagePicture, left, top , right, bottom [,clipLeft ,clipTop ...] 
V      m1BitStagePicture, left, top , right, bottom [,clipLeft ,clipTop ...] 
V      mEPSFile, fileName, left, top , right, bottom
--
--  PRINTING
II     mSetLandscapeMode, trueOrFalse
XS     mSetDocumentName, name
I      mDoPageSetup
I      mDoJobSetup
XS     mSetProgressMsg, text
V      mSetProgressPict, pict
XII    mSetProgressLoc, left, top
I      mPrintPreview
V      mPrintPicts [, folder]
X      mPrint
--
--  SAVING TO DISK
ISISS  mSavePageSetup, fileName, resID, fileType, fileCreator
ISI    mGetPageSetup, fileName, resID
--
--  MISCELLANEOUS
XI     mHideMessages, trueOrFalse
IS     mSetPageNumSymbol, symbol
IS     mRegister, serialNumber
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/printomatic.h"


namespace Director {

const char *PrintOMaticXObj::xlibName = "PrintOMatic";
const char *PrintOMaticXObj::fileNames[] = {
	"PMATIC",
	"PrintOMatic",
	0
};

static MethodProto xlibMethods[] = {
	{ "New",					PrintOMaticXObj::m_new,						0,	0,	400 },	// D4
	{ "Dispose",				PrintOMaticXObj::m_dispose,					0,	0,	400 },	// D4
	{ "Reset",					PrintOMaticXObj::m_reset,					0,	0,	400 },	// D4
	{ "NewPage",				PrintOMaticXObj::m_newPage,					0,	0,	400 },	// D4
	{ "SetPrintableMargins",	PrintOMaticXObj::m_setPrintableMargins,		0,	0,	400 },	// D4
	{ "GetPageWidth",			PrintOMaticXObj::m_getPageWidth,			0,	0,	400 },	// D4
	{ "GetPageHeight",			PrintOMaticXObj::m_getPageHeight,			0,	0,	400 },	// D4
	{ "Picture",				PrintOMaticXObj::m_picture,					3,	5,	400 },	// D4
	{ "StagePicture",			PrintOMaticXObj::m_stagePicture,			4,	8,	400 },	// D4
	{ "1BitStagePicture",		PrintOMaticXObj::m_1bitStagePicture,		4,	8,	400 },	// D4
	{ "SetLandscapeMode",		PrintOMaticXObj::m_setLandscapeMode,		1,	1,	400 },	// D4
	{ "DoPageSetup",			PrintOMaticXObj::m_doPageSetup,				0,	0,	400 },	// D4
	{ "DoJobSetup",				PrintOMaticXObj::m_doJobSetup,				0,	0,	400 },	// D4
	{ "SetProgressMsg",			PrintOMaticXObj::m_setProgressMsg,			1,	1,	400 },	// D4
	{ "PrintPreview",			PrintOMaticXObj::m_printPreview,			0,	0,	400 },	// D4
	{ "PrintPicts",				PrintOMaticXObj::m_printPicts,				0,	1,	400 },	// D4
	{ "Print",					PrintOMaticXObj::m_print,					0,	0,	400 },	// D4
	{ "Register",				PrintOMaticXObj::m_register,				1,	1,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void PrintOMaticXObj::open(int type) {
	if (type == kXObj) {
		PrintOMaticXObject::initMethods(xlibMethods);
		PrintOMaticXObject *xobj = new PrintOMaticXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PrintOMaticXObj::close(int type) {
	if (type == kXObj) {
		PrintOMaticXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PrintOMaticXObject::PrintOMaticXObject(ObjectType ObjectType) :Object<PrintOMaticXObject>("PrintOMaticXObj") {
	_objType = ObjectType;
}

void PrintOMaticXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void PrintOMaticXObj::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_register(int nargs) {
	Common::String serialNumber = g_lingo->pop().asString();
	warning("PrintOMaticXObj::m_register: Registered with serial \"%s\"", serialNumber.c_str());
}

void PrintOMaticXObj::m_reset(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_reset", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_newPage(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_newPage", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_setPrintableMargins(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_setPrintableMargins", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_getPageWidth(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_getPageWidth", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(-1));
}

void PrintOMaticXObj::m_getPageHeight(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_getPageHeight", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(-1));
}

void PrintOMaticXObj::m_picture(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_picture", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_stagePicture(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_stagePicture", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_1bitStagePicture(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_1bitStagePicture", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_setLandscapeMode(int nargs) {
	// int trueOrFalse = g_lingo->pop.asInt()
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_setLandscapeMode", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_doPageSetup(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_doPageSetup", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(1));
}

void PrintOMaticXObj::m_doJobSetup(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_doJobSetup", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(1));
}

void PrintOMaticXObj::m_setProgressMsg(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_setProgressMsg", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_printPreview(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_printPreview", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_printPicts(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_printPicts", nargs);
	g_lingo->dropStack(nargs);
}

void PrintOMaticXObj::m_print(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_print", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director
