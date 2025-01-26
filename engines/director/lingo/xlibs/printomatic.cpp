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
#include "director/lingo/xlibs/printomatic.h"

/**************************************************
 *
 * USED IN:
 * Plates are People Too!
 * I Spy
 *
 **************************************************/

/*
--  PrintOMatic XObject
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


-- xtra PrintOMatic
--
-- PrintOMatic Xtra
-- Version 1.5.3
-- Copyright 1994-97 Electronic Ink
-- Published by g/matter, inc.
--
-- Product Information: http://www.gmatter.com/
-- Technical Support & Updates: <support@gmatter.com>
--
--
-- CREATE/DESTROY/RESET A DOCUMENT
new object
forget object
reset object
--
-- DOCUMENT/JOB SETUP
doPageSetup object      -- returns TRUE or FALSE
doJobSetup object       -- returns TRUE or FALSE
--
-- DOCUMENT ATTRIBUTES
setDocumentName object, string name
setLandscapeMode object, boolean landscape
setMargins object, rect margins
setPrintableMargins object
getPageWidth object   -- returns page width
getPageHeight object  -- returns page height
getPaperWidth object  -- returns paper width
getPaperHeight object -- returns paper height
--
-- CREATE/SET PAGES
newPage object -- returns page number
setPage object, int pageNumber
--
-- TEXT/GRAPHIC ATTRIBUTES
setTextFont object, string fontName -- returns TRUE if font was set
setTextSize object, int pointSize
setTextStyle object, string styleCodes
setTextJust object, string justification
setTextLineSpacing object, int spacing
setColor object, int red, int green, int blue
setGray object, int graylevel
setLineWeight object, int pointSize
--
-- GRAPHIC ELEMENTS
drawRect object, rect bounds, boolean filled
drawLine object, point start, point end
drawRoundRect object, rect bounds, int cornerRadius, boolean filled
drawOval object, rect bounds, boolean filled
drawText object, string text, point location
drawPicture object, *      -- castmem or fileName, location (point or rect)
drawStagePicture object, * -- location (point or rect), stage portion (rect)
--
-- CREATE FRAMES AND APPEND CONTENTS
newFrame object, rect bounds, boolean linkedToPrevious
append object, * any
appendFile object, * fileName
getInsertionPoint object -- returns string "page, x, y"
--
-- CUSTOMIZE THE PROGRESS BOX
setProgressMsg object, string message
setProgressPict object, * pictCastMember
setProgressLoc object, point location
--
-- PRINT OR PREVIEW
* printPreview *
* print *
--
-- MISCELLANEOUS
hideMessages object, boolean hide
setPageNumSymbol object, string symbol
+ register object, string serialNumber -- returns TRUE or FALSE
+ setLowMemLimits object, globalHeap, localHeap

 */

