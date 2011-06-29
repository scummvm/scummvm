/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/palette.h"
#include "sci/graphics/screen.h"

#include "common/debug-channels.h"
#include "common/list.h"
#include "common/system.h"

namespace Sci {

// TODO: Code cleanup

#define AVOIDPATH_DYNMEM_STRING "AvoidPath polyline"

#define POLY_LAST_POINT 0x7777
#define POLY_POINT_SIZE 4

// SCI-defined polygon types
enum {
	POLY_TOTAL_ACCESS = 0,
	POLY_NEAREST_ACCESS = 1,
	POLY_BARRED_ACCESS = 2,
	POLY_CONTAINED_ACCESS = 3
};

// Polygon containment types
enum {
	CONT_OUTSIDE = 0,
	CONT_ON_EDGE = 1,
	CONT_INSIDE = 2
};

#define HUGE_DISTANCE 0xFFFFFFFF

#define VERTEX_HAS_EDGES(V) ((V) != CLIST_NEXT(V))

// Error codes
enum {
	PF_OK = 0,
	PF_ERROR = -1,
	PF_FATAL = -2
};

// Floating point struct
struct FloatPoint {
	FloatPoint() : x(0), y(0) {}
	FloatPoint(float x_, float y_) : x(x_), y(y_) {}

	Common::Point toPoint() {
		return Common::Point((int16)(x + 0.5), (int16)(y + 0.5));
	}

	float x, y;
};

struct Vertex {
	// Location
	Common::Point v;

	// Vertex circular list entry
	Vertex *_next;	// next element
	Vertex *_prev;	// previous element

	// A* cost variables
	uint32 costF;
	uint32 costG;

	// Previous vertex in shortest path
	Vertex *path_prev;

public:
	Vertex(const Common::Point &p) : v(p) {
		costG = HUGE_DISTANCE;
		path_prev = NULL;
	}
};

class VertexList: public Common::List<Vertex *> {
public:
	bool contains(Vertex *v) {
		for (iterator it = begin(); it != end(); ++it) {
			if (v == *it)
				return true;
		}
		return false;
	}
};

/* Circular list definitions. */

#define CLIST_FOREACH(var, head)					\
	for ((var) = (head)->first();					\
		(var);							\
		(var) = ((var)->_next == (head)->first() ?	\
		    NULL : (var)->_next))

/* Circular list access methods. */
#define CLIST_NEXT(elm)		((elm)->_next)
#define CLIST_PREV(elm)		((elm)->_prev)

class CircularVertexList {
public:
	Vertex *_head;

public:
	CircularVertexList() : _head(0) {}

	Vertex *first() const {
		return _head;
	}

	void insertHead(Vertex *elm) {
		if (_head == NULL) {
			elm->_next = elm->_prev = elm;
		} else {
			elm->_next = _head;
			elm->_prev = _head->_prev;
			_head->_prev = elm;
			elm->_prev->_next = elm;
		}
		_head = elm;
	}

	static void insertAfter(Vertex *listelm, Vertex *elm) {
		elm->_prev = listelm;
		elm->_next = listelm->_next;
		listelm->_next->_prev = elm;
		listelm->_next = elm;
	}

	void remove(Vertex *elm) {
		if (elm->_next == elm) {
			_head = NULL;
		} else {
			if (_head == elm)
				_head = elm->_next;
			elm->_prev->_next = elm->_next;
			elm->_next->_prev = elm->_prev;
		}
	}

	bool empty() const {
		return _head == NULL;
	}

	uint size() const {
		int n = 0;
		Vertex *v;
		CLIST_FOREACH(v, this)
			++n;
		return n;
	}

	/**
	 * Reverse the order of the elements in this circular list.
	 */
	void reverse() {
		if (!_head)
			return;

		Vertex *elm = _head;
		do {
			SWAP(elm->_prev, elm->_next);
			elm = elm->_next;
		} while (elm != _head);
	}
};

struct Polygon {
	// SCI polygon type
	int type;

	// Circular list of vertices
	CircularVertexList vertices;

public:
	Polygon(int t) : type(t) {
	}

	~Polygon() {
		while (!vertices.empty()) {
			Vertex *vertex = vertices.first();
			vertices.remove(vertex);
			delete vertex;
		}
	}
};

typedef Common::List<Polygon *> PolygonList;

// Pathfinding state
struct PathfindingState {
	// List of all polygons
	PolygonList polygons;

	// Start and end points for pathfinding
	Vertex *vertex_start, *vertex_end;

	// Array of all vertices, used for sorting
	Vertex **vertex_index;

	// Total number of vertices
	int vertices;

	// Point to prepend and append to final path
	Common::Point *_prependPoint;
	Common::Point *_appendPoint;

	// Screen size
	int _width, _height;

	PathfindingState(int width, int height) : _width(width), _height(height) {
		vertex_start = NULL;
		vertex_end = NULL;
		vertex_index = NULL;
		_prependPoint = NULL;
		_appendPoint = NULL;
		vertices = 0;
	}

	~PathfindingState() {
		free(vertex_index);

		delete _prependPoint;
		delete _appendPoint;

		for (PolygonList::iterator it = polygons.begin(); it != polygons.end(); ++it) {
			delete *it;
		}
	}

	bool pointOnScreenBorder(const Common::Point &p);
	bool edgeOnScreenBorder(const Common::Point &p, const Common::Point &q);
	int findNearPoint(const Common::Point &p, Polygon *polygon, Common::Point *ret);
};

static Common::Point readPoint(SegmentRef list_r, int offset) {
	Common::Point point;

	if (list_r.isRaw) {	// dynmem blocks are raw
		point.x = (int16)READ_SCIENDIAN_UINT16(list_r.raw + offset * POLY_POINT_SIZE);
		point.y = (int16)READ_SCIENDIAN_UINT16(list_r.raw + offset * POLY_POINT_SIZE + 2);
	} else {
		point.x = list_r.reg[offset * 2].toUint16();
		point.y = list_r.reg[offset * 2 + 1].toUint16();
	}
	return point;
}

static void writePoint(SegmentRef ref, int offset, const Common::Point &point) {
	if (ref.isRaw) {	// dynmem blocks are raw
		WRITE_SCIENDIAN_UINT16(ref.raw + offset * POLY_POINT_SIZE, point.x);
		WRITE_SCIENDIAN_UINT16(ref.raw + offset * POLY_POINT_SIZE + 2, point.y);
	} else {
		ref.reg[offset * 2] = make_reg(0, point.x);
		ref.reg[offset * 2 + 1] = make_reg(0, point.y);
	}
}

static void draw_line(EngineState *s, Common::Point p1, Common::Point p2, int type, int width, int height) {
	// Colors for polygon debugging.
	// Green: Total access
	// Blue: Near-point access
	// Red : Barred access
	// Yellow: Contained access
	int poly_colors[4] = {
		g_sci->_gfxPalette->kernelFindColor(0, 255, 0),	// green
		g_sci->_gfxPalette->kernelFindColor(0, 0, 255),	// blue
		g_sci->_gfxPalette->kernelFindColor(255, 0, 0),	// red
		g_sci->_gfxPalette->kernelFindColor(255, 255, 0)	// yellow
	};

	// Clip
	// FIXME: Do proper line clipping
	p1.x = CLIP<int16>(p1.x, 0, width - 1);
	p1.y = CLIP<int16>(p1.y, 0, height - 1);
	p2.x = CLIP<int16>(p2.x, 0, width - 1);
	p2.y = CLIP<int16>(p2.y, 0, height - 1);

	assert(type >= 0 && type <= 3);
	g_sci->_gfxPaint->kernelGraphDrawLine(p1, p2, poly_colors[type], 255, 255);
}

static void draw_point(EngineState *s, Common::Point p, int start, int width, int height) {
	// Colors for starting and end point
	// Green: End point
	// Blue: Starting point
	int point_colors[2] = {
		g_sci->_gfxPalette->kernelFindColor(0, 255, 0),	// green
		g_sci->_gfxPalette->kernelFindColor(0, 0, 255)		// blue
	};

	Common::Rect rect = Common::Rect(p.x - 1, p.y - 1, p.x - 1 + 3, p.y - 1 + 3);

	// Clip
	rect.top = CLIP<int16>(rect.top, 0, height - 1);
	rect.bottom = CLIP<int16>(rect.bottom, 0, height - 1);
	rect.left = CLIP<int16>(rect.left, 0, width - 1);
	rect.right = CLIP<int16>(rect.right, 0, width - 1);

	assert(start >= 0 && start <= 1);
	if (g_sci->_gfxPaint16)
		g_sci->_gfxPaint16->kernelGraphFrameBox(rect, point_colors[start]);
}

static void draw_polygon(EngineState *s, reg_t polygon, int width, int height) {
	SegManager *segMan = s->_segMan;
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));

