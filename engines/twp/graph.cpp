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

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "twp/graph.h"
#include "twp/util.h"
#include "twp/clipper/clipper.hpp"

namespace Twp {

IndexedPriorityQueue::IndexedPriorityQueue(Common::Array<float> &keys)
	: _keys(keys) {
}

void IndexedPriorityQueue::insert(int index) {
	_data.push_back(index);
	reorderUp();
}

int IndexedPriorityQueue::pop() {
	int r = _data[0];
	_data[0] = _data[_data.size() - 1];
	_data.pop_back();
	reorderDown();
	return r;
}

void IndexedPriorityQueue::reorderUp() {
	if (_data.empty())
		return;
	size_t a = _data.size() - 1;
	while (a > 0) {
		if (_keys[_data[a]] >= _keys[_data[a - 1]])
			return;
		int tmp = _data[a];
		_data[a] = _data[a - 1];
		_data[a - 1] = tmp;
		a--;
	}
}

void IndexedPriorityQueue::reorderDown() {
	if (_data.empty())
		return;
	for (int a = 0; a < static_cast<int>(_data.size() - 1); a++) {
		if (_keys[_data[a]] <= _keys[_data[a + 1]])
			return;
		int tmp = _data[a];
		_data[a] = _data[a + 1];
		_data[a + 1] = tmp;
	}
}

bool IndexedPriorityQueue::isEmpty() {
	return _data.empty();
}

Graph::Graph() {}

GraphEdge::GraphEdge(int s, int t, float c)
	: start(s), to(t), cost(c) {
}

void Graph::addNode(Vector2i node) {
	_nodes.push_back(node);
	_edges.push_back(Common::Array<GraphEdge>());
}

AStar::AStar(Graph *graph)
	: _fCost(graph->_nodes.size()), _gCost(graph->_nodes.size()), _spt(graph->_nodes.size()), _sf(graph->_nodes.size()) {
	_graph = graph;
}

static float dot(Vector2i u, Vector2i v) {
	return (u.x * v.x) + (u.y * v.y);
}

static float length(Vector2i v) { return sqrt(dot(v, v)); }

void AStar::search(int source, int target) {
	IndexedPriorityQueue pq(_fCost);
	pq.insert(source);
	while (!pq.isEmpty()) {
		int NCN = pq.pop();
		_spt[NCN] = _sf[NCN];
		if (NCN != target) {
			for (size_t i = 0; i < _graph->_edges[NCN].size(); i++) {
				GraphEdge &edge = _graph->_edges[NCN][i];
				float Hcost = length(_graph->_nodes[edge.to] - _graph->_nodes[target]);
				float Gcost = _gCost[NCN] + edge.cost;
				if (!_sf[edge.to]) {
					_fCost[edge.to] = Gcost + Hcost;
					_gCost[edge.to] = Gcost;
					pq.insert(edge.to);
					_sf[edge.to] = &edge;
				} else if (Gcost < _gCost[edge.to] && !_spt[edge.to]) {
					_fCost[edge.to] = Gcost + Hcost;
					_gCost[edge.to] = Gcost;
					pq.reorderUp();
					_sf[edge.to] = &edge;
				}
			}
		}
	}
}

void Graph::addEdge(GraphEdge e) {
	if (!edge(e.start, e.to)) {
		_edges[e.start].push_back(e);
	}
	if (!edge(e.to, e.start)) {
		GraphEdge e2(e.to, e.start, e.cost);
		_edges[e.to].push_back(e);
	}
}

GraphEdge *Graph::edge(int start, int to) {
	Common::Array<GraphEdge> &edges = _edges[start];
	for (size_t i = 0; i < edges.size(); i++) {
		GraphEdge *e = &edges[i];
		if (e->to == to)
			return e;
	}
	return nullptr;
}

Common::Array<int> reverse(const Common::Array<int> &arr) {
	Common::Array<int> result(arr.size());
	for (size_t i = 0; i < arr.size(); i++) {
		result[arr.size() - 1 - i] = arr[i];
	}
	return result;
}

Common::Array<int> Graph::getPath(int source, int target) {
	Common::Array<int> result;
	AStar astar(this);
	if (target >= 0) {
		astar.search(source, target);
		int nd = target;
		result.push_back(nd);
		while ((nd != source) && (astar._spt[nd] != nullptr)) {
			nd = astar._spt[nd]->start;
			result.push_back(nd);
		}
		return reverse(result);
	}
	return result;
}

void PathFinder::setWalkboxes(const Common::Array<Walkbox> &walkboxes) {
	_walkboxes = walkboxes;
	_graph = nullptr;
}

static Vector2i toVector2i(float x, float y) {
	return Vector2i(round(x), round(y));
}

Vector2i Walkbox::getClosestPointOnEdge(Vector2i p) const {
	int vi1 = -1;
	int vi2 = -1;
	float minDist = 100000.0f;

	const Common::Array<Vector2i> &polygon = getPoints();
	for (size_t i = 0; i < polygon.size(); i++) {
		float dist = distanceToSegment(p, polygon[i], polygon[(i + 1) % polygon.size()]);
		if (dist < minDist) {
			minDist = dist;
			vi1 = i;
			vi2 = (i + 1) % polygon.size();
		}
	}

	Vector2i p1 = polygon[vi1];
	Vector2i p2 = polygon[vi2];

	float x1 = p1.x;
	float y1 = p1.y;
	float x2 = p2.x;
	float y2 = p2.y;
	float x3 = p.x;
	float y3 = p.y;

	float u = (((x3 - x1) * (x2 - x1)) + ((y3 - y1) * (y2 - y1))) / (((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

	float xu = x1 + u * (x2 - x1);
	float yu = y1 + u * (y2 - y1);

	if (u < 0)
		return toVector2i(x1, y1);
	if (u > 1)
		return toVector2i(x2, y2);
	return toVector2i(xu, yu);
}

static bool lineSegmentsCross(Vector2i a, Vector2i b, Vector2i c, Vector2i d) {
	const float EPSILON = 4.f;
	const float denominator = ((b.x - a.x) * (d.y - c.y)) - ((b.y - a.y) * (d.x - c.x));
	if (abs(denominator) < EPSILON) {
		return false;
	}

	const float numerator1 = ((a.y - c.y) * (d.x - c.x)) - ((a.x - c.x) * (d.y - c.y));
	const float numerator2 = ((a.y - c.y) * (b.x - a.x)) - ((a.x - c.x) * (b.y - a.y));
	if ((abs(numerator1) < EPSILON) || (abs(numerator2) < EPSILON)) {
		return false;
	}

	const float r = numerator1 / denominator;
	const float s = numerator2 / denominator;
	return ((r > 0.f) && (r < 1.f)) && ((s > 0.f) && (s < 1.f));
}

bool PathFinder::inLineOfSight(Vector2i start, Vector2i to) {
	const float epsilon = 0.5f;

	// Not in LOS if any of the ends is outside the polygon
	if (!_walkboxes[0].contains(start) || !_walkboxes[0].contains(to))
		return false;

	// In LOS if it's the same start and end location
	if (length(start - to) < epsilon)
		return true;

	// Not in LOS if any edge is intersected by the start-end line segment
	for (uint i = 0; i < _walkboxes.size(); i++) {
		const Walkbox &walkbox = _walkboxes[i];
		const Common::Array<Vector2i> &polygon = walkbox.getPoints();
		const uint size = polygon.size();
		for (uint j = 0; j < size; j++) {
			Vector2i v1 = polygon[j];
			Vector2i v2 = polygon[(j + 1) % size];
			if (!lineSegmentsCross(start, to, v1, v2))
				continue;

			// In some cases a 'snapped' endpoint is just a little over the line due to rounding errors. So a 0.5 margin is used to tackle those cases.
			if ((distanceToSegment(start, v1, v2) > epsilon) && (distanceToSegment(to, v1, v2) > epsilon))
				return false;
		}
	}

	// Finally the middle point in the segment determines if in LOS or not
	const Vector2i v2 = (start + to) / 2.0f;
	if (!_walkboxes[0].contains(v2))
		return false;
	for (uint i = 1; i < _walkboxes.size(); i++) {
		if (_walkboxes[i].contains(v2, false))
			return false;
	}
	return true;
}

static uint minIndex(const Common::Array<float> &values) {
	float min = values[0];
	uint index = 0;
	for (uint i = 1; i < values.size(); i++) {
		if (values[i] < min) {
			index = i;
			min = values[i];
		}
	}
	return index;
}

Graph *PathFinder::createGraph() {
	Graph *result = new Graph();
	for (uint i = 0; i < _walkboxes.size(); i++) {
		const Walkbox &walkbox = _walkboxes[i];
		if (walkbox.getPoints().size() > 2) {
			bool firstWalkbox = (i == 0);
			if (!walkbox.isVisible())
				firstWalkbox = true;
			for (uint j = 0; j < walkbox.getPoints().size(); j++) {
				if (walkbox.concave(j) == firstWalkbox) {
					const Vector2i &vertex = walkbox.getPoints()[j];
					result->_concaveVertices.push_back(vertex);
					result->addNode(vertex);
				}
			}
		}
	}

	for (uint i = 0; i < result->_concaveVertices.size(); i++) {
		for (uint j = 0; j < result->_concaveVertices.size(); j++) {
			const Vector2i c1(result->_concaveVertices[i]);
			const Vector2i c2(result->_concaveVertices[j]);
			if (inLineOfSight(c1, c2)) {
				const float d = distance(c1, c2);
				result->addEdge(GraphEdge(i, j, d));
			}
		}
	}
	return result;
}

Common::Array<Vector2i> PathFinder::calculatePath(Vector2i start, Vector2i to) {
	Common::Array<Vector2i> result;
	if (_walkboxes.size() > 0) {
		// find the walkbox where the actor is and put it first
		for (uint i = 0; i < _walkboxes.size(); i++) {
			const Walkbox &wb = _walkboxes[i];
			if (wb.contains(start) && (i != 0)) {
				SWAP(_walkboxes[0], _walkboxes[i]);
				break;
			}
		}

		// if no walkbox has been found => find the nearest walkbox
		if (!_walkboxes[0].contains(start)) {
			Common::Array<float> dists(_walkboxes.size());
			for (uint i = 0; i < _walkboxes.size(); i++) {
				const Walkbox &wb = _walkboxes[i];
				dists[i] = distance(wb.getClosestPointOnEdge(start), start);
			}

			const uint index = minIndex(dists);
			if (index != 0)
				SWAP(_walkboxes[0], _walkboxes[index]);
		}

		if (!_graph)
			_graph = createGraph();

		// create new node on start position
		_walkgraph = *_graph;
		const uint startNodeIndex = _walkgraph._nodes.size();

		// if destination is not inside current walkable area, then get the closest point
		const Walkbox &wb = _walkboxes[0];
		if (wb.isVisible() && !wb.contains(start)) {
			start = wb.getClosestPointOnEdge(start);
		}
		if (wb.isVisible() && !wb.contains(to)) {
			to = wb.getClosestPointOnEdge(to);
		}
		// we don't want the actor to walk in a different walkbox
		// then check if endpoint is inside one of the other walkboxes and find closest point on edge
		for (uint i = 1; i < _walkboxes.size(); i++) {
			if (_walkboxes[i].contains(to)) {
				to = _walkboxes[i].getClosestPointOnEdge(to);
				break;
			}
		}

		_walkgraph.addNode(start);

		for (uint i = 0; i < _walkgraph._concaveVertices.size(); i++) {
			const Vector2i c = _walkgraph._concaveVertices[i];
			if (inLineOfSight(start, c))
				_walkgraph.addEdge(GraphEdge(startNodeIndex, i, distance(start, c)));
		}

		// create new node on end position
		const uint endNodeIndex = _walkgraph._nodes.size();
		_walkgraph.addNode(to);

		for (uint i = 0; i < _walkgraph._concaveVertices.size(); i++) {
			const Vector2i c = _walkgraph._concaveVertices[i];
			if (inLineOfSight(to, c))
				_walkgraph.addEdge(GraphEdge(i, endNodeIndex, distance(to, c)));
		}

		if (inLineOfSight(start, to))
			_walkgraph.addEdge(GraphEdge(startNodeIndex, endNodeIndex, distance(start, to)));

		const Common::Array<int> indices = _walkgraph.getPath(startNodeIndex, endNodeIndex);
		for (uint i = 0; i < indices.size(); i++) {
			const int index = indices[i];
			result.push_back(_walkgraph._nodes[index]);
		}
	}
	return result;
}

} // namespace Twp
