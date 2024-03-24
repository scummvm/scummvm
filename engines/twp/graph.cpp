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

#include "twp/graph.h"

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

void Graph::addNode(const Math::Vector2d &node) {
	_nodes.push_back(node);
	_edges.push_back(Common::Array<GraphEdge>());
}

AStar::AStar(Graph *graph)
	: _fCost(graph->_nodes.size()), _gCost(graph->_nodes.size()), _spt(graph->_nodes.size()), _sf(graph->_nodes.size()) {
	_graph = graph;
}

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

void Graph::addEdge(const GraphEdge &e) {
	if (!edge(e.start, e.to)) {
		_edges[e.start].push_back(e);
	}
	if (!edge(e.to, e.start)) {
		GraphEdge e2(e.to, e.start, e.cost);
		_edges[e.to].push_back(e2);
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

Math::Vector2d Walkbox::getClosestPointOnEdge(const Math::Vector2d &p) const {
	int vi1 = -1;
	int vi2 = -1;
	float minDist = 100000.0f;

	const Common::Array<Vector2i> &polygon = getPoints();
	for (size_t i = 0; i < polygon.size(); i++) {
		float dist = distanceToSegment(p, (Math::Vector2d)polygon[i], (Math::Vector2d)polygon[(i + 1) % polygon.size()]);
		if (dist < minDist) {
			minDist = dist;
			vi1 = i;
			vi2 = (i + 1) % polygon.size();
		}
	}

	Math::Vector2d p1 = (Math::Vector2d)polygon[vi1];
	Math::Vector2d p2 = (Math::Vector2d)polygon[vi2];

	float x1 = p1.getX();
	float y1 = p1.getY();
	float x2 = p2.getX();
	float y2 = p2.getY();
	float x3 = p.getX();
	float y3 = p.getY();

	float u = (((x3 - x1) * (x2 - x1)) + ((y3 - y1) * (y2 - y1))) / (((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

	float xu = x1 + u * (x2 - x1);
	float yu = y1 + u * (y2 - y1);

	if (u < 0)
		return Math::Vector2d(x1, y1);
	if (u > 1)
		return Math::Vector2d(x2, y2);
	return Math::Vector2d(xu, yu);
}

bool PathFinder::inLineOfSight(const Math::Vector2d &start, const Math::Vector2d &to) {
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
			Math::Vector2d v1 = (Math::Vector2d)polygon[j];
			Math::Vector2d v2 = (Math::Vector2d)polygon[(j + 1) % size];
			if (!lineSegmentsCross(start, to, v1, v2))
				continue;

			// In some cases a 'snapped' endpoint is just a little over the line due to rounding errors. So a 0.5 margin is used to tackle those cases.
			if ((distanceToSegment(start, v1, v2) > epsilon) && (distanceToSegment(to, v1, v2) > epsilon))
				return false;
		}
	}

	// Finally the middle point in the segment determines if in LOS or not
	const Math::Vector2d v2 = (start + to) / 2.0f;
	if (!_walkboxes[0].contains(v2))
		return false;
	for (uint i = 1; i < _walkboxes.size(); i++) {
		if (_walkboxes[i].contains(v2, false))
			return false;
	}
	return true;
}

Common::SharedPtr<Graph> PathFinder::createGraph() {
	Common::SharedPtr<Graph> result(new Graph());
	for (uint i = 0; i < _walkboxes.size(); i++) {
		const Walkbox &walkbox = _walkboxes[i];
		if (walkbox.getPoints().size() > 2) {
			bool firstWalkbox = (i == 0);
			if (!walkbox.isVisible())
				firstWalkbox = true;
			for (uint j = 0; j < walkbox.getPoints().size(); j++) {
				if (walkbox.concave(j) == firstWalkbox) {
					Math::Vector2d vertex = (Math::Vector2d)walkbox.getPoints()[j];
					result->_concaveVertices.push_back(vertex);
					result->addNode(vertex);
				}
			}
		}
	}

	for (uint i = 0; i < result->_concaveVertices.size(); i++) {
		for (uint j = 0; j < result->_concaveVertices.size(); j++) {
			const Math::Vector2d c1(result->_concaveVertices[i]);
			const Math::Vector2d c2(result->_concaveVertices[j]);
			if (inLineOfSight(c1, c2)) {
				const float d = distance(c1, c2);
				result->addEdge(GraphEdge(i, j, d));
			}
		}
	}
	return result;
}

Common::Array<Math::Vector2d> PathFinder::calculatePath(const Math::Vector2d &s, const Math::Vector2d &t) {
	Math::Vector2d start(s);
	Math::Vector2d to(t);
	Common::Array<Math::Vector2d> result;
	if (!_walkboxes.empty()) {
		// find the walkbox where the actor is and put it first
		for (uint i = 0; i < _walkboxes.size(); i++) {
			const Walkbox &wb = _walkboxes[i];
			if (wb.contains(start) && (i != 0)) {
				_graph.reset();
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

			const size_t index = minIndex(dists);
			if (index != 0) {
				_graph.reset();
				SWAP(_walkboxes[0], _walkboxes[index]);
			}
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
		// then check if endpoint is inside one of the other walkboxes and find the closest point on edge
		for (uint i = 1; i < _walkboxes.size(); i++) {
			if (_walkboxes[i].contains(to)) {
				to = _walkboxes[i].getClosestPointOnEdge(to);
				break;
			}
		}

		_walkgraph.addNode(start);

		for (uint i = 0; i < _walkgraph._concaveVertices.size(); i++) {
			const Math::Vector2d c = _walkgraph._concaveVertices[i];
			if (inLineOfSight(start, c))
				_walkgraph.addEdge(GraphEdge(startNodeIndex, i, distance(start, c)));
		}

		// create new node on end position
		const uint endNodeIndex = _walkgraph._nodes.size();
		_walkgraph.addNode(to);

		for (uint i = 0; i < _walkgraph._concaveVertices.size(); i++) {
			const Math::Vector2d c = _walkgraph._concaveVertices[i];
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