#ifdef ENABLE_SCI32
	if (segMan->isHeapObject(points))
		points = readSelector(segMan, points, SELECTOR(data));
#endif

	int size = readSelectorValue(segMan, polygon, SELECTOR(size));
	int type = readSelectorValue(segMan, polygon, SELECTOR(type));
	Common::Point first, prev;
	int i;

	SegmentRef pointList = segMan->dereference(points);
	if (!pointList.isValid() || pointList.skipByte) {
		warning("draw_polygon: Polygon data pointer is invalid, skipping polygon");
		return;
	}

	prev = first = readPoint(pointList, 0);

	for (i = 1; i < size; i++) {
		Common::Point point = readPoint(pointList, i);
		draw_line(s, prev, point, type, width, height);
		prev = point;
	}

	draw_line(s, prev, first, type % 3, width, height);
}

static void draw_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt, int width, int height) {
	List *list;
	Node *node;

	draw_point(s, start, 1, width, height);
	draw_point(s, end, 0, width, height);

	if (!poly_list.segment)
		return;

	list = s->_segMan->lookupList(poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	node = s->_segMan->lookupNode(list->first);

	while (node) {
		draw_polygon(s, node->value, width, height);
		node = s->_segMan->lookupNode(node->succ);
	}
}

static void print_polygon(SegManager *segMan, reg_t polygon) {
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));

#ifdef ENABLE_SCI32
	if (segMan->isHeapObject(points))
		points = readSelector(segMan, points, SELECTOR(data));
#endif

	int size = readSelectorValue(segMan, polygon, SELECTOR(size));
	int type = readSelectorValue(segMan, polygon, SELECTOR(type));
	int i;
	Common::Point point;

	debugN(-1, "%i:", type);

	SegmentRef pointList = segMan->dereference(points);
	if (!pointList.isValid() || pointList.skipByte) {
		warning("print_polygon: Polygon data pointer is invalid, skipping polygon");
		return;
	}

	for (i = 0; i < size; i++) {
		point = readPoint(pointList, i);
		debugN(-1, " (%i, %i)", point.x, point.y);
	}

	point = readPoint(pointList, 0);
	debug(" (%i, %i);", point.x, point.y);
}

static void print_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	List *list;
	Node *node;

	debug("Start point: (%i, %i)", start.x, start.y);
	debug("End point: (%i, %i)", end.x, end.y);
	debug("Optimization level: %i", opt);

	if (!poly_list.segment)
		return;

	list = s->_segMan->lookupList(poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	debug("Polygons:");
	node = s->_segMan->lookupNode(list->first);

	while (node) {
		print_polygon(s->_segMan, node->value);
		node = s->_segMan->lookupNode(node->succ);
	}
}

/**
 * Computes the area of a triangle
 * Parameters: (const Common::Point &) a, b, c: The points of the triangle
 * Returns   : (int) The area multiplied by two
 */
static int area(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return (b.x - a.x) * (a.y - c.y) - (c.x - a.x) * (a.y - b.y);
}

/**
 * Determines whether or not a point is to the left of a directed line
 * Parameters: (const Common::Point &) a, b: The directed line (a, b)
 *             (const Common::Point &) c: The query point
 * Returns   : (int) true if c is to the left of (a, b), false otherwise
 */
static bool left(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return area(a, b, c) > 0;
}

/**
 * Determines whether or not three points are collinear
 * Parameters: (const Common::Point &) a, b, c: The three points
 * Returns   : (int) true if a, b, and c are collinear, false otherwise
 */
static bool collinear(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	return area(a, b, c) == 0;
}

/**
 * Determines whether or not a point lies on a line segment
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (const Common::Point &) c: The query point
 * Returns   : (int) true if c lies on (a, b), false otherwise
 */
