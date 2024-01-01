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

#ifndef TWP_GRAPH_H
#define TWP_GRAPH_H

#include "common/array.h"
#include "math/vector2d.h"

namespace Twp {

class IndexedPriorityQueue {
public:
	explicit IndexedPriorityQueue(Common::Array<float> &keys);

	void insert(int index);
	int pop();

	void reorderUp();
	void reorderDown();

	bool isEmpty();

private:
	Common::Array<float> &_keys;
	Common::Array<int> _data;
};

// An edge is a part of a walkable area, it is used by a Graph.
// See also:
//  - PathFinder
//  - Graph
struct GraphEdge {
	GraphEdge(int start, int to, float cost);

	int start;  // Index of the node in the graph representing the start of the edge.
	int to;     // Index of the node in the graph representing the end of the edge.
	float cost; // Cost of the edge in the graph.
};

// A graph helps to find a path between two points.
// This class has been ported from http://www.groebelsloot.com/2016/03/13/pathfinding-part-2/
// and modified
class Graph {
public:
	Graph();
	Graph(const Graph &graph);
	void addNode(Math::Vector2d node);
	void addEdge(GraphEdge edge);
	// Gets the edge from 'from' index to 'to' index.
	GraphEdge *edge(int start, int to);
	Common::Array<int> getPath(int source, int target);

	Common::Array<Math::Vector2d> _nodes;
	Common::Array<Common::Array<GraphEdge> > _edges;
	Common::Array<Math::Vector2d> _concaveVertices;
};

class AStar {
public:
	AStar(Graph *graph);
	void search(int source, int target);

	Graph *_graph = nullptr;
	Common::Array<GraphEdge *> _spt; // The Shortest Path Tree
	Common::Array<float> _gCost;     // This array will store the G cost of each node
	Common::Array<float> _fCost;     // This array will store the F cost of each node
	Common::Array<GraphEdge*> _sf;    // The Search Frontier
};

// Represents an area where an actor can or cannot walk
class Walkbox {
public:
	Walkbox(const Common::Array<Math::Vector2d> &polygon, bool visible = true);

  	// Indicates whether or not the specified position is inside this walkbox.
	bool contains(Math::Vector2d position, bool toleranceOnOutside = true) const;
	bool concave(int vertex) const;
	void setVisible(bool visible) { _visible = visible; }
	bool isVisible() const { return _visible; }
	const Common::Array<Math::Vector2d>& getPoints() const { return _polygon; }
	Math::Vector2d getClosestPointOnEdge(Math::Vector2d p3) const;

public:
	Common::String _name;

private:
	Common::Array<Math::Vector2d> _polygon;
	bool _visible;
};

// A PathFinder is used to find a walkable path within one or several walkboxes.
class PathFinder {
public:
	void setWalkboxes(const Common::Array<Walkbox> &walkboxes);
	Common::Array<Math::Vector2d> calculatePath(Math::Vector2d start, Math::Vector2d to);
	void setDirty(bool dirty) { _isDirty = dirty; }
	bool isDirty() const { return _isDirty; }
	const Graph* getGraph() const { return _graph; }

private:
	Graph *createGraph();
	bool inLineOfSight(Math::Vector2d start, Math::Vector2d to);

private:
	Common::Array<Walkbox> _walkboxes;
	Graph *_graph = nullptr;
	bool _isDirty = true;
};

} // namespace Twp

#endif
