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

/**************************************************
 *
 * USED IN:
 * DEVO Presents: Adventures of the Smart Patrol
 * The Dark Eye
 *
 **************************************************/

/*
 * -- SpaceMgr XObject. Copyright 1995 Inscape v1.0b1 10March95 BDL
 * SpaceMgr
 * I      mNew                                                                --Creates a new instance
 * X      mDispose                                                            --Disposes XObject instance
 * I                  mLastError                                                                                      --Last error code
 * I                  mMemUsed                                                                                                --RAM occupied by this object and data
 * S                  mListData                                                                                               --List all data
 * I      mSortAll                                                            --Sort internal arrays
 * IS     mCheckForDups, bool                 --Set to true to check for Duplicate items before adding an element
 * IS     mParseText, text                            --Parse Text into a space structure
 * S                  mGetCurData                                                                                     --List Current data
 * ISSSS      mSetCurData, sc, s, n, v                                --pass in names of elements to make current
 * ISS                mAddSpaceCollection, name, data --Add a Space Collection
 * IS                 mRemoveSpaceCollection, name            --Remove a Space Collection
 * IS                 mSetCurSpaceCollection, name            --Set current SpaceCollection
 * S                  mGetCurSpaceCollection                                  --Get current SpaceCollection
 * SS                 mGetSpaceCollection, name                               --Get SpaceCollection with name
 * ISS                mAddSpace, name, data                                           --Add a Space
 * IS                 mRemoveSpace, name                                                      --Remove a Space
 * IS                 mSetCurSpace, name                                                      --Set current Space
 * S                  mGetCurSpace                                                                            --Get current Space
 * SS                 mGetSpace, name                                                                 --Get Space with name
 * ISS                mAddNode, name, data                                            --Add a Node
 * IS                 mRemoveNode, name                                                               --Remove a Node
 * IS                 mSetCurNode, name                                                               --Set current Node
 * S                  mGetCurNode                                                                                     --Get current Node
 * SS                 mGetNode, name                                                                  --Get Node with name
 * ISS                mAddView, name, data                                            --Add a View
 * IS                 mRemoveView, name                                                               --Remove a View
 * IS                 mSetCurView, name                                                               --Set current View
 * S                  mGetCurView                                                                                     --Get current View
 * SS                 mGetView, name                                                                  --Get View with name
 * ISS                mAddLocalLink, name, data                               --Add a link to the current view
 * IS                 mRemoveLocalLink, name                                  --Remove a link from the current View
 * I                  mRemoveLocalLinks                                                               --Remove all links from the current View
 * SS                 mGetLocalLink, name                                                     --Get link with name from the current view
 * S                  mGetLocalLinks                                                                  --Get all links from the current view
 * ISS                mAddGlobalLink, name, data                      --Add a global link
 * IS                 mRemoveGlobalLink, name                                 --Remove a global link
 * SS                 mGetGlobalLink, name                                            --Get global link with name
 * S                  mGetGlobalLinks                                                                 --Get list of all global links
 */

#include "common/system.h"
#include "common/tokenizer.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/spacemgr.h"
#include "director/util.h"