static bool between(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	if (!collinear(a, b, c))
		return false;

	// Assumes a != b.
	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

/**
 * Determines whether or not two line segments properly intersect
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (const Common::Point &) c, d: The line segment (c, d)
 * Returns   : (int) true if (a, b) properly intersects (c, d), false otherwise
 */
static bool intersect_proper(const Common::Point &a, const Common::Point &b, const Common::Point &c, const Common::Point &d) {
	int ab = (left(a, b, c) && left(b, a, d)) || (left(a, b, d) && left(b, a, c));
	int cd = (left(c, d, a) && left(d, c, b)) || (left(c, d, b) && left(d, c, a));

	return ab && cd;
}

/**
 * Polygon containment test
 * Parameters: (const Common::Point &) p: The point
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) CONT_INSIDE if p is strictly contained in polygon,
 *                   CONT_ON_EDGE if p lies on an edge of polygon,
 *                   CONT_OUTSIDE otherwise
 * Number of ray crossing left and right
 */
static int contained(const Common::Point &p, Polygon *polygon) {
	int lcross = 0, rcross = 0;
	Vertex *vertex;

	// Iterate over edges
	CLIST_FOREACH(vertex, &polygon->vertices) {
		const Common::Point &v1 = vertex->v;
		const Common::Point &v2 = CLIST_NEXT(vertex)->v;

		// Flags for ray straddling left and right
		int rstrad, lstrad;

		// Check if p is a vertex
		if (p == v1)
			return CONT_ON_EDGE;

		// Check if edge straddles the ray
		rstrad = (v1.y < p.y) != (v2.y < p.y);
		lstrad = (v1.y > p.y) != (v2.y > p.y);

		if (lstrad || rstrad) {
			// Compute intersection point x / xq
			int x = v2.x * v1.y - v1.x * v2.y + (v1.x - v2.x) * p.y;
			int xq = v1.y - v2.y;

			// Multiply by -1 if xq is negative (for comparison that follows)
			if (xq < 0) {
				x = -x;
				xq = -xq;
			}

			// Avoid floats by multiplying instead of dividing
			if (rstrad && (x > xq * p.x))
				rcross++;
			else if (lstrad && (x < xq * p.x))
				lcross++;
		}
	}

	// If we counted an odd number of total crossings the point is on an edge
	if ((lcross + rcross) % 2 == 1)
		return CONT_ON_EDGE;

	// If there are an odd number of crossings to one side the point is contained in the polygon
	if (rcross % 2 == 1) {
		// Invert result for contained access polygons.
		if (polygon->type == POLY_CONTAINED_ACCESS)
			return CONT_OUTSIDE;
		return CONT_INSIDE;
	}

	// Point is outside polygon. Invert result for contained access polygons
	if (polygon->type == POLY_CONTAINED_ACCESS)
		return CONT_INSIDE;

	return CONT_OUTSIDE;
}

/**
 * Computes polygon area
 * Parameters: (Polygon *) polygon: The polygon
 * Returns   : (int) The area multiplied by two
 */
static int polygon_area(Polygon *polygon) {
	Vertex *first = polygon->vertices.first();
	Vertex *v;
	int size = 0;

	v = CLIST_NEXT(first);

	while (CLIST_NEXT(v) != first) {
		size += area(first->v, v->v, CLIST_NEXT(v)->v);
		v = CLIST_NEXT(v);
	}

	return size;
}

/**
 * Fixes the vertex order of a polygon if incorrect. Contained access
 * polygons should have their vertices ordered clockwise, all other types
 * anti-clockwise
 * Parameters: (Polygon *) polygon: The polygon
 */
static void fix_vertex_order(Polygon *polygon) {
	int area = polygon_area(polygon);

	// When the polygon area is positive the vertices are ordered
	// anti-clockwise. When the area is negative the vertices are ordered
	// clockwise
	if (((area > 0) && (polygon->type == POLY_CONTAINED_ACCESS))
	        || ((area < 0) && (polygon->type != POLY_CONTAINED_ACCESS))) {

		polygon->vertices.reverse();
	}
}

/**
 * Determines whether or not a line from a point to a vertex intersects the
 * interior of the polygon, locally at that vertex
 * Parameters: (Common::Point) p: The point
 *             (Vertex *) vertex: The vertex
 * Returns   : (int) 1 if the line (p, vertex->v) intersects the interior of
 *                   the polygon, locally at the vertex. 0 otherwise
 */
static int inside(const Common::Point &p, Vertex *vertex) {
	// Check that it's not a single-vertex polygon
	if (VERTEX_HAS_EDGES(vertex)) {
		const Common::Point &prev = CLIST_PREV(vertex)->v;
		const Common::Point &next = CLIST_NEXT(vertex)->v;
		const Common::Point &cur = vertex->v;

		if (left(prev, cur, next)) {
			// Convex vertex, line (p, cur) intersects the inside
			// if p is located left of both edges
			if (left(cur, next, p) && left(prev, cur, p))
				return 1;
		} else {
			// Non-convex vertex, line (p, cur) intersects the
			// inside if p is located left of either edge
			if (left(cur, next, p) || left(prev, cur, p))
				return 1;
		}
	}

	return 0;
}

/**
 * Returns a list of all vertices that are visible from a particular vertex.
 * @param s				the pathfinding state
 * @param vertex_cur	the vertex
 * @return list of vertices that are visible from vert
 */
static VertexList *visible_vertices(PathfindingState *s, Vertex *vertex_cur) {
	VertexList *visVerts = new VertexList();

	for (int i = 0; i < s->vertices; i++) {
		Vertex *vertex = s->vertex_index[i];

		// Make sure we don't intersect a polygon locally at the vertices
		if ((vertex == vertex_cur) || (inside(vertex->v, vertex_cur)) || (inside(vertex_cur->v, vertex)))
			continue;

		// Check for intersecting edges
		int j;
		for (j = 0; j < s->vertices; j++) {
			Vertex *edge = s->vertex_index[j];
			if (VERTEX_HAS_EDGES(edge)) {
				if (between(vertex_cur->v, vertex->v, edge->v)) {
					// If we hit a vertex, make sure we can pass through it without intersecting its polygon
					if ((inside(vertex_cur->v, edge)) || (inside(vertex->v, edge)))
						break;

					// This edge won't properly intersect, so we continue
					continue;
				}

				if (intersect_proper(vertex_cur->v, vertex->v, edge->v, CLIST_NEXT(edge)->v))
					break;
			}
		}

		if (j == s->vertices)
			visVerts->push_front(vertex);
	}

	return visVerts;
}

/**
 * Determines if a point lies on the screen border
 * Parameters: (const Common::Point &) p: The point
 * Returns   : (int) true if p lies on the screen border, false otherwise
 */
bool PathfindingState::pointOnScreenBorder(const Common::Point &p) {
	return (p.x == 0) || (p.x == _width - 1) || (p.y == 0) || (p.y == _height - 1);
}

/**
 * Determines if an edge lies on the screen border
 * Parameters: (const Common::Point &) p, q: The edge (p, q)
 * Returns   : (int) true if (p, q) lies on the screen border, false otherwise
 */
bool PathfindingState::edgeOnScreenBorder(const Common::Point &p, const Common::Point &q) {
	return ((p.x == 0 && q.x == 0) || (p.y == 0 && q.y == 0)
			|| ((p.x == _width - 1) && (q.x == _width - 1))
			|| ((p.y == _height - 1) && (q.y == _height - 1)));
}

/**
 * Searches for a nearby point that is not contained in a polygon
 * Parameters: (FloatPoint) f: The pointf to search nearby
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) PF_OK on success, PF_FATAL otherwise
 *             (Common::Point) *ret: The non-contained point on success
 */
static int find_free_point(FloatPoint f, Polygon *polygon, Common::Point *ret) {
	Common::Point p;

	// Try nearest point first
	p = Common::Point((int)floor(f.x + 0.5), (int)floor(f.y + 0.5));

	if (contained(p, polygon) != CONT_INSIDE) {
		*ret = p;
		return PF_OK;
	}

	p = Common::Point((int)floor(f.x), (int)floor(f.y));

	// Try (x, y), (x + 1, y), (x , y + 1) and (x + 1, y + 1)
	if (contained(p, polygon) == CONT_INSIDE) {
		p.x++;
		if (contained(p, polygon) == CONT_INSIDE) {
			p.y++;
			if (contained(p, polygon) == CONT_INSIDE) {
				p.x--;
				if (contained(p, polygon) == CONT_INSIDE)
					return PF_FATAL;
			}
		}
	}

	*ret = p;
	return PF_OK;
}

/**
 * Computes the near point of a point contained in a polygon
 * Parameters: (const Common::Point &) p: The point
 *             (Polygon *) polygon: The polygon
 * Returns   : (int) PF_OK on success, PF_FATAL otherwise
 *             (Common::Point) *ret: The near point of p in polygon on success
 */
int PathfindingState::findNearPoint(const Common::Point &p, Polygon *polygon, Common::Point *ret) {
	Vertex *vertex;
	FloatPoint near_p;
	uint32 dist = HUGE_DISTANCE;

	CLIST_FOREACH(vertex, &polygon->vertices) {
		const Common::Point &p1 = vertex->v;
		const Common::Point &p2 = CLIST_NEXT(vertex)->v;
		float u;
		FloatPoint new_point;
		uint32 new_dist;

		// Ignore edges on the screen border, except for contained access polygons
		if ((polygon->type != POLY_CONTAINED_ACCESS) && (edgeOnScreenBorder(p1, p2)))
			continue;

		// Compute near point
		u = ((p.x - p1.x) * (p2.x - p1.x) + (p.y - p1.y) * (p2.y - p1.y)) / (float)p1.sqrDist(p2);

		// Clip to edge
		if (u < 0.0f)
			u = 0.0f;
		if (u > 1.0f)
			u = 1.0f;

		new_point.x = p1.x + u * (p2.x - p1.x);
		new_point.y = p1.y + u * (p2.y - p1.y);

		new_dist = p.sqrDist(new_point.toPoint());

		if (new_dist < dist) {
			near_p = new_point;
			dist = new_dist;
		}
	}

	// Find point not contained in polygon
	return find_free_point(near_p, polygon, ret);
}

/**
 * Computes the intersection point of a line segment and an edge (not
 * including the vertices themselves)
 * Parameters: (const Common::Point &) a, b: The line segment (a, b)
 *             (Vertex *) vertex: The first vertex of the edge
 * Returns   : (int) FP_OK on success, PF_ERROR otherwise
 *             (FloatPoint) *ret: The intersection point
 */
static int intersection(const Common::Point &a, const Common::Point &b, Vertex *vertex, FloatPoint *ret) {
	// Parameters of parametric equations
	float s, t;
	// Numerator and denominator of equations
	float num, denom;
	const Common::Point &c = vertex->v;
	const Common::Point &d = CLIST_NEXT(vertex)->v;

	denom = a.x * (float)(d.y - c.y) + b.x * (float)(c.y - d.y) +
	        d.x * (float)(b.y - a.y) + c.x * (float)(a.y - b.y);

	if (denom == 0.0)
		// Segments are parallel, no intersection
		return PF_ERROR;

	num = a.x * (float)(d.y - c.y) + c.x * (float)(a.y - d.y) + d.x * (float)(c.y - a.y);

	s = num / denom;

	num = -(a.x * (float)(c.y - b.y) + b.x * (float)(a.y - c.y) + c.x * (float)(b.y - a.y));

	t = num / denom;

	if ((0.0 <= s) && (s <= 1.0) && (0.0 < t) && (t < 1.0)) {
		// Intersection found
		ret->x = a.x + s * (b.x - a.x);
		ret->y = a.y + s * (b.y - a.y);
		return PF_OK;
	}

	return PF_ERROR;
}

/**
 * Computes the nearest intersection point of a line segment and the polygon
 * set. Intersection points that are reached from the inside of a polygon
 * are ignored as are improper intersections which do not obstruct
 * visibility
 * Parameters: (PathfindingState *) s: The pathfinding state
 *             (const Common::Point &) p, q: The line segment (p, q)
 * Returns   : (int) PF_OK on success, PF_ERROR when no intersections were
 *                   found, PF_FATAL otherwise
 *             (Common::Point) *ret: On success, the closest intersection point
 */
static int nearest_intersection(PathfindingState *s, const Common::Point &p, const Common::Point &q, Common::Point *ret) {
	Polygon *polygon = 0;
	FloatPoint isec;
	Polygon *ipolygon = 0;
	uint32 dist = HUGE_DISTANCE;

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices) {
			uint32 new_dist;
			FloatPoint new_isec;

			// Check for intersection with vertex
			if (between(p, q, vertex->v)) {
				// Skip this vertex if we hit it from the
				// inside of the polygon
				if (inside(q, vertex)) {
					new_isec.x = vertex->v.x;
					new_isec.y = vertex->v.y;
				} else
					continue;
			} else {
				// Check for intersection with edges

				// Skip this edge if we hit it from the
				// inside of the polygon
				if (!left(vertex->v, CLIST_NEXT(vertex)->v, q))
					continue;

				if (intersection(p, q, vertex, &new_isec) != PF_OK)
					continue;
			}

			new_dist = p.sqrDist(new_isec.toPoint());
			if (new_dist < dist) {
				ipolygon = polygon;
				isec = new_isec;
				dist = new_dist;
			}
		}
	}

	if (dist == HUGE_DISTANCE)
		return PF_ERROR;

	// Find point not contained in polygon
	return find_free_point(isec, ipolygon, ret);
}

