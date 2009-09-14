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
 * $URL$
 * $Id$
 *
 */

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/gfx/gfx_widgets.h"
#include "sci/gfx/gfx_state_internal.h"	// required for GfxPort, GfxContainer

#include "common/list.h"

namespace Sci {

#define POLY_LAST_POINT 0x7777
#define POLY_POINT_SIZE 4
//#define DEBUG_AVOIDPATH	//enable for avoidpath debugging

static void POLY_GET_POINT(const byte *p, int i, Common::Point &pt) {
	pt.x = (int16)READ_LE_UINT16((p) + (i) * POLY_POINT_SIZE);
	pt.y = (int16)READ_LE_UINT16((p) + (i) * POLY_POINT_SIZE + 2);
}

static void POLY_SET_POINT(byte *p, int i, const Common::Point &pt) {
	WRITE_LE_UINT16((p) + (i) * POLY_POINT_SIZE, pt.x);
	WRITE_LE_UINT16((p) + (i) * POLY_POINT_SIZE + 2, pt.y);
}

static void POLY_GET_POINT_REG_T(const reg_t *p, int i, Common::Point &pt) {
	pt.x = (p)[(i) * 2].toUint16();
	pt.y = (p)[(i) * 2 + 1].toUint16();
}

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

	// Distance from starting vertex
	uint32 dist;

	// Previous vertex in shortest path
	Vertex *path_prev;

public:
	Vertex(const Common::Point &p) : v(p) {
		dist = HUGE_DISTANCE;
		path_prev = NULL;
	}
};

typedef Common::List<Vertex *> VertexList;

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

	PathfindingState() {
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
};


static Vertex *s_vertex_cur;	// FIXME: Avoid non-const global vars

// FIXME: Temporary hack to deal with points in reg_ts
static bool polygon_is_reg_t(const byte *list, int size) {
	// Check the first three reg_ts
	for (int i = 0; i < (size < 3 ? size : 3); i++)
		if ((((reg_t *) list) + i)->segment)
			// Non-zero segment, cannot be reg_ts
			return false;

	// First three segments were zero, assume reg_ts
	return true;
}

static Common::Point read_point(const byte *list, int is_reg_t, int offset) {
	Common::Point point;

	if (!is_reg_t) {
		POLY_GET_POINT(list, offset, point);
	} else {
		POLY_GET_POINT_REG_T((reg_t *)list, offset, point);
	}

	return point;
}

/**
 * Checks whether two polygons are equal
 */
static bool polygons_equal(SegManager *segMan, reg_t p1, reg_t p2) {
	// Check for same type
	if (GET_SEL32(p1, type).toUint16() != GET_SEL32(p2, type).toUint16())
		return false;

	int size = GET_SEL32(p1, size).toUint16();

	// Check for same number of points
	if (size != GET_SEL32(p2, size).toUint16())
		return false;

	const byte *p1_points = segMan->derefBulkPtr(GET_SEL32(p1, points), size * POLY_POINT_SIZE);
	const byte *p2_points = segMan->derefBulkPtr(GET_SEL32(p2, points), size * POLY_POINT_SIZE);
	bool p1_is_reg_t = polygon_is_reg_t(p1_points, size);
	bool p2_is_reg_t = polygon_is_reg_t(p2_points, size);

	// Check for the same points
	for (int i = 0; i < size; i++) {
		if (read_point(p1_points, p1_is_reg_t, i) != read_point(p2_points, p2_is_reg_t, i))
			return false;
	}

	return true;
}

#ifdef DEBUG_AVOIDPATH

