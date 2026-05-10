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
#include "common/file.h"
#include "common/memstream.h"

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/m/mapnavigatorxobj.h"

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

void mapnav_initialize_hidden_flags(MapNavigatorXObject *me) {
	for (int i = 0; i < me->_nodeCount; i++) {
		for (int j = 0; j < me->_nodes[i].hotspot_count; j++) {
			 me->_nodes[i].hotspots[j].isHidden = me->_nodes[i].hotspots[j].initially_hidden;
		}
	}
}


void MapNavigatorXObj::m_new(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	me->_filename = g_lingo->pop().asString();
	Common::File file;

	if (!file.open(Common::Path(me->_filename))) {
		warning("MapNavigatorXObj::m_new(): Cannot open file %s", me->_filename.c_str());
		g_lingo->push(g_lingo->_state->me);
		return;
	}

	byte *data = (byte *)calloc(file.size(), 1);
	file.read(data, file.size());

	Common::MemoryReadStream in(data, file.size(), DisposeAfterUse::YES);


	me->_nodeCount = in.readUint16BE();
	me->_hotspotCount = in.readUint16BE();
	me->_firstNodeIndex = in.readUint16BE();

	debug(1, "nodes: %d  hotspots: %d, firstNodeIdx: %d", me->_nodeCount, me->_hotspotCount, me->_firstNodeIndex);

	for (int i = 0; i < me->_nodeCount; i++) {
		NavNode n;
		n.background_picture = in.readUint16BE();
		n.hotspot_count = in.readUint16BE();
		n.unknown_04 = in.readUint16BE();
		n.hotspot_list_offset = in.readUint16BE();
		n.name = in.readPascalString();

		if (in.pos() % 2) // align to a word
			(void)in.readByte();

		me->_nodes.push_back(n);
	}

	for (int i = 0; i < me->_nodeCount; i++) {
		uint32 pos = me->_nodes[i].hotspot_list_offset;

		debug(1, "%d: pict: %04x hotspots: %04x unk04: %04x listoff: %04x name: %s",
			i, me->_nodes[i].background_picture, me->_nodes[i].hotspot_count, me->_nodes[i].unknown_04,
			me->_nodes[i].hotspot_list_offset, me->_nodes[i].name.c_str());

		for (int j = 0; j < me->_nodes[i].hotspot_count; j++) {
			NavHotSpot h;

			in.seek(pos);

			h.record_size = in.readUint16BE();
			h.left = in.readUint16BE();
			h.top = in.readUint16BE();
			h.right = in.readUint16BE();
			h.bottom = in.readUint16BE();
			h.cursor_id = in.readUint16BE();
			h.initially_hidden = in.readByte();
			h.unknown_0d = in.readByte();
			h.condition_count = in.readUint16BE();
			h.evaluation_name = in.readPascalString();

			me->_nodes[i].hotspots.push_back(h);

			debug(1, "  %d: size: %04x [%d, %d, %d, %d], cursorId: %d hidden: %d unk: %d condCnt: %d evalName: %s",
				j, h.record_size, h.left, h.top, h.right, h.bottom, h.cursor_id, h.initially_hidden,
				h.unknown_0d, h.condition_count, h.evaluation_name.c_str());

			// Reading conditions
			if (in.pos() % 2) // align to a word
				(void)in.readByte();

			int pos1 = in.pos();

			for (int k = 0; k < h.condition_count; k++) {
				NavCondition c;

				in.seek(pos1);

				c.record_size = in.readUint16BE();
				c.destination_node = in.readUint16BE();
				c.condition_id = in.readUint16BE();
				c.instruction_count = in.readUint16BE();

				debug(1, "    %d: size: %d destnode: %d condId: %d instCnt: %d", k, c.record_size, c.destination_node,
						c.condition_id, c.instruction_count);

				me->_nodes[i].hotspots[j].conditions.push_back(c);

				// Reading instructions
				for (int l = 0; l < c.instruction_count; l++) {
					NavInstruction ii;

					ii.instruction_type = in.readByte();
					ii.reserved = in.readByte();;
					ii.text = in.readPascalString();

					if (in.pos() % 2) // align to a word
						(void)in.readByte();

					debug(1, "      %d: type: %d reserved: %d text: %s", l, ii.instruction_type,
							ii.reserved, ii.text.c_str());

					me->_nodes[i].hotspots[j].conditions[k].instructions.push_back(ii);
				}

				pos1 += c.record_size;
			}

			pos += h.record_size;
		 }
	}

	mapnav_initialize_hidden_flags(me);

	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MapNavigatorXObj::m_dispose)