/**
 * Checks whether a point is nearby a contained-access polygon (distance 1 pixel)
 * @param point			the point
 * @param polygon		the contained-access polygon
 * @return true when point is nearby polygon, false otherwise
 */
static bool nearbyPolygon(const Common::Point &point, Polygon *polygon) {
	assert(polygon->type == POLY_CONTAINED_ACCESS);

	return ((contained(Common::Point(point.x, point.y + 1), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x, point.y - 1), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x + 1, point.y), polygon) != CONT_INSIDE)
			|| (contained(Common::Point(point.x - 1, point.y), polygon) != CONT_INSIDE));
}

/**
 * Checks that the start point is in a valid position, and takes appropriate action if it's not.
 * @param s				the pathfinding state
 * @param start			the start point
 * @return a valid start point on success, NULL otherwise
 */
static Common::Point *fixup_start_point(PathfindingState *s, const Common::Point &start) {
	PolygonList::iterator it = s->polygons.begin();
	Common::Point *new_start = new Common::Point(start);

	while (it != s->polygons.end()) {
		int cont = contained(start, *it);
		int type = (*it)->type;

		switch (type) {
		case POLY_TOTAL_ACCESS:
			// Remove totally accessible polygons that contain the start point
			if (cont != CONT_OUTSIDE) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			break;
		case POLY_CONTAINED_ACCESS:
			// Remove contained access polygons that do not contain
			// the start point (containment test is inverted here).
			// SSCI appears to be using a small margin of error here,
			// so we do the same.
			if ((cont == CONT_INSIDE) && !nearbyPolygon(start, *it)) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			// Fall through
		case POLY_BARRED_ACCESS:
		case POLY_NEAREST_ACCESS:
			if (cont != CONT_OUTSIDE) {
				if (s->_prependPoint != NULL) {
					// We shouldn't get here twice.
					// We need to break in this case, otherwise we'll end in an infinite
					// loop.
					warning("AvoidPath: start point is contained in multiple polygons");
					break;
				}

				if (s->findNearPoint(start, (*it), new_start) != PF_OK) {
					delete new_start;
					return NULL;
				}

				if ((type == POLY_BARRED_ACCESS) || (type == POLY_CONTAINED_ACCESS))
					debugC(kDebugLevelAvoidPath, "AvoidPath: start position at unreachable location");

				// The original start position is in an invalid location, so we
				// use the moved point and add the original one to the final path
				// later on.
				if (start != *new_start)
					s->_prependPoint = new Common::Point(start);
			}
		}

		++it;
	}

	return new_start;
}