namespace Director {

const char *PrintOMaticXObj::xlibName = "PrintOMatic";
const XlibFileDesc PrintOMaticXObj::fileNames[] = {
	{ "PrintOMatic",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				PrintOMaticXObj::m_new,		 0, 0,	400 },
	{ "dispose",				PrintOMaticXObj::m_dispose,		 0, 0,	400 },
	{ "forget",				PrintOMaticXObj::m_dispose,		 0, 0,	500 },
	{ "reset",				PrintOMaticXObj::m_reset,		 0, 0,	400 },
	{ "newPage",				PrintOMaticXObj::m_newPage,		 0, 0,	400 },
	{ "setPage",				PrintOMaticXObj::m_setPage,		 1, 1,	400 },
	{ "setMargins",				PrintOMaticXObj::m_setMargins,		 4, 4,	400 },
	{ "setPrintableMargins",				PrintOMaticXObj::m_setPrintableMargins,		 0, 0,	400 },
	{ "getPageWidth",				PrintOMaticXObj::m_getPageWidth,		 0, 0,	400 },
	{ "getPageHeight",				PrintOMaticXObj::m_getPageHeight,		 0, 0,	400 },
	{ "getPaperWidth",				PrintOMaticXObj::m_getPaperWidth,		 0, 0,	500 },
	{ "getPaperHeight",				PrintOMaticXObj::m_getPaperHeight,		 0, 0,	500 },
	{ "setColor",				PrintOMaticXObj::m_setColor,		 3, 3,	400 },
	{ "setGray",				PrintOMaticXObj::m_setGray,		 1, 1,	400 },
	{ "setPenSize",				PrintOMaticXObj::m_setPenSize,		 2, 2,	400 },
	{ "setLineWeight",				PrintOMaticXObj::m_setLineWeight,		 1, 1,	500 },
	{ "setTextFont",				PrintOMaticXObj::m_setTextFont,		 1, 1,	400 },
	{ "setTextSize",				PrintOMaticXObj::m_setTextSize,		 1, 1,	400 },
	{ "setTextStyle",				PrintOMaticXObj::m_setTextStyle,		 1, 1,	400 },
	{ "setTextJust",				PrintOMaticXObj::m_setTextJust,		 1, 1,	400 },
	{ "setTextLineSpacing",				PrintOMaticXObj::m_setTextLineSpacing,		 1, 1,	500 },
	{ "textBox",				PrintOMaticXObj::m_textBox,		 5, 5,	400 },
	{ "setText",				PrintOMaticXObj::m_setText,		 0, 0,	400 },
	{ "appendText",				PrintOMaticXObj::m_appendText,		 0, 0,	400 },
	{ "appendTextFile",				PrintOMaticXObj::m_appendTextFile,		 0, 0,	400 },
	{ "append",				PrintOMaticXObj::m_appendText,		 0, 0,	500 },
	{ "appendFile",				PrintOMaticXObj::m_appendTextFile,		 0, 0,	500 },
	{ "appendTextResource",				PrintOMaticXObj::m_appendTextResource,		 0, 0,	400 },
	{ "newFrame",				PrintOMaticXObj::m_newFrame,		 2, 2,	500 },
	{ "drawRect",				PrintOMaticXObj::m_drawRect,		 2, 2,	500 },
	{ "drawLine",				PrintOMaticXObj::m_drawLine,		 2, 2,	500 },
	{ "drawRoundRect",				PrintOMaticXObj::m_drawRoundRect,		 3, 3,	500 },
	{ "drawOval",				PrintOMaticXObj::m_drawOval,		 2, 2,	500 },
	{ "drawText",				PrintOMaticXObj::m_drawText,		 3, 3,	400 },
	{ "drawPicture",				PrintOMaticXObj::m_drawPicture,		 2, 2,	500 },
	{ "drawStagePicture",				PrintOMaticXObj::m_drawStagePicture,		 2, 2,	500 },
	{ "getInsertionPoint",				PrintOMaticXObj::m_getInsertionPoint,		 0, 0,	400 },
	{ "masterTextBox",				PrintOMaticXObj::m_masterTextBox,		 4, 4,	400 },
	{ "appendMasterText",				PrintOMaticXObj::m_appendMasterText,		 0, 0,	400 },
	{ "appendMasterTextFiles",				PrintOMaticXObj::m_appendMasterTextFiles,		 0, 0,	400 },
	{ "masterPictBox",				PrintOMaticXObj::m_masterPictBox,		 4, 4,	400 },
	{ "appendMasterPict",				PrintOMaticXObj::m_appendMasterPict,		 0, 0,	400 },
	{ "stageToMasterPict",				PrintOMaticXObj::m_stageToMasterPict,		 0, 0,	400 },
	{ "strokedRect",				PrintOMaticXObj::m_strokedRect,		 4, 4,	400 },
	{ "filledRect",				PrintOMaticXObj::m_filledRect,		 4, 4,	400 },
	{ "strokedRoundRect",				PrintOMaticXObj::m_strokedRoundRect,		 5, 5,	400 },
	{ "filledRoundRect",				PrintOMaticXObj::m_filledRoundRect,		 5, 5,	400 },
	{ "strokedOval",				PrintOMaticXObj::m_strokedOval,		 0, 0,	400 },
	{ "filledOval",				PrintOMaticXObj::m_filledOval,		 0, 0,	400 },
	{ "line",				PrintOMaticXObj::m_line,		 4, 4,	400 },
	{ "picture",				PrintOMaticXObj::m_picture,		 0, 0,	400 },
	{ "stagePicture",				PrintOMaticXObj::m_stagePicture,		 0, 0,	400 },
	{ "1BitStagePicture",				PrintOMaticXObj::m_1BitStagePicture,		 0, 0,	400 },
	{ "ePSFile",				PrintOMaticXObj::m_ePSFile,		 0, 0,	400 },
	{ "setLandscapeMode",				PrintOMaticXObj::m_setLandscapeMode,		 1, 1,	400 },
	{ "setDocumentName",				PrintOMaticXObj::m_setDocumentName,		 1, 1,	400 },
	{ "doPageSetup",				PrintOMaticXObj::m_doPageSetup,		 0, 0,	400 },
	{ "doJobSetup",				PrintOMaticXObj::m_doJobSetup,		 0, 0,	400 },
	{ "setProgressMsg",				PrintOMaticXObj::m_setProgressMsg,		 1, 1,	400 },
	{ "setProgressPict",				PrintOMaticXObj::m_setProgressPict,		 0, 0,	400 },
	{ "setProgressLoc",				PrintOMaticXObj::m_setProgressLoc,		 2, 2,	400 },
	{ "printPreview",				PrintOMaticXObj::m_printPreview,		 0, 0,	400 },
	{ "printPicts",				PrintOMaticXObj::m_printPicts,		 0, 0,	400 },
	{ "print",				PrintOMaticXObj::m_print,		 0, 0,	400 },
	{ "savePageSetup",				PrintOMaticXObj::m_savePageSetup,		 4, 4,	400 },
	{ "getPageSetup",				PrintOMaticXObj::m_getPageSetup,		 2, 2,	400 },
	{ "hideMessages",				PrintOMaticXObj::m_hideMessages,		 1, 1,	400 },
	{ "setPageNumSymbol",				PrintOMaticXObj::m_setPageNumSymbol,		 1, 1,	400 },
	{ "register",				PrintOMaticXObj::m_register,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PrintOMaticXObject::PrintOMaticXObject(ObjectType ObjectType) :Object<PrintOMaticXObject>("PrintOMatic") {
	_objType = ObjectType;
}

void PrintOMaticXObj::open(ObjectType type, const Common::Path &path) {
    PrintOMaticXObject::initMethods(xlibMethods);
    PrintOMaticXObject *xobj = new PrintOMaticXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PrintOMaticXObj::close(ObjectType type) {
    PrintOMaticXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PrintOMaticXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PrintOMaticXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(PrintOMaticXObj::m_dispose)
XOBJSTUBNR(PrintOMaticXObj::m_reset)
XOBJSTUB(PrintOMaticXObj::m_newPage, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setPage)
XOBJSTUB(PrintOMaticXObj::m_setMargins, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setPrintableMargins)
XOBJSTUB(PrintOMaticXObj::m_getPageWidth, 0)
XOBJSTUB(PrintOMaticXObj::m_getPageHeight, 0)
XOBJSTUB(PrintOMaticXObj::m_getPaperWidth, 0)
XOBJSTUB(PrintOMaticXObj::m_getPaperHeight, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setColor)
XOBJSTUBNR(PrintOMaticXObj::m_setGray)
XOBJSTUBNR(PrintOMaticXObj::m_setPenSize)
XOBJSTUBNR(PrintOMaticXObj::m_setLineWeight)
XOBJSTUBNR(PrintOMaticXObj::m_setTextFont)
XOBJSTUBNR(PrintOMaticXObj::m_setTextSize)
XOBJSTUBNR(PrintOMaticXObj::m_setTextStyle)
XOBJSTUBNR(PrintOMaticXObj::m_setTextJust)
XOBJSTUBNR(PrintOMaticXObj::m_setTextLineSpacing)
XOBJSTUBNR(PrintOMaticXObj::m_textBox)
XOBJSTUB(PrintOMaticXObj::m_setText, 0)
XOBJSTUB(PrintOMaticXObj::m_appendText, 0)
XOBJSTUB(PrintOMaticXObj::m_appendTextFile, 0)
XOBJSTUB(PrintOMaticXObj::m_appendTextResource, 0)
XOBJSTUB(PrintOMaticXObj::m_newFrame, 0)
XOBJSTUBNR(PrintOMaticXObj::m_drawRect)
XOBJSTUBNR(PrintOMaticXObj::m_drawLine)
XOBJSTUBNR(PrintOMaticXObj::m_drawRoundRect)
XOBJSTUBNR(PrintOMaticXObj::m_drawOval)
XOBJSTUB(PrintOMaticXObj::m_drawText, 0)
XOBJSTUBNR(PrintOMaticXObj::m_drawPicture)
XOBJSTUBNR(PrintOMaticXObj::m_drawStagePicture)
XOBJSTUB(PrintOMaticXObj::m_getInsertionPoint, "page, 0, 0")
XOBJSTUBNR(PrintOMaticXObj::m_masterTextBox)
XOBJSTUB(PrintOMaticXObj::m_appendMasterText, 0)
XOBJSTUB(PrintOMaticXObj::m_appendMasterTextFiles, 0)
XOBJSTUBNR(PrintOMaticXObj::m_masterPictBox)
XOBJSTUB(PrintOMaticXObj::m_appendMasterPict, 0)
XOBJSTUBNR(PrintOMaticXObj::m_stageToMasterPict)
XOBJSTUBNR(PrintOMaticXObj::m_strokedRect)
XOBJSTUBNR(PrintOMaticXObj::m_filledRect)
XOBJSTUBNR(PrintOMaticXObj::m_strokedRoundRect)
XOBJSTUBNR(PrintOMaticXObj::m_filledRoundRect)
XOBJSTUB(PrintOMaticXObj::m_strokedOval, 0)
XOBJSTUB(PrintOMaticXObj::m_filledOval, 0)
XOBJSTUBNR(PrintOMaticXObj::m_line)
XOBJSTUB(PrintOMaticXObj::m_picture, 0)
XOBJSTUB(PrintOMaticXObj::m_stagePicture, 0)
XOBJSTUB(PrintOMaticXObj::m_1BitStagePicture, 0)
XOBJSTUB(PrintOMaticXObj::m_ePSFile, 0)
XOBJSTUB(PrintOMaticXObj::m_setLandscapeMode, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setDocumentName)
XOBJSTUB(PrintOMaticXObj::m_doPageSetup, 0)
XOBJSTUB(PrintOMaticXObj::m_doJobSetup, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setProgressMsg)
XOBJSTUB(PrintOMaticXObj::m_setProgressPict, 0)
XOBJSTUBNR(PrintOMaticXObj::m_setProgressLoc)
XOBJSTUB(PrintOMaticXObj::m_printPreview, 0)
XOBJSTUB(PrintOMaticXObj::m_printPicts, 0)
XOBJSTUBNR(PrintOMaticXObj::m_print)
XOBJSTUB(PrintOMaticXObj::m_savePageSetup, 0)
XOBJSTUB(PrintOMaticXObj::m_getPageSetup, 0)
XOBJSTUBNR(PrintOMaticXObj::m_hideMessages)
XOBJSTUB(PrintOMaticXObj::m_setPageNumSymbol, 0)

void PrintOMaticXObj::m_register(int nargs) {
	Common::String serialNumber = g_lingo->pop().asString();
	debugC(1, kDebugXObj, "PrintOMaticXObj::m_register: Registered with serial \"%s\"", serialNumber.c_str());
}

}