void MapNavigatorXObj::m_getFirstNode(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	g_lingo->push(me->_firstNodeIndex + 1); // Lingo node indices are 1-based
}

void MapNavigatorXObj::m_resetHidden(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	mapnav_initialize_hidden_flags(me);
}

void MapNavigatorXObj::m_getNodeName(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	Common::String nodeName = "";
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex >= 0 && nodeIndex < me->_nodeCount) {
		nodeName = me->_nodes[nodeIndex].name;
	} else {
		warning("MapNavigatorXObj::m_getNodeName: Invalid node index %d", nodeIndex);
	}

	g_lingo->push(nodeName);
}

void MapNavigatorXObj::m_getNodeIndex(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	Common::String nodeName = g_lingo->pop().asString();

	// The originakl used binary search since the node names were sorted. We ignore that
	for (int i = 0; i < me->_nodeCount; i++) {
		if (me->_nodes[i].name == nodeName) {
			g_lingo->push(i + 1); // Lingo node indices are 1-based
			return;
		}
	}
	warning("MapNavigatorXObj::m_getNodeIndex: Node name '%s' not found", nodeName.c_str());

	g_lingo->push(0);
}

void MapNavigatorXObj::m_getBackgroundPicture(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int result = 0;
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex >= 0 && nodeIndex < me->_nodeCount) {
		result = me->_nodes[nodeIndex].background_picture;
	} else {
		warning("MapNavigatorXObj::m_getBackgroundPicture: Invalid node index %d", nodeIndex);
	}

	g_lingo->push(result);
}

void MapNavigatorXObj::m_getHotSpotCount(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int result = 0;
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex >= 0 && nodeIndex < me->_nodeCount) {
		result = me->_nodes[nodeIndex].hotspot_count;
	} else {
		warning("MapNavigatorXObj::m_getHotSpotCount: Invalid node index %d", nodeIndex);
	}

	g_lingo->push(result);
}

void MapNavigatorXObj::m_setHidden(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int flag = g_lingo->pop().asInt();
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_setHidden: Invalid node index %d", nodeIndex);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_setHidden: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		return;
	}

	me->_nodes[nodeIndex].hotspots[hotspotIndex].isHidden = flag ? true : false;
}

void MapNavigatorXObj::m_getHidden(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getHidden: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getHidden: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].isHidden ? 1 : 0);
}

void MapNavigatorXObj::m_pointInside(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_pointInside: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_pointInside: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	Common::Rect rect(me->_nodes[nodeIndex].hotspots[hotspotIndex].top, me->_nodes[nodeIndex].hotspots[hotspotIndex].left,
			me->_nodes[nodeIndex].hotspots[hotspotIndex].bottom,
			me->_nodes[nodeIndex].hotspots[hotspotIndex].right);

	g_lingo->push(rect.contains(x, y) ? 1 : 0);
}

void MapNavigatorXObj::m_getHotSpotRect(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_pointInside: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_pointInside: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	Common::String result = Common::String::format("%d,%d,%d,%d",
			me->_nodes[nodeIndex].hotspots[hotspotIndex].top, me->_nodes[nodeIndex].hotspots[hotspotIndex].left,
			me->_nodes[nodeIndex].hotspots[hotspotIndex].bottom, me->_nodes[nodeIndex].hotspots[hotspotIndex].right);

	g_lingo->push(result);
}