/**
 * Checks that the end point is in a valid position, and takes appropriate action if it's not.
 * @param s				the pathfinding state
 * @param end			the end point
 * @return a valid end point on success, NULL otherwise
 */
static Common::Point *fixup_end_point(PathfindingState *s, const Common::Point &end) {
	PolygonList::iterator it = s->polygons.begin();
	Common::Point *new_end = new Common::Point(end);

	while (it != s->polygons.end()) {
		int cont = contained(end, *it);
		int type = (*it)->type;

		switch (type) {
		case POLY_TOTAL_ACCESS:
			// Remove totally accessible polygons that contain the end point
			if (cont != CONT_OUTSIDE) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			break;
		case POLY_CONTAINED_ACCESS:
		case POLY_BARRED_ACCESS:
		case POLY_NEAREST_ACCESS:
			if (cont != CONT_OUTSIDE) {
				if (s->_appendPoint != NULL) {
					// We shouldn't get here twice.
					// Happens in LB2CD, inside the speakeasy when walking from the
					// speakeasy (room 310) into the bathroom (room 320), after having
					// consulted the notebook (bug #3036299).
					// We need to break in this case, otherwise we'll end in an infinite
					// loop.
					warning("AvoidPath: end point is contained in multiple polygons");
					break;
				}

				// The original end position is in an invalid location, so we move the point
				if (s->findNearPoint(end, (*it), new_end) != PF_OK) {
					delete new_end;
					return NULL;
				}

				// For near-point access polygons we need to add the original end point
				// to the path after pathfinding.
				if ((type == POLY_NEAREST_ACCESS) && (end != *new_end))
					s->_appendPoint = new Common::Point(end);
			}
		}

		++it;
	}

	return new_end;
}

/**
 * Merges a point into the polygon set. A new vertex is allocated for this
 * point, unless a matching vertex already exists. If the point is on an
 * already existing edge that edge is split up into two edges connected by
 * the new vertex
 * Parameters: (PathfindingState *) s: The pathfinding state
 *             (const Common::Point &) v: The point to merge
 * Returns   : (Vertex *) The vertex corresponding to v
 */
static Vertex *merge_point(PathfindingState *s, const Common::Point &v) {
	Vertex *vertex;
	Vertex *v_new;
	Polygon *polygon;

	// Check for already existing vertex
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		CLIST_FOREACH(vertex, &polygon->vertices) {
			if (vertex->v == v)
				return vertex;
		}
	}

	v_new = new Vertex(v);

	// Check for point being on an edge
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		// Skip single-vertex polygons
		if (VERTEX_HAS_EDGES(polygon->vertices.first())) {
			CLIST_FOREACH(vertex, &polygon->vertices) {
				Vertex *next = CLIST_NEXT(vertex);

				if (between(vertex->v, next->v, v)) {
					// Split edge by adding vertex
					polygon->vertices.insertAfter(vertex, v_new);
					return v_new;
				}
			}
		}
	}

	// Add point as single-vertex polygon
	polygon = new Polygon(POLY_BARRED_ACCESS);
	polygon->vertices.insertHead(v_new);
	s->polygons.push_front(polygon);

	return v_new;
}

/**
 * Converts an SCI polygon into a Polygon
 * Parameters: (EngineState *) s: The game state
 *             (reg_t) polygon: The SCI polygon to convert
 * Returns   : (Polygon *) The converted polygon, or NULL on error
 */
static Polygon *convert_polygon(EngineState *s, reg_t polygon) {
	SegManager *segMan = s->_segMan;
	int i;
	reg_t points = readSelector(segMan, polygon, SELECTOR(points));
	int size = readSelectorValue(segMan, polygon, SELECTOR(size));

#ifdef ENABLE_SCI32
	// SCI32 stores the actual points in the data property of points (in a new array)
	if (segMan->isHeapObject(points))
		points = readSelector(segMan, points, SELECTOR(data));
#endif

	if (size == 0) {
		// If the polygon has no vertices, we skip it
		return NULL;
	}

	SegmentRef pointList = segMan->dereference(points);
	// Check if the target polygon is still valid. It may have been released
	// in the meantime (e.g. in LSL6, room 700, when using the elevator).
	// Refer to bug #3034501.
	if (!pointList.isValid() || pointList.skipByte) {
		warning("convert_polygon: Polygon data pointer is invalid, skipping polygon");
		return NULL;
	}

	// Make sure that we have enough points
	if (pointList.maxSize < size * POLY_POINT_SIZE) {
		warning("convert_polygon: Not enough memory allocated for polygon points. "
				"Expected %d, got %d. Skipping polygon",
				size * POLY_POINT_SIZE, pointList.maxSize);
		return NULL;
	}

	int skip = 0;

	// WORKAROUND: broken polygon in lsl1sci, room 350, after opening elevator
	// Polygon has 17 points but size is set to 19
	if ((size == 19) && g_sci->getGameId() == GID_LSL1) {
		if ((s->currentRoomNumber() == 350)
		&& (readPoint(pointList, 18) == Common::Point(108, 137))) {
			debug(1, "Applying fix for broken polygon in lsl1sci, room 350");
			size = 17;
		}
	}

	Polygon *poly = new Polygon(readSelectorValue(segMan, polygon, SELECTOR(type)));

	for (i = skip; i < size; i++) {
		Vertex *vertex = new Vertex(readPoint(pointList, i));
		poly->vertices.insertHead(vertex);
	}

	fix_vertex_order(poly);

	return poly;
}

/**
 * Changes the polygon list for optimization level 0 (used for keyboard
 * support). Totally accessible polygons are removed and near-point
 * accessible polygons are changed into totally accessible polygons.
 * Parameters: (PathfindingState *) s: The pathfinding state
 */
static void change_polygons_opt_0(PathfindingState *s) {

	PolygonList::iterator it = s->polygons.begin();
	while (it != s->polygons.end()) {
		Polygon *polygon = *it;
		assert(polygon);

		if (polygon->type == POLY_TOTAL_ACCESS) {
			delete polygon;
			it = s->polygons.erase(it);
		} else {
			if (polygon->type == POLY_NEAREST_ACCESS)
				polygon->type = POLY_TOTAL_ACCESS;
			++it;
		}
	}
}

/**
 * Converts the SCI input data for pathfinding
 * Parameters: (EngineState *) s: The game state
 *             (reg_t) poly_list: Polygon list
 *             (Common::Point) start: The start point
 *             (Common::Point) end: The end point
 *             (int) opt: Optimization level (0, 1 or 2)
 * Returns   : (PathfindingState *) On success a newly allocated pathfinding state,
 *                            NULL otherwise
 */
