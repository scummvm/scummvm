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

#ifndef NANCY_SCRIPT_GRAPHNODE_H
#define NANCY_SCRIPT_GRAPHNODE_H

#include "engines/nancy/script/mathtypes.h"

struct lua_State;

namespace Nancy {
namespace Script {

class SceneContext;

// Nancy21 lets a scene group its records under nodes of their own, made with
// Scene:GraphNode(). A record that was handed to a node with node:TransferChild()
// is moved by the transform of the node on top of its own, and its z counts from
// the one of the node. The tray of puzzle pieces scrolls its pieces that way.
// Nodes can be put under other nodes as well.
class GraphNode {
public:
	GraphNode(SceneContext &context) : _context(context) {}

	SceneContext &getContext() const { return _context; }

	// Including those of the nodes above
	double getZ() const;
	ScriptMatrix getTransform() const;

	GraphNode *getParent() const { return _parent; }
	void setParent(GraphNode *parent);

	// Whether this is the node, or one under it
	bool isUnder(const GraphNode *node) const;

	void push(lua_State *L);

	// Reads or writes "z" or "transform"; the key is at stack index 2, and a
	// new value at 3
	int getScriptField(lua_State *L);
	int setScriptField(lua_State *L);

private:
	SceneContext &_context;
	double _z = 0.0;
	ScriptMatrix _transform;
	GraphNode *_parent = nullptr;
};

void registerGraphNodeAPI(lua_State *L);

// The node at a stack index, or null when something else is there
GraphNode *testGraphNode(lua_State *L, int index);

} // End of namespace Script
} // End of namespace Nancy

#endif // NANCY_SCRIPT_GRAPHNODE_H
