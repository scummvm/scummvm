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
#include "director/lingo/xlibs/spacemgr.h"

namespace Director {

/**************************************************
 *
 * USED IN:
 * DEVO Presents: Adventures of the Smart Patrol
 *
 **************************************************/
/*
-- SpaceMgr XObject. Copyright 1995 Inscape v1.0b1 10March95 BDL
SpaceMgr
I      mNew                                                                --Creates a new instance
X      mDispose                                                            --Disposes XObject instance
I                  mLastError                                                                                      --Last error code
I                  mMemUsed                                                                                                --RAM occupied by this object and data
S                  mListData                                                                                               --List all data
I      mSortAll                                                            --Sort internal arrays
IS     mCheckForDups, bool                 --Set to true to check for Duplicate items before adding an element
IS     mParseText, text                            --Parse Text into a space structure
S                  mGetCurData                                                                                     --List Current data
ISSSS      mSetCurData, sc, s, n, v                                --pass in names of elements to make current
ISS                mAddSpaceCollection, name, data --Add a Space Collection
IS                 mRemoveSpaceCollection, name            --Remove a Space Collection
IS                 mSetCurSpaceCollection, name            --Set current SpaceCollection
S                  mGetCurSpaceCollection                                  --Get current SpaceCollection
SS                 mGetSpaceCollection, name                               --Get SpaceCollection with name
ISS                mAddSpace, name, data                                           --Add a Space
IS                 mRemoveSpace, name                                                      --Remove a Space
IS                 mSetCurSpace, name                                                      --Set current Space
S                  mGetCurSpace                                                                            --Get current Space
SS                 mGetSpace, name                                                                 --Get Space with name
ISS                mAddNode, name, data                                            --Add a Node
IS                 mRemoveNode, name                                                               --Remove a Node
IS                 mSetCurNode, name                                                               --Set current Node
S                  mGetCurNode                                                                                     --Get current Node
SS                 mGetNode, name                                                                  --Get Node with name
ISS                mAddView, name, data                                            --Add a View
IS                 mRemoveView, name                                                               --Remove a View
IS                 mSetCurView, name                                                               --Set current View
S                  mGetCurView                                                                                     --Get current View
SS                 mGetView, name                                                                  --Get View with name
ISS                mAddLocalLink, name, data                               --Add a link to the current view
IS                 mRemoveLocalLink, name                                  --Remove a link from the current View
I                  mRemoveLocalLinks                                                               --Remove all links from the current View
SS                 mGetLocalLink, name                                                     --Get link with name from the current view
S                  mGetLocalLinks                                                                  --Get all links from the current view
ISS                mAddGlobalLink, name, data                      --Add a global link
IS                 mRemoveGlobalLink, name                                 --Remove a global link
SS                 mGetGlobalLink, name                                            --Get global link with name
S                  mGetGlobalLinks                                                                 --Get list of all global links
*/

const char *SpaceMgr::xlibName = "SpaceMgr";
const char *SpaceMgr::fileNames[] = {
	"SpaceMgr",
	"SPACEMGR",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",					SpaceMgr::m_new,					0, 0,	400 },	// D4
	{ "dispose",				SpaceMgr::m_dispose,				1, 1,	400 },	// D4
	{ "lastError",				SpaceMgr::m_lastError,				0, 0,	400 },	// D4
	{ "memUsed",				SpaceMgr::m_memUsed,				0, 0,	400 },	// D4
	{ "listData",				SpaceMgr::m_listData,				0, 0,	400 },	// D4
	{ "sortAll",				SpaceMgr::m_sortAll,				0, 0,	400 },	// D4
	{ "checkForDups",			SpaceMgr::m_checkForDups,			1, 1,	400 },	// D4
	{ "parseText",				SpaceMgr::m_parseText,				1, 1,	400 },	// D4
	{ "getCurData",				SpaceMgr::m_getCurData,				0, 0,	400 },	// D4
	{ "setCurData",				SpaceMgr::m_setCurData,				4, 4,	400 },	// D4
	{ "addSpaceCollection",		SpaceMgr::m_addSpaceCollection,		2, 2,	400 },	// D4
	{ "removeSpaceCollection",	SpaceMgr::m_removeSpaceCollection,	1, 1,	400 },	// D4
	{ "setCurSpaceCollection",	SpaceMgr::m_setCurSpaceCollection,	1, 1,	400 },	// D4
	{ "getCurSpaceCollection",	SpaceMgr::m_getCurSpaceCollection,	0, 0,	400 },	// D4
	{ "getSpaceCollection",		SpaceMgr::m_getSpaceCollection,		1, 1,	400 },	// D4
	{ "addSpace",				SpaceMgr::m_addSpace,				2, 2,	400 },	// D4
	{ "removeSpace",			SpaceMgr::m_removeSpace,			1, 1,	400 },	// D4
	{ "setCurSpace",			SpaceMgr::m_setCurSpace,			1, 1,	400 },	// D4
	{ "getCurSpace",			SpaceMgr::m_getCurSpace,			0, 0,	400 },	// D4
	{ "getSpace",				SpaceMgr::m_getSpace,				1, 1,	400 },	// D4
	{ "addNode",				SpaceMgr::m_addNode,				2, 2,	400 },	// D4
	{ "removeNode",				SpaceMgr::m_removeNode,				1, 1,	400 },	// D4
	{ "setCurNode",				SpaceMgr::m_setCurNode,				1, 1,	400 },	// D4
	{ "getCurNode",				SpaceMgr::m_getCurNode,				0, 0,	400 },	// D4
	{ "getNode",				SpaceMgr::m_getNode,				1, 1,	400 },	// D4
	{ "addView",				SpaceMgr::m_addView,				2, 2,	400 },	// D4
	{ "removeView",				SpaceMgr::m_removeView,				1, 1,	400 },	// D4
	{ "setCurView",				SpaceMgr::m_setCurView,				1, 1,	400 },	// D4
	{ "getCurView",				SpaceMgr::m_getCurView,				0, 0,	400 },	// D4
	{ "getView",				SpaceMgr::m_getView,				1, 1,	400 },	// D4
	{ "addLocalLink",			SpaceMgr::m_addLocalLink,			2, 2,	400 },	// D4
	{ "removeLocalLink",		SpaceMgr::m_removeLocalLink,		1, 1,	400 },	// D4
	{ "removeLocalLinks",		SpaceMgr::m_removeLocalLinks,		0, 0,	400 },	// D4
	{ "getLocalLink",			SpaceMgr::m_getLocalLink,			1, 1,	400 },	// D4
	{ "getLocalLinks",			SpaceMgr::m_getLocalLink,			1, 1,	400 },	// D4
	{ "addGlobalLink",			SpaceMgr::m_addGlobalLink,			2, 2,	400 },	// D4
	{ "removeGlobalLink",		SpaceMgr::m_removeGlobalLink,		1, 1,	400 },	// D4
	{ "getGlobalLink",			SpaceMgr::m_getGlobalLink,			1, 1,	400 },	// D4
	{ "getGlobalLinks",			SpaceMgr::m_getGlobalLinks,			0, 0,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

SpaceMgrXObject::SpaceMgrXObject(ObjectType ObjectType) :Object<SpaceMgrXObject>("SpaceMgr") {
	_objType = ObjectType;
}

void SpaceMgr::open(int type) {
	if (type == kXObj) {
		SpaceMgrXObject::initMethods(xlibMethods);
		SpaceMgrXObject *xobj = new SpaceMgrXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void SpaceMgr::close(int type) {
	if (type == kXObj) {
		SpaceMgrXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	} else if (type == kXtraObj) {
		// TODO - Implement Xtra
	}
}

void SpaceMgr::m_new(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_new: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(g_lingo->_currentMe);
}

void SpaceMgr::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void SpaceMgr::m_lastError(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_lastError", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_memUsed(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_memUsed", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_listData(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_listData", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_sortAll(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_sortAll", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_checkForDups(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_checkForDups", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_parseText(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_parseText", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurData(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getCurData", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_setCurData(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_setCurData", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_addSpaceCollection(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addSpaceCollection", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeSpaceCollection(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeSpaceCollection", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_setCurSpaceCollection(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_setCurSpaceCollection", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurSpaceCollection(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getCurSpaceCollection", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getSpaceCollection(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getSpaceCollection", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_addSpace(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addSpace", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeSpace(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeSpace", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_setCurSpace(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_setCurSpace", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurSpace(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getCurSpace", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getSpace(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getSpace", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_addNode(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addNode", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeNode(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeNode", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_setCurNode(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_setCurNode", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurNode(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getCurNode", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getNode(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getNode", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_addView(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addView", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeView(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeView", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_setCurView(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_setCurView", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurView(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getCurView", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getView(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getView", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_addLocalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addLocalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeLocalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeLocalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_removeLocalLinks(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeLocalLinks", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getLocalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getLocalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getLocalLinks(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getLocalLinks", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_addGlobalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_addGlobalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_removeGlobalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_removeGlobalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getGlobalLink(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getGlobalLink", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void SpaceMgr::m_getGlobalLinks(int nargs) {
	g_lingo->printSTUBWithArglist("SpaceMgr::m_getGlobalLinks", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

}