static void draw_line(EngineState *s, Common::Point p1, Common::Point p2, int type) {
	// Colors for polygon debugging.
	// Green: Total access
	// Red : Barred access
	// Blue: Near-point access
	// Yellow: Contained access
	int poly_colors[][3] = {{0, 255, 0}, {0, 0, 255}, {255, 0, 0}, {255, 255, 0}};
	gfx_color_t col;
	GfxList *decorations = s->picture_port->_decorations;
	GfxPrimitive *line;

	col.visual = PaletteEntry(poly_colors[type][0], poly_colors[type][1], poly_colors[type][2]);
	col.alpha = 0;
	col.priority = -1;
	col.control = 0;
	col.mask = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;

	p1.y += 10;
	p2.y += 10;

	line = gfxw_new_line(p1, p2, col, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
	decorations->add((GfxContainer *)decorations, (GfxWidget *)line);
}

static void draw_point(EngineState *s, Common::Point p, int start) {
	// Colors for starting and end point
	// Green: End point
	// Blue: Starting point
	int point_colors[][3] = {{0, 255, 0}, {0, 0, 255}};
	gfx_color_t col;
	GfxList *decorations = s->picture_port->_decorations;
	GfxBox *box;

	col.visual = PaletteEntry(point_colors[start][0], point_colors[start][1], point_colors[start][2]);
	col.alpha = 0;
	col.priority = -1;
	col.control = 0;
	col.mask = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;

	box = gfxw_new_box(s->gfx_state, gfx_rect(p.x - 1, p.y - 1 + 10, 3, 3), col, col, GFX_BOX_SHADE_FLAT);
	decorations->add((GfxContainer *)decorations, (GfxWidget *)box);
}

static void draw_polygon(EngineState *s, reg_t polygon) {
	reg_t points = GET_SEL32(polygon, points);
	int size = GET_SEL32(polygon, size).toUint16();
	int type = GET_SEL32(polygon, type).toUint16();
	Common::Point first, prev;
	const byte *list = s->segMan->derefBulkPtr(points, size * POLY_POINT_SIZE);
	int is_reg_t = polygon_is_reg_t(list, size);
	int i;

	prev = first = read_point(list, is_reg_t, 0);

	for (i = 1; i < size; i++) {
		Common::Point point = read_point(list, is_reg_t, i);
		draw_line(s, prev, point, type);
		prev = point;
	}

	draw_line(s, prev, first, type % 3);
}

static void draw_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	List *list;
	Node *node;

	draw_point(s, start, 1);
	draw_point(s, end, 0);

	if (!poly_list.segment)
		return;

	list = lookup_list(s, poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	node = lookup_node(s, list->first);

	while (node) {
		draw_polygon(s, node->value);
		node = lookup_node(s, node->succ);
	}
}

#endif	// DEBUG_AVOIDPATH

static void print_polygon(SegManager *segMan, reg_t polygon) {
	reg_t points = GET_SEL32(polygon, points);
	int size = GET_SEL32(polygon, size).toUint16();
	int type = GET_SEL32(polygon, type).toUint16();
	int i;
	const byte *point_array = segMan->derefBulkPtr(points, size * POLY_POINT_SIZE);
	int is_reg_t = polygon_is_reg_t(point_array, size);
	Common::Point point;

	printf("%i:", type);

	for (i = 0; i < size; i++) {
		point = read_point(point_array, is_reg_t, i);
		printf(" (%i, %i)", point.x, point.y);
	}

	point = read_point(point_array, is_reg_t, 0);
	printf(" (%i, %i);\n", point.x, point.y);
}

static void print_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	List *list;
	Node *node;

	printf("Start point: (%i, %i)\n", start.x, start.y);
	printf("End point: (%i, %i)\n", end.x, end.y);
	printf("Optimization level: %i\n", opt);

	if (!poly_list.segment)
		return;

	list = lookup_list(s, poly_list);

	if (!list) {
		warning("[avoidpath] Could not obtain polygon list");
		return;
	}

	printf("Polygons:\n");
	node = lookup_node(s, list->first);

	while (node) {
		print_polygon(s->segMan, node->value);
		node = lookup_node(s, node->succ);
	}
}

static int area(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	// Computes the area of a triangle
	// Parameters: (const Common::Point &) a, b, c: The points of the triangle
	// Returns   : (int) The area multiplied by two
	return (b.x - a.x) * (a.y - c.y) - (c.x - a.x) * (a.y - b.y);
}

static bool left(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	// Determines whether or not a point is to the left of a directed line
	// Parameters: (const Common::Point &) a, b: The directed line (a, b)
	//             (const Common::Point &) c: The query point
	// Returns   : (int) true if c is to the left of (a, b), false otherwise
	return area(a, b, c) > 0;
}

static bool left_on(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	// Determines whether or not a point is to the left of or collinear with a
	// directed line
	// Parameters: (const Common::Point &) a, b: The directed line (a, b)
	//             (const Common::Point &) c: The query point
	// Returns   : (int) true if c is to the left of or collinear with (a, b), false
	//                   otherwise
	return area(a, b, c) >= 0;
}

static bool collinear(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	// Determines whether or not three points are collinear
	// Parameters: (const Common::Point &) a, b, c: The three points
	// Returns   : (int) true if a, b, and c are collinear, false otherwise
	return area(a, b, c) == 0;
}

