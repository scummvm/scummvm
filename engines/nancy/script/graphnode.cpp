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

#include "engines/nancy/script/graphnode.h"
#include "engines/nancy/script/luabind.h"
#include "engines/nancy/script/record.h"
#include "engines/nancy/script/scenecontext.h"

namespace Nancy {
namespace Script {

static const char *const kGraphNodeType = "ND.GraphNode";

double GraphNode::getZ() const {
	return _parent ? _z + _parent->getZ() : _z;
}

ScriptMatrix GraphNode::getTransform() const {
	return _parent ? _transform * _parent->getTransform() : _transform;
}

void GraphNode::setParent(GraphNode *parent) {
	// A node can't go under itself
	if (parent && parent->isUnder(this)) {
		return;
	}

	_parent = parent;
	_context.onGraphNodeChanged(this);
}

bool GraphNode::isUnder(const GraphNode *node) const {
	for (const GraphNode *n = this; n; n = n->_parent) {
		if (n == node) {
			return true;
		}
	}

	return false;
}

void GraphNode::push(lua_State *L) {
	pushValue<GraphNode *>(L, kGraphNodeType, this);
}

int GraphNode::getScriptField(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "z") {
		lua_pushnumber(L, _z);
		return 1;
	} else if (key == "transform") {
		pushMatrix(L, _transform);
		return 1;
	}

	return indexFunctions(L);
}

int GraphNode::setScriptField(lua_State *L) {
	const Common::String key = luaL_checkstring(L, 2);

	if (key == "z") {
		_z = luaL_checknumber(L, 3);
	} else if (key == "transform") {
		_transform = checkMatrix(L, 3);
	} else {
		return luaL_error(L, "GraphNode does not have field %s", key.c_str());
	}

	_context.onGraphNodeChanged(this);
	return 0;
}

GraphNode *testGraphNode(lua_State *L, int index) {
	GraphNode **node = (GraphNode **)testUserData(L, index, kGraphNodeType);
	return node ? *node : nullptr;
}

static GraphNode *checkGraphNode(lua_State *L, int index) {
	return *(GraphNode **)checkUserData(L, index, kGraphNodeType);
}

static int graphNodeIndex(lua_State *L) {
	return checkGraphNode(L, 1)->getScriptField(L);
}

static int graphNodeNewIndex(lua_State *L) {
	return checkGraphNode(L, 1)->setScriptField(L);
}

// node:TransferChild(child), where the child is a record or another node
static int graphNodeTransferChild(lua_State *L) {
	GraphNode *node = checkGraphNode(L, 1);

	GraphNode *childNode = testGraphNode(L, 2);
	if (childNode) {
		childNode->setParent(node);
	} else {
		checkRecord(L, 2)->setParentNode(node);
	}

	return 0;
}

static const luaL_Reg kGraphNodeFunctions[] = {
	{ "__index", graphNodeIndex },
	{ "__newindex", graphNodeNewIndex },
	{ "TransferChild", graphNodeTransferChild },
	{ nullptr, nullptr }
};

void registerGraphNodeAPI(lua_State *L) {
	registerType(L, kGraphNodeType, kGraphNodeFunctions);
}

} // End of namespace Script
} // End of namespace Nancy
