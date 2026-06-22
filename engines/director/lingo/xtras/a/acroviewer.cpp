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
#include "director/lingo/xtras/a/acroviewer.h"

/**************************************************
 *
 * USED IN:
 * Aterotrombosis 2001: De La Genética A La Salud (Pfizer)
 * Lucas Learning Demo
 *
 **************************************************/

/*
-- xtra AcroViewer -- version 1.5 (c) 1997-99 XtraMedia International

-- First 15 Methods work with all Viewers:

--CLASS 1:

new     obj gObj
release obj gObj
getLastError obj gObj

--CLASS 2:

lookForAcrobat obj gObj

launchViewer * -- NOTE: first two args required; last three are optional:
-- syntax: obj gObj, str viewerFullPath, any optionalAppWindowRect, str optionalPDFpath, int optionalPrintFlag

bringViewerToFront    obj gObj
bringDirectorToFront  obj gObj
quitViewer            obj gObj

--CLASS 3:

openViewerDocument obj gObj, str fullPath
openDocInvisible   obj gObj, str fullPath
maximizeWindow     obj gObj, str docName, bool winSize

printOpenDocument  obj gObj
closeAllDocuments  obj gObj

getAppWindowRect   obj gObj
setAppWindowRect   obj gObj, rect windRect

-- 27 Methods for use with Acrobat Exchange Only:

--CLASS 4:

executeMenuItem    obj gObj, str menuItemWord

--CLASS 5:

getNumPages        obj gObj, str filePath

--CLASS 6:

getNumOpenDocs     obj gObj
bringDocForward    obj gObj, str docName
setViewMode        obj gObj, str docName, int mode

findText           obj gObj, str targetStr, str docName, bool caseSens, bool wholeWord, bool bWrap
clearSelection     obj gObj, str docName

printPages         obj gObj, str docName, int firstPage, int lastPage, int shrinkToFit
closeViewerDocument obj gObj, str docName

--CLASS 7:

gotoPage           obj gObj, int pageNum
gotoNext           obj gObj
gotoPrevious       obj gObj

gotoFirst          obj gObj
gotoLast           obj gObj

readPageUp         obj gObj
readPageDown       obj gObj
readPageLeft       obj gObj
readPageRight      obj gObj

scrollPage         obj gObj, int Hpixels, int Vpixels
zoomPage           obj gObj, int zoomFactor
goBackward         obj gObj
goForward          obj gObj

--CLASS 8:

drawIntoXWindow    obj gObj, str docName, int pageNum, rect windRect, int fitInWindow, int scaleFactor, int hOffset, int vOffset
hideXWindow        obj gObj
showXWindow        obj gObj
disposeXWindow     obj gObj

--WIN ONLY:

getStageAndParentParams obj gObj
setStageAndParentParams obj gObj, rect windRect, int modalFlag

--STATUS:

status  obj gObj, int index

"
 */

