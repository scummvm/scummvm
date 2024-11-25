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
#include "director/lingo/xlibs/mapnavigatorxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MapNav External Factory. 16/03/95 - AAF - New
MapNav
IS mNew, fileName -- read map given file name
X mDispose --  dispose of map
I mGetFirstNode -- return index of first node
X mResetHidden --  reset hidden flags to initial values
SI mGetNodeName, node --  given index, return node name
IS mGetNodeIndex, name -- given name, return node index
II mGetBackgroundPicture, node --  return picture number for node
II mGetHotSpotCount, node --   return no. of hot spots for node
XIII mSetHidden, node, hotspot, flag -- set/clear hidden flag
III mGetHidden, node, hotspot --  set/clear hidden flag
IIIII mPointInside, node, hotspot, x, y -- return true if x,y inside hot spot
SII mGetHotSpotRect, node, hotspot -- return list of l,t,r,b for hot spot
III mGetHotSpotCursor, node, hotspot -- return cursor number for node
SII mGetEvaluationFcn, node, hotspot -- return function name to call to get condition number
IIII mGetDestinationNode, node, hotspot, condition --  return destination node for hot spot
IIII mGetInstructionCount, node, hotspot, condition --  return number of instructions
IIIII mGetInstructionType, node, hotspot, condition, index -- return type of instruction 'Q' or 'L'
SIIII mGetInstruction, node, hotspot, condition, index --  return ith instruction

-- Navigator, Navigation XObject - aep 95.03.28
IS mNew, fileName -- read map given file name
X mDispose --  dispose of map
I mGetFirstNode -- return index of first node
X mResetHidden --  reset hidden flags to initial values
SI mGetNodeName, node --  given index, return node name
IS mGetNodeIndex, name -- given name, return node index
II mGetBackgroundPicture, node --  return picture number for node
II mGetHotSpotCount, node --   return no. of hot spots for node
XIII mSetHidden, node, hotspot, flag -- set/clear hidden flag
III mGetHidden, node, hotspot --  set/clear hidden flag
IIIII mPointInside, node, hotspot, h, v -- return true if h,v inside hot spot
SII mGetHotSpotRect, node, hotspot -- return list of l,t,r,b for hot spot
III mGetHotSpotCursor, node, hotspot -- return cursor number for node
SII mGetEvaluationFcn, node, hotspot -- return function name to call to get condition no.
IIII mGetDestinationNode, node, hotspot, condition --  return destination node for hot spot
IIII mGetInstructionCount, node, hotspot, condition --  return number of instructions
IIIII mGetInstructionType, node, hotspot, condition, i -- return type of instruction 'Q' or 'L'
SIIII mGetInstruction, node, hotspot, condition, i --  return ith instruction
 */

namespace Director {

const char *const MapNavigatorXObj::xlibName = "MapNav";
const XlibFileDesc MapNavigatorXObj::fileNames[] = {
	{ "MAPNAV",				nullptr }, // Jewels of the Oracle - Win
	{ "MapNavigator.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,				nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					MapNavigatorXObj::m_new,		 		 1, 1,	400 },
	{ "dispose",				MapNavigatorXObj::m_dispose,		 	 0, 0,	400 },
	{ "getFirstNode",			MapNavigatorXObj::m_getFirstNode,		 0, 0,	400 },
	{ "resetHidden",			MapNavigatorXObj::m_resetHidden,		 0, 0,	400 },
	{ "getNodeName",			MapNavigatorXObj::m_getNodeName,		 1, 1,	400 },
	{ "getNodeIndex",			MapNavigatorXObj::m_getNodeIndex,		 1, 1,	400 },
	{ "getBackgroundPicture",	MapNavigatorXObj::m_getBackgroundPicture,1, 1,	400 },
	{ "getHotSpotCount",		MapNavigatorXObj::m_getHotSpotCount,	 1, 1,	400 },
	{ "setHidden",				MapNavigatorXObj::m_setHidden,			 3, 3,	400 },
	{ "getHidden",				MapNavigatorXObj::m_getHidden,			 2, 2,	400 },
	{ "pointInside",			MapNavigatorXObj::m_pointInside,		 4, 4,	400 },
	{ "getHotSpotRect",			MapNavigatorXObj::m_getHotSpotRect,		 2, 2,	400 },
	{ "getHotSpotCursor",		MapNavigatorXObj::m_getHotSpotCursor,	 2, 2,	400 },
	{ "getEvaluationFcn",		MapNavigatorXObj::m_getEvaluationFcn,	 2, 2,	400 },
	{ "getDestinationNode",		MapNavigatorXObj::m_getDestinationNode,	 3, 3,	400 },
	{ "getInstructionCount",	MapNavigatorXObj::m_getInstructionCount, 3, 3,	400 },
	{ "getInstructionType",		MapNavigatorXObj::m_getInstructionType,	 4, 4,	400 },
	{ "getInstruction",			MapNavigatorXObj::m_getInstruction,		 4, 4,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MapNavigatorXObject::MapNavigatorXObject(ObjectType ObjectType) :Object<MapNavigatorXObject>("MapNav") {
	_objType = ObjectType;
}

void MapNavigatorXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MapNavigatorXObject::initMethods(xlibMethods);
		MapNavigatorXObject *xobj = new MapNavigatorXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MapNavigatorXObj::close(ObjectType type) {
	if (type == kXObj) {
		MapNavigatorXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MapNavigatorXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MapNavigatorXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MapNavigatorXObj::m_dispose)
XOBJSTUB(MapNavigatorXObj::m_getFirstNode, 0)
XOBJSTUBNR(MapNavigatorXObj::m_resetHidden)
XOBJSTUB(MapNavigatorXObj::m_getNodeName, "")
XOBJSTUB(MapNavigatorXObj::m_getNodeIndex, 0)
XOBJSTUB(MapNavigatorXObj::m_getBackgroundPicture, 0)
XOBJSTUB(MapNavigatorXObj::m_getHotSpotCount, 0)
XOBJSTUBNR(MapNavigatorXObj::m_setHidden)
XOBJSTUB(MapNavigatorXObj::m_getHidden, 0)
XOBJSTUB(MapNavigatorXObj::m_pointInside, 0)
XOBJSTUB(MapNavigatorXObj::m_getHotSpotRect, "")
XOBJSTUB(MapNavigatorXObj::m_getHotSpotCursor, 0)
XOBJSTUB(MapNavigatorXObj::m_getEvaluationFcn, "")
XOBJSTUB(MapNavigatorXObj::m_getDestinationNode, 0)
XOBJSTUB(MapNavigatorXObj::m_getInstructionCount, 0)
XOBJSTUB(MapNavigatorXObj::m_getInstructionType, 0)
XOBJSTUB(MapNavigatorXObj::m_getInstruction, "")

}