static bool between(const Common::Point &a, const Common::Point &b, const Common::Point &c) {
	// Determines whether or not a point lies on a line segment
	// Parameters: (const Common::Point &) a, b: The line segment (a, b)
	//             (const Common::Point &) c: The query point
	// Returns   : (int) true if c lies on (a, b), false otherwise
	if (!collinear(a, b, c))
		return false;

	// Assumes a != b.
	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

static bool intersect_proper(const Common::Point &a, const Common::Point &b, const Common::Point &c, const Common::Point &d) {
	// Determines whether or not two line segments properly intersect
	// Parameters: (const Common::Point &) a, b: The line segment (a, b)
	//             (const Common::Point &) c, d: The line segment (c, d)
	// Returns   : (int) true if (a, b) properly intersects (c, d), false otherwise
	int ab = (left(a, b, c) && left(b, a, d)) || (left(a, b, d) && left(b, a, c));
	int cd = (left(c, d, a) && left(d, c, b)) || (left(c, d, b) && left(d, c, a));

	return ab && cd;
}

static bool intersect(const Common::Point &a, const Common::Point &b, const Common::Point &c, const Common::Point &d) {
	// Determines whether or not two line segments intersect
	// Parameters: (const Common::Point &) a, b: The line segment (a, b)
	//             (const Common::Point &) c, d: The line segment (c, d)
	// Returns   : (int) true if (a, b) intersects (c, d), false otherwise
	if (intersect_proper(a, b, c, d))
		return true;

	return between(a, b, c) || between(a, b, d) || between(c, d, a) || between(c, d, b);
}

static int contained(const Common::Point &p, Polygon *polygon) {
	// Polygon containment test
	// Parameters: (const Common::Point &) p: The point
	//             (Polygon *) polygon: The polygon
	// Returns   : (int) CONT_INSIDE if p is strictly contained in polygon,
	//                   CONT_ON_EDGE if p lies on an edge of polygon,
	//                   CONT_OUTSIDE otherwise
	// Number of ray crossing left and right
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

static int polygon_area(Polygon *polygon) {
	// Computes polygon area
	// Parameters: (Polygon *) polygon: The polygon
	// Returns   : (int) The area multiplied by two
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

static void fix_vertex_order(Polygon *polygon) {
	// Fixes the vertex order of a polygon if incorrect. Contained access
	// polygons should have their vertices ordered clockwise, all other types
	// anti-clockwise
	// Parameters: (Polygon *) polygon: The polygon
	int area = polygon_area(polygon);

	// When the polygon area is positive the vertices are ordered
	// anti-clockwise. When the area is negative the vertices are ordered
	// clockwise
	if (((area > 0) && (polygon->type == POLY_CONTAINED_ACCESS))
	        || ((area < 0) && (polygon->type != POLY_CONTAINED_ACCESS))) {

		polygon->vertices.reverse();
	}
}

static int vertex_compare(const void *a, const void *b) {
	// Compares two vertices by angle (first) and distance (second) in relation
	// to s_vertex_cur. The angle is relative to the horizontal line extending
	// right from s_vertex_cur, and increases clockwise
	// Parameters: (const void *) a, b: The vertices
	// Returns   : (int) -1 if a is smaller than b, 1 if a is larger than b, and
	//                   0 if a and b are equal
	const Common::Point &p0 = s_vertex_cur->v;
	const Common::Point &p1 = (*(Vertex **) a)->v;
	const Common::Point &p2 = (*(Vertex **) b)->v;

	if (p1 == p2)
		return 0;

	// Points above p0 have larger angle than points below p0
	if ((p1.y < p0.y) && (p2.y >= p0.y))
		return 1;

	if ((p2.y < p0.y) && (p1.y >= p0.y))
		return -1;

	// Handle case where all points have the same y coordinate
	if ((p0.y == p1.y) && (p0.y == p2.y)) {
		// Points left of p0 have larger angle than points right of p0
		if ((p1.x < p0.x) && (p2.x >= p0.x))
			return 1;
		if ((p1.x >= p0.x) && (p2.x < p0.x))
			return -1;
	}

	if (collinear(p0, p1, p2)) {
		// At this point collinear points must have the same angle,
		// so compare distance to p0
		if (abs(p1.x - p0.x) < abs(p2.x - p0.x))
			return -1;
		if (abs(p1.y - p0.y) < abs(p2.y - p0.y))
			return -1;

		return 1;
	}

	// If p2 is left of the directed line (p0, p1) then p1 has greater angle
	if (left(p0, p1, p2))
		return 1;

	return -1;
}

static void clockwise(const Vertex *vertex_cur, const Vertex *v, const Common::Point *&p1, const Common::Point *&p2) {
	// Orders the points of an edge clockwise around vertex_cur. If all three
	// points are collinear the original order is used
	// Parameters: (const Vertex *) v: The first vertex of the edge
	// Returns   : (void)
	//             (const Common::Point *&) p1: The first point in clockwise order
	//             (const Common::Point *&) p2: The second point in clockwise order
	Vertex *w = CLIST_NEXT(v);

	if (left_on(vertex_cur->v, w->v, v->v)) {
		p1 = &v->v;
		p2 = &w->v;
	} else {
		p1 = &w->v;
		p2 = &v->v;
	}
}

/**
 * Compares two edges that are intersected by the sweeping line by distance from vertex_cur
 * @param a				the first edge
 * @param b				the second edge
 * @return true if a is closer to vertex_cur than b, false otherwise
 */
static bool edgeIsCloser(const Vertex *vertex_cur, const Vertex *a, const Vertex *b) {
	const Common::Point *v1, *v2, *w1, *w2;

	// Check for comparison of the same edge
	if (a == b)
		return false;

	// We can assume that the sweeping line intersects both edges and
	// that the edges do not properly intersect

	// Order vertices clockwise so we know vertex_cur is to the right of
	// directed edges (v1, v2) and (w1, w2)
	clockwise(vertex_cur, a, v1, v2);
	clockwise(vertex_cur, b, w1, w2);

	// At this point we know that one edge must lie entirely to one side
	// of the other, as the edges are not collinear and cannot intersect
	// other than possibly sharing a vertex.

	return ((left_on(*v1, *v2, *w1) && left_on(*v1, *v2, *w2)) || (left_on(*w2, *w1, *v1) && left_on(*w2, *w1, *v2)));
}

static int inside(const Common::Point &p, Vertex *vertex) {
	// Determines whether or not a line from a point to a vertex intersects the
	// interior of the polygon, locally at that vertex
	// Parameters: (Common::Point) p: The point
	//             (Vertex *) vertex: The vertex
	// Returns   : (int) 1 if the line (p, vertex->v) intersects the interior of
	//                   the polygon, locally at the vertex. 0 otherwise
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
 * Determines whether or not a vertex is visible from vertex_cur.
 * @param vertex_cur	the base vertex
 * @param vertex		the vertex
 * @param vertex_prev	the previous vertex in the sort order, or NULL
 * @param visible		true if vertex_prev is visible, false otherwise
 * @param intersected	the list of edges intersected by the sweeping line
 * @return true if vertex is visible from vertex_cur, false otherwise
 */
static bool visible(Vertex *vertex_cur, Vertex *vertex, Vertex *vertex_prev, bool visible, const VertexList &intersected) {
	const Common::Point &p = vertex_cur->v;
	const Common::Point &w = vertex->v;

	// Check if sweeping line intersects the interior of the polygon
	// locally at vertex
	if (inside(p, vertex))
		return false;

	// If vertex_prev is on the sweeping line, then vertex is invisible
	// if vertex_prev is invisible
	if (vertex_prev && !visible && between(p, w, vertex_prev->v))
		return false;

	if (intersected.empty()) {
		// No intersected edges
		return true;
	}

	// Look for the intersected edge that is closest to vertex_cur
	VertexList::const_iterator it = intersected.begin();
	const Vertex *edge = *it++;

	for (; it != intersected.end(); ++it) {
		if (edgeIsCloser(vertex_cur, *it, edge))
			edge = *it;
	}

	const Common::Point *p1, *p2;

	// Check for intersection with sweeping line before vertex
	clockwise(vertex_cur, edge, p1, p2);
	if (left(*p2, *p1, p) && left(*p1, *p2, w))
		return false;

	return true;
}

/**
 * Returns a list of all vertices that are visible from a particular vertex.
 * @param s				the pathfinding state
 * @param vertex_cur	the vertex
 * @return list of vertices that are visible from vert
 */
static VertexList *visible_vertices(PathfindingState *s, Vertex *vertex_cur) {
	// List of edges intersected by the sweeping line
	VertexList intersected;
	VertexList *visVerts = new VertexList();
	const Common::Point &p = vertex_cur->v;

	// Sort vertices by angle (first) and distance (second)
	s_vertex_cur = vertex_cur;
	qsort(s->vertex_index, s->vertices, sizeof(Vertex *), vertex_compare);

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		Polygon *polygon = *it;
		Vertex *vertex;
		vertex = polygon->vertices.first();

		// Check that there is more than one vertex.
		if (VERTEX_HAS_EDGES(vertex)) {
			CLIST_FOREACH(vertex, &polygon->vertices) {
				const Common::Point *high, *low;

				// Add edges that intersect the initial position of the sweeping line
				clockwise(vertex_cur, vertex, high, low);

				if ((high->y < p.y) && (low->y >= p.y) && (*low != p))
					intersected.push_front(vertex);
			}
		}
	}

	int is_visible = 1;

	// The first vertex will be s_vertex_cur, so we skip it
	for (int i = 1; i < s->vertices; i++) {
		Vertex *v1;

		// Compute visibility of vertex_index[i]
		is_visible = visible(s_vertex_cur, s->vertex_index[i], s->vertex_index[i - 1], is_visible, intersected);

		// Update visibility matrix
		if (is_visible)
			visVerts->push_front(s->vertex_index[i]);

		// Delete anti-clockwise edges from list
		v1 = CLIST_PREV(s->vertex_index[i]);
		if (left(p, s->vertex_index[i]->v, v1->v))
			intersected.remove(v1);

		v1 = CLIST_NEXT(s->vertex_index[i]);
		if (left(p, s->vertex_index[i]->v, v1->v))
			intersected.remove(s->vertex_index[i]);

		// Add clockwise edges of collinear vertices when sweeping line moves
		if ((i < s->vertices - 1) && !collinear(p, s->vertex_index[i]->v, s->vertex_index[i + 1]->v)) {
			int j;
			for (j = i; (j >= 1) && collinear(p, s->vertex_index[i]->v, s->vertex_index[j]->v); j--) {
				v1 = CLIST_PREV(s->vertex_index[j]);
				if (left(s->vertex_index[j]->v, p, v1->v))
					intersected.push_front(v1);

				v1 = CLIST_NEXT(s->vertex_index[j]);
				if (left(s->vertex_index[j]->v, p, v1->v))
					intersected.push_front(s->vertex_index[j]);
			}
		}
	}

	return visVerts;
}

#if 0
static bool point_on_screen_border(const Common::Point &p) {
	// Determines if a point lies on the screen border
	// Parameters: (const Common::Point &) p: The point
	// Returns   : (int) true if p lies on the screen border, false otherwise
	// FIXME get dimensions from somewhere?
	return (p.x == 0) || (p.x == 319) || (p.y == 0) || (p.y == 189);
}
#endif

static bool edge_on_screen_border(const Common::Point &p, const Common::Point &q) {
	// Determines if an edge lies on the screen border
	// Parameters: (const Common::Point &) p, q: The edge (p, q)
	// Returns   : (int) true if (p, q) lies on the screen border, false otherwise
	// FIXME get dimensions from somewhere?
	return ((p.x == 0 && q.x == 0) || (p.x == 319 && q.x == 319) || (p.y == 0 && q.y == 0) || (p.y == 189 && q.y == 189));
}

static int find_free_point(FloatPoint f, Polygon *polygon, Common::Point *ret) {
	// Searches for a nearby point that is not contained in a polygon
	// Parameters: (FloatPoint) f: The pointf to search nearby
	//             (Polygon *) polygon: The polygon
	// Returns   : (int) PF_OK on success, PF_FATAL otherwise
	//             (Common::Point) *ret: The non-contained point on success
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

static int near_point(const Common::Point &p, Polygon *polygon, Common::Point *ret) {
	// Computes the near point of a point contained in a polygon
	// Parameters: (const Common::Point &) p: The point
	//             (Polygon *) polygon: The polygon
	// Returns   : (int) PF_OK on success, PF_FATAL otherwise
	//             (Common::Point) *ret: The near point of p in polygon on success
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
		if ((polygon->type != POLY_CONTAINED_ACCESS) && (edge_on_screen_border(p1, p2)))
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

static int intersection(const Common::Point &a, const Common::Point &b, Vertex *vertex, FloatPoint *ret) {
	// Computes the intersection point of a line segment and an edge (not
	// including the vertices themselves)
	// Parameters: (const Common::Point &) a, b: The line segment (a, b)
	//             (Vertex *) vertex: The first vertex of the edge
	// Returns   : (int) FP_OK on success, PF_ERROR otherwise
	//             (FloatPoint) *ret: The intersection point
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

static int nearest_intersection(PathfindingState *s, const Common::Point &p, const Common::Point &q, Common::Point *ret) {
	// Computes the nearest intersection point of a line segment and the polygon
	// set. Intersection points that are reached from the inside of a polygon
	// are ignored as are improper intersections which do not obstruct
	// visibility
	// Parameters: (PathfindingState *) s: The pathfinding state
	//             (const Common::Point &) p, q: The line segment (p, q)
	// Returns   : (int) PF_OK on success, PF_ERROR when no intersections were
	//                   found, PF_FATAL otherwise
	//             (Common::Point) *ret: On success, the closest intersection point
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
			if (cont == CONT_INSIDE) {
				delete *it;
				it = s->polygons.erase(it);
				continue;
			}
			break;
		case POLY_BARRED_ACCESS:
		case POLY_NEAREST_ACCESS:
			if (cont == CONT_INSIDE) {
				if (s->_prependPoint != NULL) {
					// We shouldn't get here twice
					warning("AvoidPath: start point is contained in multiple polygons");
					continue;
				}

				if (near_point(start, (*it), new_start) != PF_OK) {
					delete new_start;
					return NULL;
				}

				if (type == POLY_BARRED_ACCESS)
					warning("AvoidPath: start position at unreachable location");

				// The original start position is in an invalid location, so we
				// use the moved point and add the original one to the final path
				// later on.
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
					// We shouldn't get here twice
					warning("AvoidPath: end point is contained in multiple polygons");
					continue;
				}

				// The original end position is in an invalid location, so we move the point
				if (near_point(end, (*it), new_end) != PF_OK) {
					delete new_end;
					return NULL;
				}

				// For near-point access polygons we need to add the original end point
				// to the path after pathfinding.
				if (type == POLY_NEAREST_ACCESS)
					s->_appendPoint = new Common::Point(end);
			}
		}

		++it;
	}

	return new_end;
}

static Vertex *merge_point(PathfindingState *s, const Common::Point &v) {
	// Merges a point into the polygon set. A new vertex is allocated for this
	// point, unless a matching vertex already exists. If the point is on an
	// already existing edge that edge is split up into two edges connected by
	// the new vertex
	// Parameters: (PathfindingState *) s: The pathfinding state
	//             (const Common::Point &) v: The point to merge
	// Returns   : (Vertex *) The vertex corresponding to v
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

static Polygon *convert_polygon(EngineState *s, reg_t polygon) {
	// Converts an SCI polygon into a Polygon
	// Parameters: (EngineState *) s: The game state
	//             (reg_t) polygon: The SCI polygon to convert
	// Returns   : (Polygon *) The converted polygon
	SegManager *segMan = s->segMan;
	int i;
	reg_t points = GET_SEL32(polygon, points);
	int size = GET_SEL32(polygon, size).toUint16();
	const byte *list = s->segMan->derefBulkPtr(points, size * POLY_POINT_SIZE);
	Polygon *poly = new Polygon(GET_SEL32(polygon, type).toUint16());
	int is_reg_t = polygon_is_reg_t(list, size);

	// WORKAROUND: broken polygon in lsl1sci, room 350, after opening elevator
	// Polygon has 17 points but size is set to 19
	if ((size == 19) && (s->_gameName == "lsl1sci")) {
		if ((s->currentRoomNumber() == 350)
		&& (read_point(list, is_reg_t, 18) == Common::Point(108, 137))) {
			debug(1, "Applying fix for broken polygon in lsl1sci, room 350");
			size = 17;
		}
	}

	// WORKAROUND: self-intersecting polygons in ECO, rooms 221, 280 and 300
	if ((size == 11) && (s->_gameName == "ecoquest")) {
		if ((s->currentRoomNumber() == 300)
		&& (read_point(list, is_reg_t, 10) == Common::Point(221, 0))) {
			debug(1, "Applying fix for self-intersecting polygon in ECO, room 300");
			size = 10;
		}
	}
	if ((size == 12) && (s->_gameName == "ecoquest")) {
		if ((s->currentRoomNumber() == 280)
		&& (read_point(list, is_reg_t, 11) == Common::Point(238, 189))) {
			debug(1, "Applying fix for self-intersecting polygon in ECO, room 280");
			size = 10;
		}
	}
	if ((size == 16) && (s->_gameName == "ecoquest")) {
		if ((s->currentRoomNumber() == 221)
		&& (read_point(list, is_reg_t, 1) == Common::Point(419, 175))) {
			debug(1, "Applying fix for self-intersecting polygon in ECO, room 221");
			// Swap the first two points
			poly->vertices.insertHead(new Vertex(read_point(list, is_reg_t, 1)));
			poly->vertices.insertHead(new Vertex(read_point(list, is_reg_t, 0)));
			size = 14;
			assert(!is_reg_t);
			list += 2 * POLY_POINT_SIZE;
		}
	}

	for (i = 0; i < size; i++) {
		Vertex *vertex = new Vertex(read_point(list, is_reg_t, i));
		poly->vertices.insertHead(vertex);
	}

	fix_vertex_order(poly);

	return poly;
}

// WORKAROUND: intersecting polygons in Longbow, room 210.
static void fixLongbowRoom210(PathfindingState *s, const Common::Point &start, const Common::Point &end) {
	Polygon *barred = NULL;
	Polygon *total = NULL;

	// Find the intersecting polygons
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		Polygon *polygon = *it;
		assert(polygon);

		if ((polygon->type == POLY_BARRED_ACCESS) && (polygon->vertices.size() == 11)
		&& (polygon->vertices.first()->v == Common::Point(319, 161)))
			barred = polygon;
		else if ((polygon->type == POLY_TOTAL_ACCESS) && (polygon->vertices.size() == 8)
		&& (polygon->vertices.first()->v == Common::Point(313, 58)))
			total = polygon;
	}

	if (!barred || !total)
		return;

	debug(1, "[avoidpath] Applying fix for intersecting polygons in Longbow, room 210");

	// If the start or end point is contained in the total access polygon, removing that
	// polygon is sufficient. Otherwise we merge the total and barred access polygons.
	bool both_outside = (contained(start, total) == CONT_OUTSIDE) && (contained(end, total) == CONT_OUTSIDE);

	s->polygons.remove(total);
	delete total;

	if (both_outside) {
		int points[28] = {
			224, 159, 223, 162 ,194, 173 ,107, 173, 74, 162, 67, 156, 2, 58,
			63, 160, 0, 160, 0, 0, 319, 0, 319, 161, 228, 161, 313, 58
		};

		s->polygons.remove(barred);
		delete barred;

		barred = new Polygon(POLY_BARRED_ACCESS);

		for (int i = 0; i < 14; i++) {
			Vertex *vertex = new Vertex(Common::Point(points[i * 2], points[i * 2 + 1]));
			barred->vertices.insertHead(vertex);
		}

		s->polygons.push_front(barred);
	}
}

static void change_polygons_opt_0(PathfindingState *s) {
	// Changes the polygon list for optimization level 0 (used for keyboard
	// support). Totally accessible polygons are removed and near-point
	// accessible polygons are changed into totally accessible polygons.
	// Parameters: (PathfindingState *) s: The pathfinding state

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

static PathfindingState *convert_polygon_set(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	// Converts the SCI input data for pathfinding
	// Parameters: (EngineState *) s: The game state
	//             (reg_t) poly_list: Polygon list
	//             (Common::Point) start: The start point
	//             (Common::Point) end: The end point
	//             (int) opt: Optimization level (0, 1 or 2)
	// Returns   : (PathfindingState *) On success a newly allocated pathfinding state,
	//                            NULL otherwise
	SegManager *segMan = s->segMan;
	Polygon *polygon;
	int err;
	int count = 0;
	PathfindingState *pf_s = new PathfindingState();

	// Convert all polygons
	if (poly_list.segment) {
		List *list = lookup_list(s, poly_list);
		Node *node = lookup_node(s, list->first);

		while (node) {
			Node *dup = lookup_node(s, list->first);

			// Workaround for game bugs that put a polygon in the list more than once
			while (dup != node) {
				if (polygons_equal(s->segMan, node->value, dup->value)) {
					warning("[avoidpath] Ignoring duplicate polygon");
					break;
				}
				dup = lookup_node(s, dup->succ);
			}

			if (dup == node) {
				// Polygon is not a duplicate, so convert it
				polygon = convert_polygon(s, node->value);
				pf_s->polygons.push_back(polygon);
				count += GET_SEL32(node->value, size).toUint16();
			}

			node = lookup_node(s, node->succ);
		}
	}

	if (opt == 0) {
		Common::Point intersection;

		// Keyboard support
		// FIXME: We don't need to dijkstra for keyboard support as we currently do
		change_polygons_opt_0(pf_s);

		// Find nearest intersection
		err = nearest_intersection(pf_s, start, end, &intersection);

		if (err == PF_FATAL) {
			warning("AvoidPath: fatal error finding nearest intersection");
			delete pf_s;
			return NULL;
		}

		if (err == PF_OK) {
			// Intersection was found, prepend original start position after pathfinding
			pf_s->_prependPoint = new Common::Point(start);
			// Merge new start point into polygon set
			pf_s->vertex_start = merge_point(pf_s, intersection);
		} else {
			// Otherwise we proceed with the original start point
			pf_s->vertex_start = merge_point(pf_s, start);
		}
		// Merge end point into polygon set
		pf_s->vertex_end = merge_point(pf_s, end);
	} else {
		Common::Point *new_start = fixup_start_point(pf_s, start);

		if (!new_start) {
			warning("AvoidPath: Couldn't fixup start position for pathfinding");
			delete pf_s;
			return NULL;
		}

		Common::Point *new_end = fixup_end_point(pf_s, end);

		if (!new_end) {
			warning("AvoidPath: Couldn't fixup end position for pathfinding");
			delete pf_s;
			return NULL;
		}

		if (s->_gameName == "longbow" && s->currentRoomNumber() == 210)
				fixLongbowRoom210(pf_s, *new_start, *new_end);

		// Merge start and end points into polygon set
		pf_s->vertex_start = merge_point(pf_s, *new_start);
		pf_s->vertex_end = merge_point(pf_s, *new_end);

		delete new_start;
		delete new_end;
	}

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

static int intersecting_polygons(PathfindingState *s) {
	// Detects (self-)intersecting polygons
	// Parameters: (PathfindingState *) s: The pathfinding state
	// Returns   : (int) 1 if s contains (self-)intersecting polygons, 0 otherwise
	int i, j;

	for (i = 0; i < s->vertices; i++) {
		Vertex *v1 = s->vertex_index[i];
		if (!VERTEX_HAS_EDGES(v1))
			continue;
		for (j = i + 1; j < s->vertices; j++) {
			Vertex *v2 = s->vertex_index[j];
			if (!VERTEX_HAS_EDGES(v2))
				continue;

			// Skip neighbouring edges
			if ((CLIST_NEXT(v1) == v2) || CLIST_PREV(v1) == v2)
				continue;

			if (intersect(v1->v, CLIST_NEXT(v1)->v,
			              v2->v, CLIST_NEXT(v2)->v))
				return 1;
		}
	}

	return 0;
}

static void dijkstra(PathfindingState *s) {
	// Computes a shortest path from vertex_start to vertex_end. The caller can
	// construct the resulting path by following the path_prev links from
	// vertex_end back to vertex_start. If no path exists vertex_end->path_prev
	// will be NULL
	// Parameters: (PathfindingState *) s: The pathfinding state
	Polygon *polygon;

	// Vertices of which the shortest path is known
	VertexList done;

	// The remaining vertices
	VertexList remain;

	// Start out with all vertices in set remain
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices) {
			remain.push_front(vertex);
		}
	}

	s->vertex_start->dist = 0;

	// Loop until we find vertex_end
	while (1) {
		// Find vertex at shortest distance from set done
		VertexList::iterator vertex_min_it = remain.end();
		Vertex *vertex_min = 0;
		uint32 min = HUGE_DISTANCE;
		for (VertexList::iterator it = remain.begin(); it != remain.end(); ++it) {
			Vertex *vertex = *it;
			if (vertex->dist < min) {
				vertex_min_it = it;
				vertex_min = *vertex_min_it;
				min = vertex->dist;
			}
		}

		if (min == HUGE_DISTANCE) {
			warning("[avoidpath] End point (%i, %i) is unreachable", s->vertex_end->v.x, s->vertex_end->v.y);
			return;
		}

		// If vertex_end is at shortest distance we can stop
		if (vertex_min == s->vertex_end)
			return;

		// Move vertex from set remain to set done
		done.push_front(vertex_min);
		remain.erase(vertex_min_it);

		VertexList *visVerts = visible_vertices(s, vertex_min);

		for (VertexList::iterator it = visVerts->begin(); it != visVerts->end(); ++it) {
			uint32 new_dist;
			Vertex *vertex = *it;

			// Early pathfinding-enabled games exclude edges on screen borders.
// FIXME: Enable this selectively for those games that need it.
#if 0
			// Avoid plotting path along screen edge
			if ((vertex != s->vertex_end) && point_on_screen_border(vertex->v))
				continue;
#endif
			new_dist = vertex_min->dist + (uint32)sqrt((float)vertex_min->v.sqrDist(vertex->v));
			if (new_dist < vertex->dist) {
				vertex->dist = new_dist;
				vertex->path_prev = vertex_min;
			}
		}

		delete visVerts;
	}
}

static reg_t output_path(PathfindingState *p, EngineState *s) {
	// Stores the final path in newly allocated dynmem
	// Parameters: (PathfindingState *) p: The pathfinding state
	//             (EngineState *) s: The game state
	// Returns   : (reg_t) Pointer to dynmem containing path
	int path_len = 0;
	byte *oref;
	reg_t output;
	Vertex *vertex = p->vertex_end;
	int unreachable = vertex->path_prev == NULL;

	if (unreachable) {
		// If pathfinding failed we only return the path up to vertex_start
		oref = s->segMan->allocDynmem(POLY_POINT_SIZE * 3, AVOIDPATH_DYNMEM_STRING, &output);

		if (p->_prependPoint)
			POLY_SET_POINT(oref, 0, *p->_prependPoint);
		else
			POLY_SET_POINT(oref, 0, p->vertex_start->v);

		POLY_SET_POINT(oref, 1, p->vertex_start->v);
		POLY_SET_POINT(oref, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

		return output;
	}

	while (vertex) {
		// Compute path length
		path_len++;
		vertex = vertex->path_prev;
	}

	// Allocate memory for path, plus 3 extra for appended point, prepended point and sentinel
	oref = s->segMan->allocDynmem(POLY_POINT_SIZE * (path_len + 3), AVOIDPATH_DYNMEM_STRING, &output);

	int offset = 0;

	if (p->_prependPoint)
		POLY_SET_POINT(oref, offset++, *p->_prependPoint);

	vertex = p->vertex_end;
	for (int i = path_len - 1; i >= 0; i--) {
		POLY_SET_POINT(oref, offset + i, vertex->v);
		vertex = vertex->path_prev;
	}
	offset += path_len;

	if (p->_appendPoint)
		POLY_SET_POINT(oref, offset++, *p->_appendPoint);

	// Sentinel
	POLY_SET_POINT(oref, offset, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

#ifdef DEBUG_AVOIDPATH
	printf("[avoidpath] Returning path:");
	for (int i = 0; i < offset; i++) {
		Common::Point pt;
		POLY_GET_POINT(oref, i, pt);
		printf(" (%i, %i)", pt.x, pt.y);
	}
	printf("\n");
#endif

	return output;
}

reg_t kAvoidPath(EngineState *s, int, int argc, reg_t *argv) {
	Common::Point start = Common::Point(argv[0].toSint16(), argv[1].toSint16());

#ifdef DEBUG_AVOIDPATH
	GfxPort *port = s->picture_port;

	if (!port->_decorations) {
		port->_decorations = gfxw_new_list(gfx_rect(0, 0, 320, 200), 0);
		port->_decorations->setVisual(port->_visual);
	} else {
		port->_decorations->free_contents(port->_decorations);
	}
#endif

	switch (argc) {

	case 3 : {
		reg_t retval;
		Polygon *polygon = convert_polygon(s, argv[2]);
		// Override polygon type to prevent inverted result for contained access polygons
		polygon->type = POLY_BARRED_ACCESS;

		retval = make_reg(0, contained(start, polygon) != CONT_OUTSIDE);
		delete polygon;
		return retval;
	}
	case 6 :
	case 7 : {
		Common::Point end = Common::Point(argv[2].toSint16(), argv[3].toSint16());
		reg_t poly_list = argv[4];
		//int poly_list_size = argv[5].toUint16();
		int opt = (argc > 6) ? argv[6].toUint16() : 1;
		reg_t output;
		PathfindingState *p;

#ifdef DEBUG_AVOIDPATH
		printf("[avoidpath] Pathfinding input:\n");
		draw_point(s, start, 1);
		draw_point(s, end, 0);

		if (poly_list.segment) {
			print_input(s, poly_list, start, end, opt);
			draw_input(s, poly_list, start, end, opt);
		}
#endif

		p = convert_polygon_set(s, poly_list, start, end, opt);

		if (p && intersecting_polygons(p)) {
			warning("[avoidpath] input set contains (self-)intersecting polygons");
			delete p;
			p = NULL;
		}

		if (!p) {
			byte *oref;
			printf("[avoidpath] Error: pathfinding failed for following input:\n");
			print_input(s, poly_list, start, end, opt);
			printf("[avoidpath] Returning direct path from start point to end point\n");
			oref = s->segMan->allocDynmem(POLY_POINT_SIZE * 3,
			                                   AVOIDPATH_DYNMEM_STRING, &output);

			POLY_SET_POINT(oref, 0, start);
			POLY_SET_POINT(oref, 1, end);
			POLY_SET_POINT(oref, 2, Common::Point(POLY_LAST_POINT, POLY_LAST_POINT));

			return output;
		}

		dijkstra(p);

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

} // End of namespace Sci