namespace Director {

const char *AcroviewerXtra::xlibName = "Acroviewer";
const XlibFileDesc AcroviewerXtra::fileNames[] = {
	{ "acroviewer",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AcroviewerXtra::m_new,		 0, 0,	500 },
	{ "release",				AcroviewerXtra::m_release,		 0, 0,	500 },
	{ "getLastError",				AcroviewerXtra::m_getLastError,		 0, 0,	500 },
	{ "lookForAcrobat",				AcroviewerXtra::m_lookForAcrobat,		 0, 0,	500 },
	{ "launchViewer",				AcroviewerXtra::m_launchViewer,		 -1, 0,	500 },
	{ "bringViewerToFront",				AcroviewerXtra::m_bringViewerToFront,		 0, 0,	500 },
	{ "bringDirectorToFront",				AcroviewerXtra::m_bringDirectorToFront,		 0, 0,	500 },
	{ "quitViewer",				AcroviewerXtra::m_quitViewer,		 0, 0,	500 },
	{ "openViewerDocument",				AcroviewerXtra::m_openViewerDocument,		 1, 1,	500 },
	{ "openDocInvisible",				AcroviewerXtra::m_openDocInvisible,		 1, 1,	500 },
	{ "maximizeWindow",				AcroviewerXtra::m_maximizeWindow,		 2, 2,	500 },
	{ "printOpenDocument",				AcroviewerXtra::m_printOpenDocument,		 0, 0,	500 },
	{ "closeAllDocuments",				AcroviewerXtra::m_closeAllDocuments,		 0, 0,	500 },
	{ "getAppWindowRect",				AcroviewerXtra::m_getAppWindowRect,		 0, 0,	500 },
	{ "setAppWindowRect",				AcroviewerXtra::m_setAppWindowRect,		 1, 1,	500 },
	{ "executeMenuItem",				AcroviewerXtra::m_executeMenuItem,		 1, 1,	500 },
	{ "getNumPages",				AcroviewerXtra::m_getNumPages,		 1, 1,	500 },
	{ "getNumOpenDocs",				AcroviewerXtra::m_getNumOpenDocs,		 0, 0,	500 },
	{ "bringDocForward",				AcroviewerXtra::m_bringDocForward,		 1, 1,	500 },
	{ "setViewMode",				AcroviewerXtra::m_setViewMode,		 2, 2,	500 },
	{ "findText",				AcroviewerXtra::m_findText,		 5, 5,	500 },
	{ "clearSelection",				AcroviewerXtra::m_clearSelection,		 1, 1,	500 },
	{ "printPages",				AcroviewerXtra::m_printPages,		 4, 4,	500 },
	{ "closeViewerDocument",				AcroviewerXtra::m_closeViewerDocument,		 1, 1,	500 },
	{ "gotoPage",				AcroviewerXtra::m_gotoPage,		 1, 1,	500 },
	{ "gotoNext",				AcroviewerXtra::m_gotoNext,		 0, 0,	500 },
	{ "gotoPrevious",				AcroviewerXtra::m_gotoPrevious,		 0, 0,	500 },
	{ "gotoFirst",				AcroviewerXtra::m_gotoFirst,		 0, 0,	500 },
	{ "gotoLast",				AcroviewerXtra::m_gotoLast,		 0, 0,	500 },
	{ "readPageUp",				AcroviewerXtra::m_readPageUp,		 0, 0,	500 },
	{ "readPageDown",				AcroviewerXtra::m_readPageDown,		 0, 0,	500 },
	{ "readPageLeft",				AcroviewerXtra::m_readPageLeft,		 0, 0,	500 },
	{ "readPageRight",				AcroviewerXtra::m_readPageRight,		 0, 0,	500 },
	{ "scrollPage",				AcroviewerXtra::m_scrollPage,		 2, 2,	500 },
	{ "zoomPage",				AcroviewerXtra::m_zoomPage,		 1, 1,	500 },
	{ "goBackward",				AcroviewerXtra::m_goBackward,		 0, 0,	500 },
	{ "goForward",				AcroviewerXtra::m_goForward,		 0, 0,	500 },
	{ "drawIntoXWindow",				AcroviewerXtra::m_drawIntoXWindow,		 7, 7,	500 },
	{ "hideXWindow",				AcroviewerXtra::m_hideXWindow,		 0, 0,	500 },
	{ "showXWindow",				AcroviewerXtra::m_showXWindow,		 0, 0,	500 },
	{ "disposeXWindow",				AcroviewerXtra::m_disposeXWindow,		 0, 0,	500 },
	{ "getStageAndParentParams",				AcroviewerXtra::m_getStageAndParentParams,		 0, 0,	500 },
	{ "setStageAndParentParams",				AcroviewerXtra::m_setStageAndParentParams,		 2, 2,	500 },
	{ "status",				AcroviewerXtra::m_status,		 1, 1,	500 },
	{ """,				AcroviewerXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AcroviewerXtraObject::AcroviewerXtraObject(ObjectType ObjectType) :Object<AcroviewerXtraObject>("Acroviewer") {
	_objType = ObjectType;
}

bool AcroviewerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AcroviewerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AcroviewerXtra::xlibName);
	warning("AcroviewerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AcroviewerXtra::open(ObjectType type, const Common::Path &path) {
    AcroviewerXtraObject::initMethods(xlibMethods);
    AcroviewerXtraObject *xobj = new AcroviewerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AcroviewerXtra::close(ObjectType type) {
    AcroviewerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AcroviewerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AcroviewerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AcroviewerXtra::m_release, 0)
XOBJSTUB(AcroviewerXtra::m_getLastError, 0)
XOBJSTUB(AcroviewerXtra::m_lookForAcrobat, 0)
XOBJSTUB(AcroviewerXtra::m_launchViewer, 0)
XOBJSTUB(AcroviewerXtra::m_bringViewerToFront, 0)
XOBJSTUB(AcroviewerXtra::m_bringDirectorToFront, 0)
XOBJSTUB(AcroviewerXtra::m_quitViewer, 0)
XOBJSTUB(AcroviewerXtra::m_openViewerDocument, 0)
XOBJSTUB(AcroviewerXtra::m_openDocInvisible, 0)
XOBJSTUB(AcroviewerXtra::m_maximizeWindow, 0)
XOBJSTUB(AcroviewerXtra::m_printOpenDocument, 0)
XOBJSTUB(AcroviewerXtra::m_closeAllDocuments, 0)
XOBJSTUB(AcroviewerXtra::m_getAppWindowRect, 0)
XOBJSTUB(AcroviewerXtra::m_setAppWindowRect, 0)
XOBJSTUB(AcroviewerXtra::m_executeMenuItem, 0)
XOBJSTUB(AcroviewerXtra::m_getNumPages, 0)
XOBJSTUB(AcroviewerXtra::m_getNumOpenDocs, 0)
XOBJSTUB(AcroviewerXtra::m_bringDocForward, 0)
XOBJSTUB(AcroviewerXtra::m_setViewMode, 0)
XOBJSTUB(AcroviewerXtra::m_findText, 0)
XOBJSTUB(AcroviewerXtra::m_clearSelection, 0)
XOBJSTUB(AcroviewerXtra::m_printPages, 0)
XOBJSTUB(AcroviewerXtra::m_closeViewerDocument, 0)
XOBJSTUB(AcroviewerXtra::m_gotoPage, 0)
XOBJSTUB(AcroviewerXtra::m_gotoNext, 0)
XOBJSTUB(AcroviewerXtra::m_gotoPrevious, 0)
XOBJSTUB(AcroviewerXtra::m_gotoFirst, 0)
XOBJSTUB(AcroviewerXtra::m_gotoLast, 0)
XOBJSTUB(AcroviewerXtra::m_readPageUp, 0)
XOBJSTUB(AcroviewerXtra::m_readPageDown, 0)
XOBJSTUB(AcroviewerXtra::m_readPageLeft, 0)
XOBJSTUB(AcroviewerXtra::m_readPageRight, 0)
XOBJSTUB(AcroviewerXtra::m_scrollPage, 0)
XOBJSTUB(AcroviewerXtra::m_zoomPage, 0)
XOBJSTUB(AcroviewerXtra::m_goBackward, 0)
XOBJSTUB(AcroviewerXtra::m_goForward, 0)
XOBJSTUB(AcroviewerXtra::m_drawIntoXWindow, 0)
XOBJSTUB(AcroviewerXtra::m_hideXWindow, 0)
XOBJSTUB(AcroviewerXtra::m_showXWindow, 0)
XOBJSTUB(AcroviewerXtra::m_disposeXWindow, 0)
XOBJSTUB(AcroviewerXtra::m_getStageAndParentParams, 0)
XOBJSTUB(AcroviewerXtra::m_setStageAndParentParams, 0)
XOBJSTUB(AcroviewerXtra::m_status, 0)
XOBJSTUB(AcroviewerXtra::m_", 0)

}