static PathfindingState *convert_polygon_set(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int width, int height, int opt) {
	SegManager *segMan = s->_segMan;
	Polygon *polygon;
	int count = 0;
	PathfindingState *pf_s = new PathfindingState(width, height);

	// Convert all polygons
	if (poly_list.segment) {
		List *list = s->_segMan->lookupList(poly_list);
		Node *node = s->_segMan->lookupNode(list->first);

		while (node) {
			// The node value might be null, in which case there's no polygon to parse.
			// Happens in LB2 floppy - refer to bug #3041232
			polygon = !node->value.isNull() ? convert_polygon(s, node->value) : NULL;

			if (polygon) {
				pf_s->polygons.push_back(polygon);
				count += readSelectorValue(segMan, node->value, SELECTOR(size));
			}

			node = s->_segMan->lookupNode(node->succ);
		}
	}

	if (opt == 0)
		change_polygons_opt_0(pf_s);

	Common::Point *new_start = fixup_start_point(pf_s, start);

	if (!new_start) {
		warning("AvoidPath: Couldn't fixup start position for pathfinding");
		delete pf_s;
		return NULL;
	}

	Common::Point *new_end = fixup_end_point(pf_s, end);

	if (!new_end) {
		warning("AvoidPath: Couldn't fixup end position for pathfinding");
		delete new_start;
		delete pf_s;
		return NULL;
	}

	if (opt == 0) {
		// Keyboard support. Only the first edge of the path we compute
		// here matches the path returned by SSCI. This is assumed to be
		// sufficient as all known use cases only use the first two
		// vertices of the returned path.
		// Pharkas uses this mode for a secondary polygon set containing
		// rectangular polygons used to block an actor's path.

		// If we have a prepended point, we do nothing here as the
		// actor is in barred territory and should be moved outside of
		// it ASAP. This matches the behavior of SSCI.
		if (!pf_s->_prependPoint) {
			// Actor position is OK, find nearest obstacle.
			int err = nearest_intersection(pf_s, start, *new_end, new_start);

			if (err == PF_FATAL) {
				warning("AvoidPath: error finding nearest intersection");
				delete new_start;
				delete new_end;
				delete pf_s;
				return NULL;
			}

			if (err == PF_OK)
				pf_s->_prependPoint = new Common::Point(start);
		}
	} else {
		// WORKAROUND LSL5 room 660. Priority glitch due to us choosing a different path
		// than SSCI. Happens when Patti walks to the control room.
		if (g_sci->getGameId() == GID_LSL5 && (s->currentRoomNumber() == 660) && (Common::Point(67, 131) == *new_start) && (Common::Point(229, 101) == *new_end)) {
			debug(1, "[avoidpath] Applying fix for priority problem in LSL5, room 660");
			pf_s->_prependPoint = new_start;
			new_start = new Common::Point(77, 107);
		}
	}

	// Merge start and end points into polygon set
	pf_s->vertex_start = merge_point(pf_s, *new_start);
	pf_s->vertex_end = merge_point(pf_s, *new_end);

	delete new_start;
	delete new_end;

	// Allocate and build vertex index
	pf_s->vertex_index = (Vertex**)malloc(sizeof(Vertex *) * (count + 2));

	count = 0;

	for (PolygonList::iterator it = pf_s->polygons.begin(); it != pf_s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices) {
			pf_s->vertex_index[count++] = vertex;
		}
	}

	pf_s->vertices = count;

	return pf_s;
}

/**
 * Computes a shortest path from vertex_start to vertex_end. The caller can
 * construct the resulting path by following the path_prev links from
 * vertex_end back to vertex_start. If no path exists vertex_end->path_prev
 * will be NULL
 * Parameters: (PathfindingState *) s: The pathfinding state
 */
static void AStar(PathfindingState *s) {
	// Vertices of which the shortest path is known
	VertexList closedSet;

	// The remaining vertices
	VertexList openSet;

	openSet.push_front(s->vertex_start);
	s->vertex_start->costG = 0;
	s->vertex_start->costF = (uint32)sqrt((float)s->vertex_start->v.sqrDist(s->vertex_end->v));

	while (!openSet.empty()) {
		// Find vertex in open set with lowest F cost
		VertexList::iterator vertex_min_it = openSet.end();
		Vertex *vertex_min = 0;
		uint32 min = HUGE_DISTANCE;

		for (VertexList::iterator it = openSet.begin(); it != openSet.end(); ++it) {
			Vertex *vertex = *it;
			if (vertex->costF < min) {
				vertex_min_it = it;
				vertex_min = *vertex_min_it;
				min = vertex->costF;
			}
		}

		assert(vertex_min != 0);	// the vertex cost should never be bigger than HUGE_DISTANCE

		// Check if we are done
		if (vertex_min == s->vertex_end)
			break;

		// Move vertex from set open to set closed
		closedSet.push_front(vertex_min);
		openSet.erase(vertex_min_it);

		VertexList *visVerts = visible_vertices(s, vertex_min);

		for (VertexList::iterator it = visVerts->begin(); it != visVerts->end(); ++it) {
			uint32 new_dist;
			Vertex *vertex = *it;

			if (closedSet.contains(vertex))
				continue;

			if (!openSet.contains(vertex))
				openSet.push_front(vertex);

			new_dist = vertex_min->costG + (uint32)sqrt((float)vertex_min->v.sqrDist(vertex->v));

			// When travelling to a vertex on the screen edge, we
			// add a penalty score to make this path less appealing.
			// NOTE: If an obstacle has only one vertex on a screen edge,
			// later SSCI pathfinders will treat that vertex like any
			// other, while we apply a penalty to paths traversing it.
			// This difference might lead to problems, but none are
			// known at the time of writing.
			if (s->pointOnScreenBorder(vertex->v))
				new_dist += 10000;

			if (new_dist < vertex->costG) {
				vertex->costG = new_dist;
				vertex->costF = vertex->costG + (uint32)sqrt((float)vertex->v.sqrDist(s->vertex_end->v));
				vertex->path_prev = vertex_min;
			}
		}

		delete visVerts;
	}

	if (openSet.empty())
		debugC(kDebugLevelAvoidPath, "AvoidPath: End point (%i, %i) is unreachable", s->vertex_end->v.x, s->vertex_end->v.y);
}

static reg_t allocateOutputArray(SegManager *segMan, int size) {
	reg_t addr;

#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2) {
		SciArray<reg_t> *array = segMan->allocateArray(&addr);
		assert(array);
		array->setType(0);
		array->setSize(size * 2);
		return addr;
	}
#endif

	segMan->allocDynmem(POLY_POINT_SIZE * size, AVOIDPATH_DYNMEM_STRING, &addr);
	return addr;
}

/**
 * Stores the final path in newly allocated dynmem
 * Parameters: (PathfindingState *) p: The pathfinding state
 *             (EngineState *) s: The game state
 * Returns   : (reg_t) Pointer to dynmem containing path
 */