namespace Director {

const char *const SpaceMgr::xlibName = "SpaceMgr";
const XlibFileDesc SpaceMgr::fileNames[] = {
	{ "SpaceMgr",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
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

void SpaceMgr::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		SpaceMgrXObject::initMethods(xlibMethods);
		SpaceMgrXObject *xobj = new SpaceMgrXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void SpaceMgr::close(ObjectType type) {
	if (type == kXObj) {
		SpaceMgrXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void SpaceMgr::m_new(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_new: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(SpaceMgr::m_dispose)
XOBJSTUB(SpaceMgr::m_lastError, 0)
XOBJSTUB(SpaceMgr::m_memUsed, 0)
XOBJSTUB(SpaceMgr::m_listData, "")
XOBJSTUB(SpaceMgr::m_sortAll, 0)

void SpaceMgr::m_checkForDups(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_checkForDups: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	Datum arg = g_lingo->pop();
	if (arg.type != STRING) {
		warning("SpaceMgr::m_checkForDups: expected arg to be a string, not %s", arg.type2str());
		g_lingo->push(Datum(0));
		return;
	}

	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);

	me->_checkForDups = (arg.u.s->c_str()[0] == 't') || (arg.u.s->c_str()[0] == 'T');
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_parseText(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_parseText: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	Datum text = g_lingo->pop();
	if (text.type != STRING) {
		warning("SpaceMgr::m_parseText: expected text to be a string, not %s", text.type2str());
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);

	Common::String result = *text.u.s;
	if (debugLevelSet(5)) {
		debugC(5, kDebugXObj, "SpaceMgr::m_parseText:\n%s", formatStringForDump(result).c_str());
	}

	Common::StringTokenizer instructions = Common::StringTokenizer(result, "\r");

	while (!instructions.empty()) {
		Common::String instructionBody = instructions.nextToken();
		Common::StringTokenizer instruction = Common::StringTokenizer(instructionBody, " ");
		Common::String type = instruction.nextToken();
		if (type == "SPACECOLLECTION") {
			me->_curSpaceCollection = instruction.nextToken();
			me->_curSpace = "";
			me->_curNode = "";
			me->_curView = "";
			if (!(me->_spaceCollections.contains(me->_curSpaceCollection) && me->_checkForDups)) {
				me->_spaceCollections[me->_curSpaceCollection] = SpaceCollection();
			}
		} else if (type == "SPACE") {
			me->_curSpace = instruction.nextToken();
			me->_curNode = "";
			me->_curView = "";
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			if (!(sc.spaces.contains(me->_curSpaceCollection) && me->_checkForDups)) {
				sc.spaces[me->_curSpace] = Space();
			}
		} else if (type == "NODE") {
			me->_curNode = instruction.nextToken();
			me->_curView = "";
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			Space &s = sc.spaces.getVal(me->_curSpace);
			if (!(s.nodes.contains(me->_curNode) && me->_checkForDups)) {
				s.nodes[me->_curNode] = Node();
			}
		} else if (type == "VIEW") {
			me->_curView = instruction.nextToken();
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			Space &s = sc.spaces.getVal(me->_curSpace);
			Node &n = s.nodes.getVal(me->_curNode);
			if (!(n.views.contains(me->_curView) && me->_checkForDups)) {
				n.views[me->_curView] = View();
				n.views[me->_curView].payload = instruction.nextToken();
			}
		} else if (type == "LLINK") {
			Common::String target = instruction.nextToken();
			Common::String payload = instruction.nextToken();
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			Space &s = sc.spaces.getVal(me->_curSpace);
			Node &n = s.nodes.getVal(me->_curNode);
			View &v = n.views.getVal(me->_curView);
			if (!(v.llinks.contains(target) && me->_checkForDups)) {
				v.llinks[target] = LLink();
				v.llinks[target].payload = payload;
			}
		} else {
			warning("SpaceMgr::m_parseText: Unhandled instruction %s", instructionBody.c_str());
		}
	}

	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurData(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_getCurData: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String result;
	if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
		result += "SPACECOLLECTION " + me->_curSpaceCollection;
		SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
		if (sc.spaces.contains(me->_curSpace)) {
			result += "\r";
			result += "SPACE " + me->_curSpace;
			Space &s = sc.spaces.getVal(me->_curSpace);
			if (s.nodes.contains(me->_curNode)) {
				result += "\r";
				result += "NODE " + me->_curNode;
				Node &n = s.nodes.getVal(me->_curNode);
				if (n.views.contains(me->_curView)) {
					result += "\r";
					result += "VIEW " + me->_curView;
				}
			}
		}
	}

	if (debugLevelSet(5)) {
		debugC(5, kDebugXObj, "SpaceMgr::m_getCurData: %s", formatStringForDump(result).c_str());
	}

	g_lingo->push(Datum(result));
}

void SpaceMgr::m_setCurData(int nargs) {
	if (nargs != 4) {
		warning("SpaceMgr::m_setCurData: expected 4 arguments");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Datum view = g_lingo->pop();
	Datum node = g_lingo->pop();
	Datum space = g_lingo->pop();
	Datum spaceCollection = g_lingo->pop();
	if (spaceCollection.type != STRING ||
		space.type != STRING ||
		node.type != STRING ||
		view.type != STRING) {
		warning("SpaceMgr::m_setCurData: expected args to be a string");
		g_lingo->push(Datum(0));
		return;
	}
	if (!spaceCollection.u.s->empty())
		me->_curSpaceCollection = *spaceCollection.u.s;
	if (!space.u.s->empty())
		me->_curSpace = *space.u.s;
	if (!node.u.s->empty())
		me->_curNode = *node.u.s;
	if (!view.u.s->empty())
		me->_curView = *view.u.s;
	debugC(5, kDebugXObj, "SpaceMgr::m_setCurData: %s %s %s %s", me->_curSpaceCollection.c_str(), me->_curSpace.c_str(), me->_curNode.c_str(), me->_curView.c_str());
	g_lingo->push(Datum(0));
}

XOBJSTUB(SpaceMgr::m_addSpaceCollection, 0)

void SpaceMgr::m_removeSpaceCollection(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_removeSpaceCollection: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	Common::String sc = g_lingo->pop().asString();
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	if (me->_spaceCollections.contains(sc)) {
		me->_spaceCollections.erase(sc);
	}
	debugC(5, kDebugXObj, "SpaceMgr::m_removeSpaceCollection: %s", sc.c_str());

	g_lingo->push(Datum(0));
}

void SpaceMgr::m_setCurSpaceCollection(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_setCurSpaceCollection: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Datum spaceCollection = g_lingo->pop();
	if (spaceCollection.type != STRING) {
		warning("SpaceMgr::m_setCurSpaceCollection: expected spaceCollection to be a string, not %s", spaceCollection.type2str());
		g_lingo->push(Datum(0));
		return;

	}
	if (!spaceCollection.u.s->empty())
		me->_curSpaceCollection = *spaceCollection.u.s;
	debugC(5, kDebugXObj, "SpaceMgr::m_setCurSpaceCollection: %s", spaceCollection.u.s->c_str());
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurSpaceCollection(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_getCurSpaceCollection: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String result;
	if (!me->_curSpaceCollection.empty()) {
		if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
			result = "SPACECOLLECTION " + me->_curSpaceCollection;
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getCurSpaceCollection: %s", result.c_str());
	g_lingo->push(Datum(result));
}

void SpaceMgr::m_getSpaceCollection(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_getSpaceCollection: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String sc = g_lingo->pop().asString();
	Common::String result;
	if (me->_spaceCollections.contains(sc)) {
		result = "SPACECOLLECTION " + me->_curSpaceCollection;
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getSpaceCollection: %s", result.c_str());
	g_lingo->push(Datum(result));
}

XOBJSTUB(SpaceMgr::m_addSpace, 0)
XOBJSTUB(SpaceMgr::m_removeSpace, 0)

void SpaceMgr::m_setCurSpace(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_setCurSpace: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Datum space = g_lingo->pop();
	if (space.type != STRING) {
		warning("SpaceMgr::m_setCurSpace: expected space to be a string, not %s", space.type2str());
		g_lingo->push(Datum(0));
		return;

	}
	if (!space.u.s->empty())
		me->_curSpace = *space.u.s;
	debugC(5, kDebugXObj, "SpaceMgr::m_setCurSpace: %s", space.u.s->c_str());
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurSpace(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_getCurSpace: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String result;
	if (!me->_curSpace.empty()) {
		if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			if (sc.spaces.contains(me->_curSpace)) {
				result = "SPACE " + me->_curSpace;
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getCurSpace: %s", result.c_str());
	g_lingo->push(Datum(result));
}

void SpaceMgr::m_getSpace(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_getSpace: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String sp = g_lingo->pop().asString();
	Common::String result;
	if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
		SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
		if (sc.spaces.contains(sp)) {
			result = "SPACE " + sp;
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getSpace: %s", result.c_str());
	g_lingo->push(Datum(result));
}

XOBJSTUB(SpaceMgr::m_addNode, 0)
XOBJSTUB(SpaceMgr::m_removeNode, 0)

void SpaceMgr::m_setCurNode(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_setCurNode: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Datum node = g_lingo->pop();
	if (node.type != STRING) {
		warning("SpaceMgr::m_setCurNode: expected node to be a string, not %s", node.type2str());
		g_lingo->push(Datum(0));
		return;

	}
	if (!node.u.s->empty())
		me->_curNode = *node.u.s;
	debugC(5, kDebugXObj, "SpaceMgr::m_setCurNode: %s", node.u.s->c_str());
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurNode(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_getCurNode: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String result;
	if (!me->_curNode.empty()) {
		if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			if (sc.spaces.contains(me->_curSpace)) {
				Space &s = sc.spaces.getVal(me->_curSpace);
				if (s.nodes.contains(me->_curNode)) {
					result = "NODE " + me->_curNode;
				}
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getCurNode: %s", result.c_str());
	g_lingo->push(Datum(result));
}

void SpaceMgr::m_getNode(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_getNode: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String no = g_lingo->pop().asString();
	Common::String result;
	if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
		SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
		if (sc.spaces.contains(me->_curSpace)) {
			Space &s = sc.spaces.getVal(me->_curSpace);
			if (s.nodes.contains(no)) {
				result = "NODE " + no;
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getNode: %s", result.c_str());
	g_lingo->push(Datum(result));
}

XOBJSTUB(SpaceMgr::m_addView, 0)
XOBJSTUB(SpaceMgr::m_removeView, 0)

void SpaceMgr::m_setCurView(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_setCurView: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(0));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Datum view = g_lingo->pop();
	if (view.type != STRING) {
		warning("SpaceMgr::m_setCurView: expected view to be a string, not %s", view.type2str());
		g_lingo->push(Datum(0));
		return;

	}
	if (!view.u.s->empty())
		me->_curView = *view.u.s;
	debugC(5, kDebugXObj, "SpaceMgr::m_setCurView: %s", view.u.s->c_str());
	g_lingo->push(Datum(0));
}

void SpaceMgr::m_getCurView(int nargs) {
	if (nargs != 0) {
		warning("SpaceMgr::m_getCurView: expected 0 arguments");
		g_lingo->dropStack(nargs);
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String result;
	if (!me->_curView.empty()) {
		if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
			SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
			if (sc.spaces.contains(me->_curSpace)) {
				Space &s = sc.spaces.getVal(me->_curSpace);
				if (s.nodes.contains(me->_curNode)) {
					Node &n = s.nodes.getVal(me->_curNode);
					if (n.views.contains(me->_curView)) {
						result = "VIEW " + me->_curView + " " + n.views[me->_curView].payload;
					}
				}
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getCurView: %s", result.c_str());
	g_lingo->push(Datum(result));
}

void SpaceMgr::m_getView(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_getView: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String vi = g_lingo->pop().asString();
	Common::String result;
	if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
		SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
		if (sc.spaces.contains(me->_curSpace)) {
			Space &s = sc.spaces.getVal(me->_curSpace);
			if (s.nodes.contains(me->_curNode)) {
				Node &n = s.nodes.getVal(me->_curNode);
				if (n.views.contains(vi)) {
					result = "VIEW " + vi + " " + n.views[vi].payload;
				}
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getView: %s", result.c_str());
	g_lingo->push(Datum(result));
}

XOBJSTUB(SpaceMgr::m_addLocalLink, 0)
XOBJSTUB(SpaceMgr::m_removeLocalLink, 0)
XOBJSTUB(SpaceMgr::m_removeLocalLinks, 0)

void SpaceMgr::m_getLocalLink(int nargs) {
	if (nargs != 1) {
		warning("SpaceMgr::m_getLocalLink: expected 1 argument");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum(""));
		return;
	}
	SpaceMgrXObject *me = static_cast<SpaceMgrXObject *>(g_lingo->_state->me.u.obj);
	Common::String ll = g_lingo->pop().asString();
	Common::String result;
	if (me->_spaceCollections.contains(me->_curSpaceCollection)) {
		SpaceCollection &sc = me->_spaceCollections.getVal(me->_curSpaceCollection);
		if (sc.spaces.contains(me->_curSpace)) {
			Space &s = sc.spaces.getVal(me->_curSpace);
			if (s.nodes.contains(me->_curNode)) {
				Node &n = s.nodes.getVal(me->_curNode);
				if (n.views.contains(me->_curView)) {
					View &v = n.views.getVal(me->_curView);
					if (v.llinks.contains(ll)) {
						result = "LLINK " + ll + " " + v.llinks[ll].payload;
					}
				}
			}
		}
	}

	debugC(5, kDebugXObj, "SpaceMgr::m_getLocalLink: %s", result.c_str());
	g_lingo->push(Datum(result));
}

XOBJSTUB(SpaceMgr::m_getLocalLinks, "")
XOBJSTUB(SpaceMgr::m_addGlobalLink, "")
XOBJSTUB(SpaceMgr::m_removeGlobalLink, 0)
XOBJSTUB(SpaceMgr::m_getGlobalLink, "")
XOBJSTUB(SpaceMgr::m_getGlobalLinks, "")

} // End of namespace Director
