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
#include "twp/util.h"

#define EPSILON 1e-9

namespace Twp {

struct Segment {
	Segment(Math::Vector2d s, Math::Vector2d t);
	void normalize();
	float distance(Math::Vector2d p);

	Math::Vector2d start, to;
	float left, right, top, bottom;
	float a, b, c;
};

Segment::Segment(Math::Vector2d s, Math::Vector2d t) {
	start = s;
	to = t;
	left = MIN(s.getX(), t.getX());
	right = MAX(s.getX(), t.getX());
	top = MIN(s.getY(), t.getY());
	bottom = MAX(s.getY(), t.getY());
	a = s.getY() - t.getY();
	b = t.getX() - s.getX();
	c = -a * s.getX() - b * s.getY();
	normalize();
}

void Segment::normalize() {
	float z = sqrt(a * a + b * b);
	if (abs(z) > EPSILON) {
		a /= z;
		b /= z;
		c /= z;
	}
}

float Segment::distance(Math::Vector2d p) {
	return a * p.getX() + b * p.getY() + c;
}

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

Graph::Graph(const Graph &graph) {
	_nodes = graph._nodes;
	_concaveVertices = graph._concaveVertices;
	for (int i = 0; i < graph._edges.size(); i++) {
		const Common::Array<GraphEdge> &e = graph._edges[i];
		Common::Array<GraphEdge> sEdges;
		for (int j = 0; j < e.size(); j++) {
			const GraphEdge &se = e[j];
			sEdges.push_back(GraphEdge(se.start, se.to, se.cost));
		}
		_edges.push_back(sEdges);
	}
}

GraphEdge::GraphEdge(int s, int t, float c)
	: start(s), to(t), cost(c) {
}

void Graph::addNode(Math::Vector2d node) {
	_nodes.push_back(node);
	_edges.push_back(Common::Array<GraphEdge>());
}

AStar::AStar(Graph *graph)
	: _fCost(graph->_nodes.size()), _gCost(graph->_nodes.size()), _spt(graph->_nodes.size()), _sf(graph->_nodes.size()) {
	_graph = graph;
}

// TODO this really should have some simd optimization
// matrix multiplication is based on this
static float dot(Math::Vector2d u, Math::Vector2d v) {
	float result = 0.f;
	result += u.getX() * v.getX();
	result += u.getY() * v.getY();
	return result;
}

static float length(Math::Vector2d v) { return sqrt(dot(v, v)); }

void AStar::search(int source, int target) {
	IndexedPriorityQueue pq(_fCost);
	pq.insert(source);
	while (!pq.isEmpty()) {
		int NCN = pq.pop();
		_spt[NCN] = _sf[NCN];
		if (NCN != target) {
			// for (edge in _graph->edges[NCN]) {
			for (int i = 0; i < _graph->_edges[NCN].size(); i++) {
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
	for (int i = 0; i < edges.size(); i++) {
		GraphEdge *e = &edges[i];
		if (e->to == to)
			return e;
	}
	return nullptr;
}

Common::Array<int> reverse(const Common::Array<int> &arr) {
	Common::Array<int> result(arr.size());
	for (int i = 0; i < arr.size(); i++) {
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

// Indicates whether or not the specified position is inside this walkbox.
static bool inside(const Walkbox &self, Math::Vector2d position, bool toleranceOnOutside = true) {
	bool result = false;
	Math::Vector2d point = position;
	const float epsilon = 1.0f;

	// Must have 3 or more edges
	const Common::Array<Math::Vector2d> &polygon = self.getPoints();
	if (polygon.size() < 3)
		return false;

	Math::Vector2d oldPoint(polygon[polygon.size() - 1]);
	float oldSqDist = distanceSquared(oldPoint, point);

	for (int i = 0; i < polygon.size(); i++) {
		Math::Vector2d newPoint = polygon[i];
		float newSqDist = distanceSquared(newPoint, point);

		if (oldSqDist + newSqDist + 2.0f * sqrt(oldSqDist * newSqDist) - distanceSquared(newPoint, oldPoint) < epsilon)
			return toleranceOnOutside;

		Math::Vector2d left;
		Math::Vector2d right;
		if (newPoint.getX() > oldPoint.getX()) {
			left = oldPoint;
			right = newPoint;
		} else {
			left = newPoint;
			right = oldPoint;
		}

		if ((left.getX() < point.getX()) && (point.getX() <= right.getX()) && ((point.getY() - left.getY()) * (right.getX() - left.getX()) < (right.getY() - left.getY()) * (point.getX() - left.getX())))
			result = !result;

		oldPoint = newPoint;
		oldSqDist = newSqDist;
	}
	return result;
}

Math::Vector2d Walkbox::getClosestPointOnEdge(Math::Vector2d p3) const {
	int vi1 = -1;
	int vi2 = -1;
	float minDist = 100000.0f;

	const Common::Array<Math::Vector2d> &polygon = getPoints();
	for (int i = 0; i < polygon.size(); i++) {
		float dist = distanceToSegment(p3, polygon[i], polygon[(i + 1) % polygon.size()]);
		if (dist < minDist) {
			minDist = dist;
			vi1 = i;
			vi2 = (i + 1) % polygon.size();
		}
	}

	Math::Vector2d p1 = polygon[vi1];
	Math::Vector2d p2 = polygon[vi2];

	float x1 = p1.getX();
	float y1 = p1.getY();
	float x2 = p2.getX();
	float y2 = p2.getY();
	float x3 = p3.getX();
	float y3 = p3.getY();

	float u = (((x3 - x1) * (x2 - x1)) + ((y3 - y1) * (y2 - y1))) / (((x2 - x1) * (x2 - x1)) + ((y2 - y1) * (y2 - y1)));

	float xu = x1 + u * (x2 - x1);
	float yu = y1 + u * (y2 - y1);

	if (u < 0)
		return Math::Vector2d(x1, y1);
	if (u > 1)
		return Math::Vector2d(x2, y2);
	return Math::Vector2d(xu, yu);
}

static bool less(Math::Vector2d p1, Math::Vector2d p2) {
	return ((p1.getX() < p2.getX() - EPSILON) || (abs(p1.getX() - p2.getX()) < EPSILON) && (p1.getY() < p2.getY() - EPSILON));
}

static float det(float a, float b, float c, float d) {
	return a * d - b * c;
}

static bool betw(float l, float r, float x) {
	return (MIN(l, r) <= x + EPSILON) && (x <= MAX(l, r) + EPSILON);
}

static bool intersect_1d(float a, float b, float c, float d) {
	float a2 = a;
	float b2 = b;
	float c2 = c;
	float d2 = d;
	if (a2 > b2)
		SWAP(a2, b2);
	if (c2 > d2)
		SWAP(c2, d2);
	return MAX(a2, c2) <= MIN(b2, d2) + EPSILON;
}

static bool lineSegmentsCross(Math::Vector2d a1, Math::Vector2d b1, Math::Vector2d c1, Math::Vector2d d1) {
	Math::Vector2d a = a1;
	Math::Vector2d b = b1;
	Math::Vector2d c = c1;
	Math::Vector2d d = d1;
	if ((!intersect_1d(a.getX(), b.getX(), c.getX(), d.getX())) || (!intersect_1d(a.getY(), b.getY(), c.getY(), d.getY())))
		return false;

	Segment m(a, b);
	Segment n(c, d);
	float zn = det(m.a, m.b, n.a, n.b);

	if (abs(zn) < EPSILON) {
		if ((abs(m.distance(c)) > EPSILON) || (abs(n.distance(a)) > EPSILON))
			return false;

		if (less(b, a))
			SWAP(a, b);
		if (less(d, c))
			SWAP(c, d);
		return true;
	}

	float lx = -det(m.c, m.b, n.c, n.b) / zn;
	float ly = -det(m.a, m.c, n.a, n.c) / zn;
	return betw(a.getX(), b.getX(), lx) && betw(a.getY(), b.getY(), ly) && betw(c.getX(), d.getX(), lx) && betw(c.getY(), d.getY(), ly);
}

bool PathFinder::inLineOfSight(Math::Vector2d start, Math::Vector2d to) {
	const float epsilon = 0.5f;

	// Not in LOS if any of the ends is outside the polygon
	if (!_walkboxes[0].contains(start) || !_walkboxes[0].contains(to))
		return false;

	// In LOS if it's the same start and end location
	if (length(start - to) < epsilon)
		return true;

	// Not in LOS if any edge is intersected by the start-end line segment
	for (int i = 0; i < _walkboxes.size(); i++) {
		Walkbox &walkbox = _walkboxes[i];
		const Common::Array<Math::Vector2d> &polygon = walkbox.getPoints();
		int size = polygon.size();
		for (int j = 0; j < size; j++) {
			Math::Vector2d v1 = polygon[j];
			Math::Vector2d v2 = polygon[(j + 1) % size];
			if (!lineSegmentsCross(start, to, v1, v2))
				continue;

			// In some cases a 'snapped' endpoint is just a little over the line due to rounding errors. So a 0.5 margin is used to tackle those cases.
			if ((distanceToSegment(start, v1, v2) > epsilon) && (distanceToSegment(to, v1, v2) > epsilon))
				return false;
		}
	}

	// Finally the middle point in the segment determines if in LOS or not
	Math::Vector2d v2 = (start + to) / 2.0f;
	bool result = _walkboxes[0].contains(v2);
	for (int i = 1; i < _walkboxes.size(); i++) {
		if (_walkboxes[i].contains(v2, false))
			result = false;
	}
	return result;
}

static int minIndex(const Common::Array<float> values) {
	float min = values[0];
	int index = 0;
	for (int i = 1; i < values.size(); i++) {
		if (values[i] < min) {
			index = i;
			min = values[i];
		}
	}
	return index;
}

Graph *PathFinder::createGraph() {
	Graph *result = new Graph();
	for (int i = 0; i < _walkboxes.size(); i++) {
		Walkbox &walkbox = _walkboxes[i];
		if (walkbox.getPoints().size() > 2) {
			bool visible = walkbox.isVisible();
			for (int j = 0; j < walkbox.getPoints().size(); j++) {
				if (walkbox.concave(j) == visible) {
					Math::Vector2d vertex = walkbox.getPoints()[j];
					result->_concaveVertices.push_back(vertex);
					result->addNode(vertex);
				}
			}
		}
	}

	for (int i = 0; i < result->_concaveVertices.size(); i++) {
		for (int j = 0; j < result->_concaveVertices.size(); j++) {
			Math::Vector2d c1(result->_concaveVertices[i]);
			Math::Vector2d c2(result->_concaveVertices[j]);
			if (inLineOfSight(c1, c2)) {
				float d = distance(c1, c2);
				result->addEdge(GraphEdge(i, j, d));
			}
		}
	}
	return result;
}

Common::Array<Math::Vector2d> PathFinder::calculatePath(Math::Vector2d start, Math::Vector2d to) {
	Common::Array<Math::Vector2d> result;
	if (_walkboxes.size() > 0) {
		// find the walkbox where the actor is and put it first
		for (int i = 0; i < _walkboxes.size(); i++) {
			const Walkbox &wb = _walkboxes[i];
			if (inside(wb, start) && (i != 0)) {
				SWAP(_walkboxes[0], _walkboxes[i]);
				break;
			}
		}

		// if no walkbox has been found => find the nearest walkbox
		if (!inside(_walkboxes[0], start)) {
			Common::Array<float> dists(_walkboxes.size());
			for (int i = 0; i < _walkboxes.size(); i++) {
				Walkbox wb = _walkboxes[i];
				dists[i] = distance(wb.getClosestPointOnEdge(start), start);
			}

			int index = minIndex(dists);
			if (index != 0)
				SWAP(_walkboxes[0], _walkboxes[index]);
		}

		if (!_graph)
			_graph = createGraph();

		// create new node on start position
		Graph *walkgraph = new Graph(*_graph);
		int startNodeIndex = walkgraph->_nodes.size();

		// if destination is not inside current walkable area, then get the closest point
		const Walkbox &wb = _walkboxes[0];
		if (wb.isVisible() && !wb.contains(to))
			to = wb.getClosestPointOnEdge(to);

		walkgraph->addNode(start);

		for (int i = 0; i < walkgraph->_concaveVertices.size(); i++) {
			Math::Vector2d c = walkgraph->_concaveVertices[i];
			if (inLineOfSight(start, c))
				walkgraph->addEdge(GraphEdge(startNodeIndex, i, distance(start, c)));
		}

		// create new node on end position
		int endNodeIndex = walkgraph->_nodes.size();
		walkgraph->addNode(to);

		for (int i = 0; i < walkgraph->_concaveVertices.size(); i++) {
			Math::Vector2d c = walkgraph->_concaveVertices[i];
			if (inLineOfSight(to, c))
				walkgraph->addEdge(GraphEdge(i, endNodeIndex, distance(to, c)));
		}

		if (inLineOfSight(start, to))
			walkgraph->addEdge(GraphEdge(startNodeIndex, endNodeIndex, distance(start, to)));

		Common::Array<int> indices = walkgraph->getPath(startNodeIndex, endNodeIndex);
		for (int i = 0; i < indices.size(); i++) {
			int index = indices[i];
			result.push_back(walkgraph->_nodes[index]);
		}
	}
	return result;
}

} // namespace Twp