static reg_t output_path(PathfindingState *p, EngineState *s) {
	int path_len = 0;
	reg_t output;
	Vertex *vertex = p->vertex_end;
	int unreachable = vertex->path_prev == NULL;

	if (!unreachable) {
		while (vertex) {
			// Compute path length
			path_len++;
			vertex = vertex->path_prev;
		}
	}

	// Allocate memory for path, plus 3 extra for appended point, prepended point and sentinel
	output = allocateOutputArray(s->_segMan, path_len + 3);
	SegmentRef arrayRef = s->_segMan->dereference(output);
	assert(arrayRef.isValid() && !arrayRef.skipByte);

	if (unreachable) {
		// If pathfinding failed we only return the path up to vertex_start

		if (p->_prependPoint)
			writePoint(arrayRef, 0, *p->_prependPoint);
		else
			writePoint(arrayRef, 0, p->vertex_start->v);

		writePoint(arrayRef, 1, p->vertex_start->v);
		writePoint(arrayRef, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

		return output;
	}

	int offset = 0;

	if (p->_prependPoint)
		writePoint(arrayRef, offset++, *p->_prependPoint);

	vertex = p->vertex_end;
	for (int i = path_len - 1; i >= 0; i--) {
		writePoint(arrayRef, offset + i, vertex->v);
		vertex = vertex->path_prev;
	}
	offset += path_len;

	if (p->_appendPoint)
		writePoint(arrayRef, offset++, *p->_appendPoint);

	// Sentinel
	writePoint(arrayRef, offset, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

	if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
		debug("\nReturning path:");

		SegmentRef outputList = s->_segMan->dereference(output);
		if (!outputList.isValid() || outputList.skipByte) {
			warning("output_path: Polygon data pointer is invalid, skipping polygon");
			return output;
		}

		for (int i = 0; i < offset; i++) {
			Common::Point pt = readPoint(outputList, i);
			debugN(-1, " (%i, %i)", pt.x, pt.y);
		}
		debug(";\n");
	}

	return output;
}

reg_t kAvoidPath(EngineState *s, int argc, reg_t *argv) {
	Common::Point start = Common::Point(argv[0].toSint16(), argv[1].toSint16());

	switch (argc) {

	case 3 : {
		reg_t retval;
		Polygon *polygon = convert_polygon(s, argv[2]);

		if (!polygon)
			return NULL_REG;

		// Override polygon type to prevent inverted result for contained access polygons
		polygon->type = POLY_BARRED_ACCESS;

		retval = make_reg(0, contained(start, polygon) != CONT_OUTSIDE);
		delete polygon;
		return retval;
	}
	case 6 :
	case 7 :
	case 8 : {
		Common::Point end = Common::Point(argv[2].toSint16(), argv[3].toSint16());
		reg_t poly_list, output;
		int width, height, opt = 1;

		if (getSciVersion() >= SCI_VERSION_2) {
			if (argc < 7)
				error("[avoidpath] Not enough arguments");

			poly_list = (!argv[4].isNull() ? readSelector(s->_segMan, argv[4], SELECTOR(elements)) : NULL_REG);
			width = argv[5].toUint16();
			height = argv[6].toUint16();
			if (argc > 7)
				opt = argv[7].toUint16();
		} else {
			// SCI1.1 and older games always ran with an internal resolution of 320x200
			poly_list = argv[4];
			width = 320;
			height = 190;
			if (argc > 6)
				opt = argv[6].toUint16();
		}

		if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
			debug("[avoidpath] Pathfinding input:");
			draw_point(s, start, 1, width, height);
			draw_point(s, end, 0, width, height);

			if (poly_list.segment) {
				print_input(s, poly_list, start, end, opt);
				draw_input(s, poly_list, start, end, opt, width, height);
			}

			// Update the whole screen
			g_sci->_gfxScreen->copyToScreen();
			g_system->updateScreen();
			if (!g_sci->_gfxPaint16)
				g_system->delayMillis(2500);
		}

		PathfindingState *p = convert_polygon_set(s, poly_list, start, end, width, height, opt);

		if (!p) {
			warning("[avoidpath] Error: pathfinding failed for following input:\n");
			print_input(s, poly_list, start, end, opt);
			warning("[avoidpath] Returning direct path from start point to end point\n");
			output = allocateOutputArray(s->_segMan, 3);
			SegmentRef arrayRef = s->_segMan->dereference(output);
			assert(arrayRef.isValid() && !arrayRef.skipByte);

			writePoint(arrayRef, 0, start);
			writePoint(arrayRef, 1, end);
			writePoint(arrayRef, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

			return output;
		}

		// Apply Dijkstra
		AStar(p);

		output = output_path(p, s);
		delete p;

		// Memory is freed by explicit calls to Memory
		return output;
	}

	default:
		warning("Unknown AvoidPath subfunction %d", argc);
		return NULL_REG;
		break;
	}
}

static bool PointInRect(const Common::Point &point, int16 rectX1, int16 rectY1, int16 rectX2, int16 rectY2) {
	int16 top = MIN<int16>(rectY1, rectY2);
	int16 left = MIN<int16>(rectX1, rectX2);
	int16 bottom = MAX<int16>(rectY1, rectY2) + 1;
	int16 right = MAX<int16>(rectX1, rectX2) + 1;

	Common::Rect rect = Common::Rect(left, top, right, bottom);
	// Add a one pixel margin of error
	rect.grow(1);

	return rect.contains(point);
}

reg_t kIntersections(EngineState *s, int argc, reg_t *argv) {
	// This function computes intersection points for the "freeway pathing" in MUMG CD.
	int32 qSourceX = argv[0].toSint16();
	int32 qSourceY = argv[1].toSint16();
	int32 qDestX = argv[2].toSint16();
	int32 qDestY = argv[3].toSint16();
	uint16 startIndex = argv[5].toUint16();
	uint16 endIndex = argv[6].toUint16();
	uint16 stepSize = argv[7].toUint16();
	bool backtrack = argv[9].toUint16();

	const int32 kVertical = 0x7fffffff;

	uint16 curIndex = startIndex;
	reg_t *inpBuf = s->_segMan->derefRegPtr(argv[4], endIndex + 2);

	if (!inpBuf) {
		warning("Intersections: input buffer invalid");
		return NULL_REG;
	}

	reg_t *outBuf = s->_segMan->derefRegPtr(argv[8], (endIndex - startIndex + 2) / stepSize * 3);

	if (!outBuf) {
		warning("Intersections: output buffer invalid");
		return NULL_REG;
	}

	// Slope and y-intercept of the query line in centipixels
	int32 qIntercept;
	int32 qSlope;

	if (qSourceX != qDestX) {
		// Compute slope of the line and round to nearest centipixel
		qSlope = (1000 * (qSourceY - qDestY)) / (qSourceX - qDestX);

		if (qSlope >= 0)
			qSlope += 5;
		else
			qSlope -= 5;

		qSlope /= 10;

		// Compute y-intercept in centipixels
		qIntercept = (100 * qDestY) - (qSlope * qDestX);

		if (backtrack) {
			// If backtrack is set we extend the line from dest to source
			// until we hit a screen edge and place the source point there

			// First we try to place the source point on the left or right
			// screen edge
			if (qSourceX >= qDestX)
				qSourceX = 319;
			else
				qSourceX = 0;

			// Compute the y-coordinate
			qSourceY = ((qSlope * qSourceX) + qIntercept) / 100;

			// If the y-coordinate is off-screen, the point we want is on the
			// top or bottom edge of the screen instead
			if (qSourceY < 0 || qSourceY > 189) {
				if (qSourceY < 0)
					qSourceY = 0;
				else if (qSourceY > 189)
					qSourceY = 189;

				// Compute the x-coordinate
				qSourceX = (((((qSourceY * 100) - qIntercept) * 10) / qSlope) + 5) / 10;
			}
		}
	} else {
		// The query line is vertical
		qIntercept = qSlope = kVertical;

		if (backtrack) {
			// If backtrack is set, extend to screen edge
			if (qSourceY >= qDestY)
				qSourceY = 189;
			else
				qSourceY = 0;
		}
	}

	int32 pSourceX = inpBuf[curIndex].toSint16();
	int32 pSourceY = inpBuf[curIndex + 1].toSint16();

	// If it's a polygon, we include the first point again at the end
	int16 doneIndex;
	if (pSourceX & (1 << 13))
		doneIndex = startIndex;
	else
		doneIndex = endIndex;

	pSourceX &= 0x1ff;

	debugCN(kDebugLevelAvoidPath, "%s: (%i, %i)[%i]",
		(doneIndex == startIndex ? "Polygon" : "Polyline"), pSourceX, pSourceY, curIndex);

	curIndex += stepSize;
	uint16 outCount = 0;

	while (1) {
		int32 pDestX = inpBuf[curIndex].toSint16() & 0x1ff;
		int32 pDestY = inpBuf[curIndex + 1].toSint16();

		if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
			draw_line(s, Common::Point(pSourceX, pSourceY),
				Common::Point(pDestX, pDestY), 2, 320, 190);
			debugN(-1, " (%i, %i)[%i]", pDestX, pDestY, curIndex);
		}

		// Slope and y-intercept of the polygon edge in centipixels
		int32 pIntercept;
		int32 pSlope;

		if (pSourceX != pDestX) {
			// Compute slope and y-intercept (as above)
			pSlope = ((pDestY - pSourceY) * 1000) / (pDestX - pSourceX);

			if (pSlope >= 0)
				pSlope += 5;
			else
				pSlope -= 5;

			pSlope /= 10;

			pIntercept = (pDestY * 100) - (pSlope * pDestX);
		} else {
			// Polygon edge is vertical
			pSlope = pIntercept = kVertical;
		}

		bool foundIntersection = true;
		int32 intersectionX = 0;
		int32 intersectionY = 0;

		if (qSlope == pSlope) {
			// If the lines overlap, we test the source and destination points
			// against the poly segment
			if ((pIntercept == qIntercept) && (PointInRect(Common::Point(pSourceX, pSourceY), qSourceX, qSourceY, qDestX, qDestY))) {
				intersectionX = pSourceX * 100;
				intersectionY = pSourceY * 100;
			} else if ((pIntercept == qIntercept) && PointInRect(Common::Point(qDestX, qDestY), pSourceX, pSourceY, pDestX, pDestY)) {
				intersectionX = qDestX * 100;
				intersectionY = qDestY * 100;
			} else {
				// Lines are parallel or segments don't overlap, no intersection
				foundIntersection = false;
			}
		} else {
			// Lines are not parallel
			if (qSlope == kVertical) {
				// Query segment is vertical, polygon segment is not vertical
				intersectionX = qSourceX * 100;
				intersectionY = pSlope * qSourceX + pIntercept;
			} else if (pSlope == kVertical) {
				// Polygon segment is vertical, query segment is not vertical
				intersectionX = pDestX * 100;
				intersectionY = qSlope * pDestX + qIntercept;
			} else {
				// Neither line is vertical
				intersectionX = ((pIntercept - qIntercept) * 100) / (qSlope - pSlope);
				intersectionY = ((intersectionX * pSlope) + (pIntercept * 100)) / 100;
			}
		}

		if (foundIntersection) {
			// Round back to pixels
			intersectionX = (intersectionX + 50) / 100;
			intersectionY = (intersectionY + 50) / 100;

			// If intersection point lies on both the query line segment and the poly
			// line segment, add it to the output
			if (((PointInRect(Common::Point(intersectionX, intersectionY), pSourceX, pSourceY, pDestX, pDestY))
				&& PointInRect(Common::Point(intersectionX, intersectionY), qSourceX, qSourceY, qDestX, qDestY))) {
				outBuf[outCount * 3] = make_reg(0, intersectionX);
				outBuf[outCount * 3 + 1] = make_reg(0, intersectionY);
				outBuf[outCount * 3 + 2] = make_reg(0, curIndex);
				outCount++;
			}
		}

		if (curIndex == doneIndex) {
			// End of polyline/polygon reached
			if (DebugMan.isDebugChannelEnabled(kDebugLevelAvoidPath)) {
				debug(";");
				debugN(-1, "Found %i intersections", outCount);

				if (outCount) {
					debugN(-1, ":");
					for (int i = 0; i < outCount; i++) {
						Common::Point p = Common::Point(outBuf[i * 3].toSint16(), outBuf[i * 3 + 1].toSint16());
						draw_point(s, p, 0, 320, 190);
						debugN(-1, " (%i, %i)[%i]", p.x, p.y, outBuf[i * 3 + 2].toSint16());
					}
				}

				debug(";");

				g_sci->_gfxScreen->copyToScreen();
				g_system->updateScreen();
			}

			return make_reg(0, outCount);
		}

		if (curIndex != endIndex) {
			// Go to next point in polyline/polygon
			curIndex += stepSize;
		} else {
			// Wrap-around for polygon case
			curIndex = startIndex;
		}

		// Current destination point is source for the next line segment
		pSourceX = pDestX;
		pSourceY = pDestY;
	}
}