void MapNavigatorXObj::m_getHotSpotCursor(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getHotSpotCursor: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getHotSpotCursor: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].cursor_id);
}

void MapNavigatorXObj::m_getEvaluationFcn(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getEvaluationFcn: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getEvaluationFcn: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].evaluation_name);
}

void MapNavigatorXObj::m_getDestinationNode(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int conditionIndex = g_lingo->pop().asInt() - 1; // Lingo condition indices are 1-based
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getDestinationNode: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getDestinationNode: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (conditionIndex < 0 || conditionIndex >= me->_nodes[nodeIndex].hotspots[hotspotIndex].condition_count) {
		warning("MapNavigatorXObj::m_getDestinationNode: Invalid condition index %d for hotspot %d of node %d", conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].destination_node + 1); // Lingo node indices are 1-based
}

void MapNavigatorXObj::m_getInstructionCount(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int conditionIndex = g_lingo->pop().asInt() - 1; // Lingo condition indices are 1-based
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getInstructionCount: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getInstructionCount: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (conditionIndex < 0 || conditionIndex >= me->_nodes[nodeIndex].hotspots[hotspotIndex].condition_count) {
		warning("MapNavigatorXObj::m_getInstructionCount: Invalid condition index %d for hotspot %d of node %d", conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].instruction_count);
}

void MapNavigatorXObj::m_getInstructionType(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int index = g_lingo->pop().asInt() - 1; // Lingo instruction indices are 1-based
	int conditionIndex = g_lingo->pop().asInt() - 1; // Lingo condition indices are 1-based
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getInstructionType: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getInstructionType: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (conditionIndex < 0 || conditionIndex >= me->_nodes[nodeIndex].hotspots[hotspotIndex].condition_count) {
		warning("MapNavigatorXObj::m_getInstructionType: Invalid condition index %d for hotspot %d of node %d", conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (index < 0 || index >= me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].instruction_count) {
		warning("MapNavigatorXObj::m_getInstructionType: Invalid instruction index %d for condition %d of hotspot %d of node %d", index, conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].instructions[index].instruction_type);
}

void MapNavigatorXObj::m_getInstruction(int nargs) {
	MapNavigatorXObject *me = static_cast<MapNavigatorXObject *>(g_lingo->_state->me.u.obj);

	int index = g_lingo->pop().asInt() - 1; // Lingo instruction indices are 1-based
	int conditionIndex = g_lingo->pop().asInt() - 1; // Lingo condition indices are 1-based
	int hotspotIndex = g_lingo->pop().asInt() - 1; // Lingo hotspot indices are 1-based
	int nodeIndex = g_lingo->pop().asInt() - 1; // Lingo node indices are 1-based

	if (nodeIndex < 0 || nodeIndex >= me->_nodeCount) {
		warning("MapNavigatorXObj::m_getInstruction: Invalid node index %d", nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (hotspotIndex < 0 || hotspotIndex >= me->_nodes[nodeIndex].hotspot_count) {
		warning("MapNavigatorXObj::m_getInstruction: Invalid hotspot index %d for node %d", hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (conditionIndex < 0 || conditionIndex >= me->_nodes[nodeIndex].hotspots[hotspotIndex].condition_count) {
		warning("MapNavigatorXObj::m_getInstruction: Invalid condition index %d for hotspot %d of node %d", conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	if (index < 0 || index >= me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].instruction_count) {
		warning("MapNavigatorXObj::m_getInstruction: Invalid instruction index %d for condition %d of hotspot %d of node %d", index, conditionIndex, hotspotIndex, nodeIndex);
		g_lingo->push(0);
		return;
	}

	g_lingo->push(me->_nodes[nodeIndex].hotspots[hotspotIndex].conditions[conditionIndex].instructions[index].text);
}

}