/**
 * This is a quite rare kernel function. An example of when it's called
 * is in QFG1VGA, after killing any monster.
 */
reg_t kMergePoly(EngineState *s, int argc, reg_t *argv) {
#if 0
	// 3 parameters: raw polygon data, polygon list, list size
	reg_t polygonData = argv[0];
	List *list = s->_segMan->lookupList(argv[1]);
	Node *node = s->_segMan->lookupNode(list->first);
	// List size is not needed

	Polygon *polygon;
	int count = 0;

	while (node) {
		polygon = convert_polygon(s, node->value);

		if (polygon) {
			count += readSelectorValue(s->_segMan, node->value, SELECTOR(size));
		}

		node = s->_segMan->lookupNode(node->succ);
	}
#endif

	// TODO: actually merge the polygon. We return an empty polygon for now.
	// In QFG1VGA, you can walk over enemy bodies after killing them, since
	// this is a stub.
	reg_t output = allocateOutputArray(s->_segMan, 1);
	SegmentRef arrayRef = s->_segMan->dereference(output);
	writePoint(arrayRef, 0, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));
	warning("Stub: kMergePoly");
	return output;
}

#ifdef ENABLE_SCI32

reg_t kInPolygon(EngineState *s, int argc, reg_t *argv) {
	// kAvoidPath already implements this
	return kAvoidPath(s, argc, argv);
}

#endif

} // End of namespace Sci
