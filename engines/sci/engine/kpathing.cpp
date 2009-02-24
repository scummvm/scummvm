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

/* Detailed information on the implementation can be found in the report
** which can be downloaded from FIXME.
*/

#include "sci/include/engine.h"
#include "sci/engine/aatree.h"
#include "sci/include/list.h"
#include "sci/gfx/gfx_widgets.h"

#include "common/list.h"

namespace Sci {

#define POLY_LAST_POINT 0x7777
#define POLY_POINT_SIZE 4

#define POLY_GET_POINT(p, i, x, y) \
				do { \
					x = getInt16((p) + (i) * POLY_POINT_SIZE); \
					y = getInt16((p) + (i) * POLY_POINT_SIZE + 2); \
				} while (0)

#define POLY_SET_POINT(p, i, x, y) \
				do { \
					putInt16((p) + (i) * POLY_POINT_SIZE, x); \
					putInt16((p) + (i) * POLY_POINT_SIZE + 2, y); \
				} while (0)

#define POLY_GET_POINT_REG_T(p, i, x, y) \
				do { \
					x = KP_SINT((p)[(i) * 2]); \
					y = KP_SINT((p)[(i) * 2 + 1]); \
			} while (0)

// SCI-defined polygon types
#define POLY_TOTAL_ACCESS 0
#define POLY_NEAREST_ACCESS 1
#define POLY_BARRED_ACCESS 2
#define POLY_CONTAINED_ACCESS 3

// Polygon containment types
#define CONT_OUTSIDE 0
#define CONT_ON_EDGE 1
#define CONT_INSIDE 2

#define POINT_EQUAL(A, B) (((A).x == (B).x) && ((A).y == (B).y))

#define HUGE_DISTANCE 1e10

// Visibility matrix
#define VIS_MATRIX_ROW_SIZE(N) (((N) / 8) + ((N) % 8 ? 1 : 0))
#define SET_VISIBLE(S, P, Q) ((S)->vis_matrix)[(P) * VIS_MATRIX_ROW_SIZE((S)->vertices) \
	+ (Q) / 8] |= (1 << ((Q) % 8))
#define IS_VISIBLE(S, P, Q) (((S)->vis_matrix[(P) * VIS_MATRIX_ROW_SIZE((S)->vertices) \
	+ (Q) / 8] & (1 << ((Q) % 8))) != 0)

#define VERTEX_HAS_EDGES(V) ((V) != CLIST_NEXT(V, entries))

// Error codes
#define PF_OK 0
#define PF_ERROR -1
#define PF_FATAL -2

// Floating point struct
struct FloatPoint {
	FloatPoint() : x(0), y(0) {}
	FloatPoint(float x_, float y_) : x(x_), y(y_) {}

	float x, y;
};

FloatPoint toFloatPoint(Common::Point p) {
	return FloatPoint(p.x, p.y);
}

struct Vertex {
	// Location
	Common::Point v;

	// Index
	int idx;

	// Vertex circular list entry
	struct {
		Vertex *cle_next;	// next element
		Vertex *cle_prev;	// previous element
	} entries;

	// Dijkstra list entry
	LIST_ENTRY(Vertex) dijkstra;	// TODO: Convert this

	// Distance from starting vertex
	float dist;

	// Previous vertex in shortest path
	Vertex *path_prev;
};

typedef Common::List<Vertex *> VertexList;


class CircularVertexList {
public:
	Vertex *_head;

public:
	CircularVertexList() : _head(0) {}
	
	Vertex *first() {
		return _head;
	}
	
	void insertHead(Vertex *elm) {
		if (_head == NULL) {
			elm->entries.cle_next = elm->entries.cle_prev = elm;
		} else {
			elm->entries.cle_next = _head;
			elm->entries.cle_prev = _head->entries.cle_prev;
			_head->entries.cle_prev = elm;
			elm->entries.cle_prev->entries.cle_next = elm;
		}
		_head = elm;
	}

	static void insertAfter(Vertex *listelm, Vertex *elm) {
		elm->entries.cle_prev = listelm;
		(elm)->entries.cle_next = listelm->entries.cle_next;
		listelm->entries.cle_next->entries.cle_prev = elm;
		listelm->entries.cle_next = elm;
	}

	void remove(Vertex *elm) {
		if (elm->entries.cle_next == elm) {
			_head = NULL;
		} else {
			if (_head == elm)
				_head = elm->entries.cle_next;
			elm->entries.cle_prev->entries.cle_next = elm->entries.cle_next;
			elm->entries.cle_next->entries.cle_prev = elm->entries.cle_prev;
		}
	}

	bool empty() const {
		return _head == NULL;
	}
};

/* Circular list definitions. */

#define CLIST_FOREACH(var, head, field)					\
	for ((var) = (head)->first();					\
		(var);							\
		(var) = ((var)->field.cle_next == (head)->first() ?	\
		    NULL : (var)->field.cle_next))

/* Circular list access methods. */
#define CLIST_NEXT(elm, field)		((elm)->field.cle_next)
#define CLIST_PREV(elm, field)		((elm)->field.cle_prev)


struct Polygon {
	// SCI polygon type
	int type;

	// Circular list of vertices
	CircularVertexList vertices;
};

typedef Common::List<Polygon *> PolygonList;

// Pathfinding state
struct PathfindingState {
	// List of all polygons
	PolygonList polygons;

	// Original start and end points
	Common::Point start, end;

	// Flags for adding original points to final path
	char keep_start, keep_end;

	// Start and end points for pathfinding
	Vertex *vertex_start, *vertex_end;

	// Array to quickly find vertices by idx
	Vertex **vertex_index;

	// Visibility matrix
	char *vis_matrix;

	// Total number of vertices
	int vertices;
	
	PathfindingState(const Common::Point &s, const Common::Point &e) : start(s), end(e) {
		keep_start = 0;
		keep_end = 0;
		vertex_start = NULL;
		vertex_end = NULL;
		vertex_index = NULL;
		vis_matrix = NULL;
		vertices = 0;
	}
};

static Vertex *vertex_cur;

// Temporary hack to deal with points in reg_ts
static int polygon_is_reg_t(unsigned char *list, int size) {
	int i;

	// Check the first three reg_ts
	for (i = 0; i < (size < 3 ? size : 3); i++)
		if ((((reg_t *) list) + i)->segment)
			// Non-zero segment, cannot be reg_ts
			return 0;

	// First three segments were zero, assume reg_ts
	return 1;
}

static Common::Point read_point(unsigned char *list, int is_reg_t, int offset) {
	Common::Point point;

	if (!is_reg_t) {
		POLY_GET_POINT(list, offset, point.x, point.y);
	} else {
		POLY_GET_POINT_REG_T((reg_t *)list, offset, point.x, point.y);
	}

	return point;
}

static void draw_line(EngineState *s, Common::Point p1, Common::Point p2, int type) {
	// Colors for polygon debugging.
	// Green: Total access
	// Red : Barred access
	// Blue: Near-point access
	// Yellow: Contained access
	int poly_colors[][3] = {{0, 255, 0}, {0, 0, 255}, {255, 0, 0}, {255, 255, 0}};
	gfx_color_t col;
	gfxw_list_t *decorations = s->picture_port->decorations;
	gfxw_primitive_t *line;

	col.visual.global_index = GFX_COLOR_INDEX_UNMAPPED;
	col.visual.r = poly_colors[type][0];
	col.visual.g = poly_colors[type][1];
	col.visual.b = poly_colors[type][2];
	col.alpha = 0;
	col.priority = -1;
	col.control = 0;
	col.mask = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;

	p1.y += 10;
	p2.y += 10;

	line = gfxw_new_line(p1, p2, col, GFX_LINE_MODE_CORRECT, GFX_LINE_STYLE_NORMAL);
	decorations->add((gfxw_container_t *)decorations, (gfxw_widget_t *)line);
}

static void draw_point(EngineState *s, Common::Point p, int start) {
	// Colors for starting and end point
	// Green: End point
	// Blue: Starting point
	int point_colors[][3] = {{0, 255, 0}, {0, 0, 255}};
	gfx_color_t col;
	gfxw_list_t *decorations = s->picture_port->decorations;
	gfxw_box_t *box;

	col.visual.global_index = GFX_COLOR_INDEX_UNMAPPED;
	col.visual.r = point_colors[start][0];
	col.visual.g = point_colors[start][1];
	col.visual.b = point_colors[start][2];
	col.alpha = 0;
	col.priority = -1;
	col.control = 0;
	col.mask = GFX_MASK_VISUAL | GFX_MASK_PRIORITY;

	box = gfxw_new_box(s->gfx_state, gfx_rect(p.x - 1, p.y - 1 + 10, 3, 3), col, col, GFX_BOX_SHADE_FLAT);
	decorations->add((gfxw_container_t *) decorations, (gfxw_widget_t *) box);
}

static void draw_polygon(EngineState *s, reg_t polygon) {
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	int type = KP_UINT(GET_SEL32(polygon, type));
	Common::Point first, prev;
	unsigned char *list = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
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
	list_t *list;
	node_t *node;

	draw_point(s, start, 1);
	draw_point(s, end, 0);

	if (!poly_list.segment)
		return;

	list = LOOKUP_LIST(poly_list);

	if (!list) {
		warning("Could not obtain polygon list");
		return;
	}

	node = LOOKUP_NODE(list->first);

	while (node) {
		draw_polygon(s, node->value);
		node = LOOKUP_NODE(node->succ);
	}
}

static void print_polygon(EngineState *s, reg_t polygon) {
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	int type = KP_UINT(GET_SEL32(polygon, type));
	int i;
	unsigned char *point_array = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
	int is_reg_t = polygon_is_reg_t(point_array, size);
	Common::Point point;

	sciprintf("%i:", type);

	for (i = 0; i < size; i++) {
		point = read_point(point_array, is_reg_t, i);
		sciprintf(" (%i, %i)", point.x, point.y);
	}

	point = read_point(point_array, is_reg_t, 0);
	sciprintf(" (%i, %i);\n", point.x, point.y);
}

static void print_input(EngineState *s, reg_t poly_list, Common::Point start, Common::Point end, int opt) {
	list_t *list;
	node_t *node;

	sciprintf("Start point: (%i, %i)\n", start.x, start.y);
	sciprintf("End point: (%i, %i)\n", end.x, end.y);
	sciprintf("Optimization level: %i\n", opt);

	if (!poly_list.segment)
		return;

	list = LOOKUP_LIST(poly_list);

	if (!list) {
		warning("Could not obtain polygon list");
		return;
	}

	sciprintf("Polygons:\n");
	node = LOOKUP_NODE(list->first);

	while (node) {
		print_polygon(s, node->value);
		node = LOOKUP_NODE(node->succ);
	}
}

static int area(Common::Point a, Common::Point b, Common::Point c) {
	// Computes the area of a triangle
	// Parameters: (Common::Point) a, b, c: The points of the triangle
	// Returns   : (int) The area multiplied by two
	return (b.x - a.x) * (a.y - c.y) - (c.x - a.x) * (a.y - b.y);
}

static int left(Common::Point a, Common::Point b, Common::Point c) {
	// Determines whether or not a point is to the left of a directed line
	// Parameters: (Common::Point) a, b: The directed line (a, b)
	//             (Common::Point) c: The query point
	// Returns   : (int) 1 if c is to the left of (a, b), 0 otherwise
	return area(a, b, c) > 0;
}

static int left_on(Common::Point a, Common::Point b, Common::Point c) {
	// Determines whether or not a point is to the left of or collinear with a
	// directed line
	// Parameters: (Common::Point) a, b: The directed line (a, b)
	//             (Common::Point) c: The query point
	// Returns   : (int) 1 if c is to the left of or collinear with (a, b), 0
	//                   otherwise
	return area(a, b, c) >= 0;
}

static int collinear(Common::Point a, Common::Point b, Common::Point c) {
	// Determines whether or not three points are collinear
	// Parameters: (Common::Point) a, b, c: The three points
	// Returns   : (int) 1 if a, b, and c are collinear, 0 otherwise
	return area(a, b, c) == 0;
}

static int between(Common::Point a, Common::Point b, Common::Point c) {
	// Determines whether or not a point lies on a line segment
	// Parameters: (Common::Point) a, b: The line segment (a, b)
	//             (Common::Point) c: The query point
	// Returns   : (int) 1 if c lies on (a, b), 0 otherwise
	if (!collinear(a, b, c))
		return 0;

	// Assumes a != b.
	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

static int intersect_proper(Common::Point a, Common::Point b, Common::Point c, Common::Point d) {
	// Determines whether or not two line segments properly intersect
	// Parameters: (Common::Point) a, b: The line segment (a, b)
	//             (Common::Point) c, d: The line segment (c, d)
	// Returns   : (int) 1 if (a, b) properly intersects (c, d), 0 otherwise
	int ab = (left(a, b, c) && left(b, a, d)) || (left(a, b, d) && left(b, a, c));
	int cd = (left(c, d, a) && left(d, c, b)) || (left(c, d, b) && left(d, c, a));

	return ab && cd;
}

static int intersect(Common::Point a, Common::Point b, Common::Point c, Common::Point d) {
	// Determines whether or not two line segments intersect
	// Parameters: (Common::Point) a, b: The line segment (a, b)
	//             (Common::Point) c, d: The line segment (c, d)
	// Returns   : (int) 1 if (a, b) intersects (c, d), 0 otherwise
	if (intersect_proper(a, b, c, d))
		return 1;

	return between(a, b, c) || between(a, b, d) || between(c, d, a) || between(c, d, b);
}

static Vertex *vertex_new(Common::Point p) {
	// Allocates and initialises a new vertex
	// Parameters: (Common::Point) p: The position of the vertex
	// Returns   : (Vertex *) A newly allocated vertex
	Vertex *vertex = (Vertex*)sci_malloc(sizeof(Vertex));

	vertex->v = p;
	vertex->dist = HUGE_DISTANCE;
	vertex->path_prev = NULL;

	return vertex;
}

static Polygon *polygon_new(int type) {
	// Allocates and initialises a new polygon
	// Parameters: (int) type: The SCI polygon type
	// Returns   : (Polygon *) A newly allocated polygon
	Polygon *polygon = new Polygon();
	polygon->type = type;

	return polygon;
}

static int contained(Common::Point p, Polygon *polygon) {
	// Polygon containment test
	// Parameters: (Common::Point) p: The point
	//             (Polygon *) polygon: The polygon
	// Returns   : (int) CONT_INSIDE if p is strictly contained in polygon,
	//                   CONT_ON_EDGE if p lies on an edge of polygon,
	//                   CONT_OUTSIDE otherwise
	// Number of ray crossing left and right
	int lcross = 0, rcross = 0;
	Vertex *vertex;

	// Iterate over edges
	CLIST_FOREACH(vertex, &polygon->vertices, entries) {
		Common::Point v1 = vertex->v;
		Common::Point v2 = CLIST_NEXT(vertex, entries)->v;

		// Flags for ray straddling left and right
		int rstrad, lstrad;

		// Check if p is a vertex
		if (POINT_EQUAL(p, v1))
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

	v = CLIST_NEXT(first, entries);

	while (CLIST_NEXT(v, entries) != first) {
		size += area(first->v, v->v, CLIST_NEXT(v, entries)->v);
		v = CLIST_NEXT(v, entries);
	}

	return size;
}

static void fix_vertex_order(Polygon *polygon) {
	// Fixes the vertex order of a polygon if incorrect. Contained access
	// polygons should have their vertices ordered clockwise, all other types
	// anti-clockwise
	// Parameters: (Polygon *) polygon: The polygon
	// Returns   : (void)
	int area = polygon_area(polygon);

	// When the polygon area is positive the vertices are ordered
	// anti-clockwise. When the area is negative the vertices are ordered
	// clockwise
	if (((area > 0) && (polygon->type == POLY_CONTAINED_ACCESS))
	        || ((area < 0) && (polygon->type != POLY_CONTAINED_ACCESS))) {

		// Create a new circular list
		CircularVertexList vertices;

		while (!polygon->vertices.empty()) {
			// Put first vertex in new list
			Vertex *vertex = polygon->vertices.first();
			polygon->vertices.remove(vertex);
			vertices.insertHead(vertex);
		}

		polygon->vertices = vertices;
	}
}

static int vertex_compare(const void *a, const void *b) {
	// Compares two vertices by angle (first) and distance (second) in relation
	// to vertex_cur. The angle is relative to the horizontal line extending
	// right from vertex_cur, and increases clockwise
	// Parameters: (const void *) a, b: The vertices
	// Returns   : (int) -1 if a is smaller than b, 1 if a is larger than b, and
	//                   0 if a and b are equal
	Common::Point p0 = vertex_cur->v;
	Common::Point p1 = (*(Vertex **) a)->v;
	Common::Point p2 = (*(Vertex **) b)->v;

	if (POINT_EQUAL(p1, p2))
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

static void clockwise(Vertex *v, Common::Point *p1, Common::Point *p2) {
	// Orders the points of an edge clockwise around vertex_cur. If all three
	// points are collinear the original order is used
	// Parameters: (Vertex *) v: The first vertex of the edge
	// Returns   : (void)
	//             (Common::Point) *p1: The first point in clockwise order
	//             (Common::Point) *p2: The second point in clockwise order
	Vertex *w = CLIST_NEXT(v, entries);

	if (left_on(vertex_cur->v, w->v, v->v)) {
		*p1 = v->v;
		*p2 = w->v;
		return;
	}

	*p1 = w->v;
	*p2 = v->v;
	return;
}

static int edge_compare(const void *a, const void *b) {
	// Compares two edges that are intersected by the sweeping line by distance
	// from vertex_cur
	// Parameters: (const void *) a, b: The first vertices of the edges
	// Returns   : (int) -1 if a is closer than b, 1 if b is closer than a, and
	//                   0 if a and b are equal
	Common::Point v1, v2, w1, w2;

	// We can assume that the sweeping line intersects both edges and
	// that the edges do not properly intersect

	if (a == b)
		return 0;

	// Order vertices clockwise so we know vertex_cur is to the right of
	// directed edges (v1, v2) and (w1, w2)
	clockwise((Vertex *) a, &v1, &v2);
	clockwise((Vertex *) b, &w1, &w2);

	// As the edges do not properly intersect one edge must lie entirely
	// to one side of another. Note that the special case where edges are
	// collinear does not need to be handled as those edges will never be
	// in the tree simultaneously

	// b is left of a
	if (left_on(v1, v2, w1) && left_on(v1, v2, w2))
		return -1;

	// b is right of a
	if (left_on(v2, v1, w1) && left_on(v2, v1, w2))
		return 1;

	// a is left of b
	if (left_on(w1, w2, v1) && left_on(w1, w2, v2))
		return 1;

	// a is right of b
	return -1;
}

static int inside(Common::Point p, Vertex *vertex) {
	// Determines whether or not a line from a point to a vertex intersects the
	// interior of the polygon, locally at that vertex
	// Parameters: (Common::Point) p: The point
	//             (Vertex *) vertex: The vertex
	// Returns   : (int) 1 if the line (p, vertex->v) intersects the interior of
	//                   the polygon, locally at the vertex. 0 otherwise
	// Check that it's not a single-vertex polygon
	if (VERTEX_HAS_EDGES(vertex)) {
		Common::Point prev = CLIST_PREV(vertex, entries)->v;
		Common::Point next = CLIST_NEXT(vertex, entries)->v;
		Common::Point cur = vertex->v;

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

static int visible(Vertex *vertex, Vertex *vertex_prev, int visible, aatree_t *tree) {
	// Determines whether or not a vertex is visible from vertex_cur
	// Parameters: (Vertex *) vertex: The vertex
	//             (Vertex *) vertex_prev: The previous vertex in the sort
	//                                       order, or NULL
	//             (int) visible: 1 if vertex_prev is visible, 0 otherwise
	//             (aatree_t *) tree: The tree of edges intersected by the
	//                                sweeping line
	// Returns   : (int) 1 if vertex is visible from vertex_cur, 0 otherwise
	Vertex *edge;
	Common::Point p = vertex_cur->v;
	Common::Point w = vertex->v;
	aatree_t *tree_n = tree;

	// Check if sweeping line intersects the interior of the polygon
	// locally at vertex
	if (inside(p, vertex))
		return 0;

	// If vertex_prev is on the sweeping line, then vertex is invisible
	// if vertex_prev is invisible
	if (vertex_prev && !visible && between(p, w, vertex_prev->v))
		return 0;

	// Find leftmost node of tree */
	while ((tree_n = aatree_walk(tree_n, AATREE_WALK_LEFT)))
		tree = tree_n;

	edge = (Vertex*)aatree_get_data(tree);

	if (edge) {
		Common::Point p1, p2;

		// Check for intersection with sweeping line before vertex
		clockwise(edge, &p1, &p2);
		if (left(p2, p1, p) && left(p1, p2, w))
			return 0;
	}

	return 1;
}

static void visible_vertices(PathfindingState *s, Vertex *vert) {
	// Determines all vertices that are visible from a particular vertex and
	// updates the visibility matrix
	// Parameters: (PathfindingState *) s: The pathfinding state
	//             (Vertex *) vert: The vertex
	// Returns   : (void)
	aatree_t *tree = aatree_new();
	Common::Point p = vert->v;
	Polygon *polygon;
	int i;
	int is_visible;
	Vertex **vert_sorted = (Vertex**)sci_malloc(sizeof(Vertex *) * s->vertices);

	// Sort vertices by angle (first) and distance (second)
	memcpy(vert_sorted, s->vertex_index, sizeof(Vertex *) * s->vertices);
	vertex_cur = vert;
	qsort(vert_sorted, s->vertices, sizeof(Vertex *), vertex_compare);

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;
		vertex = polygon->vertices.first();

		// Check that there is more than one vertex.
		if (VERTEX_HAS_EDGES(vertex))
			CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			Common::Point high, low;

			// Add edges that intersect the initial position of the sweeping line
			clockwise(vertex, &high, &low);

			if ((high.y < p.y) && (low.y >= p.y) && !POINT_EQUAL(low, p))
				aatree_insert(vertex, &tree, edge_compare);
		}
	}

	is_visible = 1;

	// The first vertex will be vertex_cur, so we skip it
	for (i = 1; i < s->vertices; i++) {
		Vertex *v1;

		// Compute visibility of vertex_index[i]
		is_visible = visible(vert_sorted[i], vert_sorted[i - 1], is_visible, tree);

		// Update visibility matrix
		if (is_visible)
			SET_VISIBLE(s, vert->idx, vert_sorted[i]->idx);

		// Delete anti-clockwise edges from tree
		v1 = CLIST_PREV(vert_sorted[i], entries);
		if (left(p, vert_sorted[i]->v, v1->v)) {
			if (aatree_delete(v1, &tree, edge_compare))
				sciprintf("[avoidpath] Error: failed to remove edge from tree\n");
		}

		v1 = CLIST_NEXT(vert_sorted[i], entries);
		if (left(p, vert_sorted[i]->v, v1->v)) {
			if (aatree_delete(vert_sorted[i], &tree, edge_compare))
				sciprintf("[avoidpath] Error: failed to remove edge from tree\n");
		}

		// Add clockwise edges of collinear vertices when sweeping line moves
		if ((i < s->vertices - 1) && !collinear(p, vert_sorted[i]->v, vert_sorted[i + 1]->v)) {
			int j;
			for (j = i; (j >= 1) && collinear(p, vert_sorted[i]->v, vert_sorted[j]->v); j--) {
				v1 = CLIST_PREV(vert_sorted[j], entries);
				if (left(vert_sorted[j]->v, p, v1->v))
					aatree_insert(v1, &tree, edge_compare);

				v1 = CLIST_NEXT(vert_sorted[j], entries);
				if (left(vert_sorted[j]->v, p, v1->v))
					aatree_insert(vert_sorted[j], &tree, edge_compare);
			}
		}
	}

	free(vert_sorted);

	// Free tree
	aatree_free(tree);
}

static float distance(FloatPoint a, FloatPoint b) {
	// Computes the distance between two pointfs
	// Parameters: (Common::Point) a, b: The two pointfs
	// Returns   : (int) The distance between a and b, rounded to int
	float w = a.x - b.x;
	float h = a.y - b.y;

	return sqrt(w * w + h * h);
}

static int point_on_screen_border(Common::Point p) {
	// Determines if a point lies on the screen border
	// Parameters: (Common::Point) p: The point
	// Returns   : (int) 1 if p lies on the screen border, 0 otherwise
	// FIXME get dimensions from somewhere?
	return (p.x == 0) || (p.x == 319) || (p.y == 0) || (p.y == 189);
}

static int edge_on_screen_border(Common::Point p, Common::Point q) {
	// Determines if an edge lies on the screen border
	// Parameters: (Common::Point) p, q: The edge (p, q)
	// Returns   : (int) 1 if (p, q) lies on the screen border, 0 otherwise
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

static int near_point(Common::Point p, Polygon *polygon, Common::Point *ret) {
	// Computes the near point of a point contained in a polygon
	// Parameters: (Common::Point) p: The point
	//             (Polygon *) polygon: The polygon
	// Returns   : (int) PF_OK on success, PF_FATAL otherwise
	//             (Common::Point) *ret: The near point of p in polygon on success
	Vertex *vertex;
	FloatPoint near_p;
	float dist = HUGE_DISTANCE;

	CLIST_FOREACH(vertex, &polygon->vertices, entries) {
		Common::Point p1 = vertex->v;
		Common::Point p2 = CLIST_NEXT(vertex, entries)->v;
		float w, h, l, u;
		FloatPoint new_point;
		float new_dist;

		// Ignore edges on the screen border
		if (edge_on_screen_border(p1, p2))
			continue;

		// Compute near point
		w = p2.x - p1.x;
		h = p2.y - p1.y;
		l = sqrt(w * w + h * h);
		u = ((p.x - p1.x) * (p2.x - p1.x) + (p.y - p1.y) * (p2.y - p1.y)) / (l * l);

		// Clip to edge
		if (u < 0.0f)
			u = 0.0f;
		if (u > 1.0f)
			u = 1.0f;

		new_point.x = p1.x + u * (p2.x - p1.x);
		new_point.y = p1.y + u * (p2.y - p1.y);

		new_dist = distance(toFloatPoint(p), new_point);

		if (new_dist < dist) {
			near_p = new_point;
			dist = new_dist;
		}
	}

	// Find point not contained in polygon
	return find_free_point(near_p, polygon, ret);
}

static int intersection(Common::Point a, Common::Point b, Vertex *vertex, FloatPoint *ret) {
	// Computes the intersection point of a line segment and an edge (not
	// including the vertices themselves)
	// Parameters: (Common::Point) a, b: The line segment (a, b)
	//             (Vertex *) vertex: The first vertex of the edge
	// Returns   : (int) FP_OK on success, PF_ERROR otherwise
	//             (FloatPoint) *ret: The intersection point
	// Parameters of parametric equations
	float s, t;
	// Numerator and denominator of equations
	float num, denom;
	Common::Point c = vertex->v;
	Common::Point d = CLIST_NEXT(vertex, entries)->v;

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

static int nearest_intersection(PathfindingState *s, Common::Point p, Common::Point q, Common::Point *ret) {
	// Computes the nearest intersection point of a line segment and the polygon
	// set. Intersection points that are reached from the inside of a polygon
	// are ignored as are improper intersections which do not obstruct
	// visibility
	// Parameters: (PathfindingState *) s: The pathfinding state
	//             (Common::Point) p, q: The line segment (p, q)
	// Returns   : (int) PF_OK on success, PF_ERROR when no intersections were
	//                   found, PF_FATAL otherwise
	//             (Common::Point) *ret: On success, the closest intersection point
	Polygon *polygon = 0;
	FloatPoint isec;
	Polygon *ipolygon = 0;
	float dist = HUGE_DISTANCE;

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			float new_dist;
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
				if (!left(vertex->v, CLIST_NEXT(vertex, entries)->v, q))
					continue;

				if (intersection(p, q, vertex, &new_isec) != PF_OK)
					continue;
			}

			new_dist = distance(toFloatPoint(p), new_isec);
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

static int fix_point(PathfindingState *s, Common::Point p, Common::Point *ret, Polygon **ret_pol) {
	// Checks a point for containment in any of the polygons in the polygon set.
	// If the point is contained in a totally accessible polygon that polygon
	// is removed from the set. If the point is contained in a polygon of another
	// type the near point is returned. Otherwise the original point is returned
	// Parameters: (Common::Point) p: The point
	// Returns   : (int) PF_OK on success, PF_FATAL otherwise
	//             (Common::Point) *ret: A valid input point for pathfinding
	//             (Polygon *) *ret_pol: The polygon p was contained in if p
	//                                     != *ret, NULL otherwise
	PolygonList::iterator it;
	*ret_pol = NULL;

	// Check for polygon containment
	for (it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		if (contained(p, *it) != CONT_OUTSIDE)
			break;
	}

	if (it != s->polygons.end()) {
		Common::Point near_p;

		if ((*it)->type == POLY_TOTAL_ACCESS) {
			// Remove totally accessible polygon if it contains p
			
			s->polygons.erase(it);
			*ret = p;
			return PF_OK;
		}

		// Otherwise, compute near point
		if (near_point(p, (*it), &near_p) == PF_OK) {
			*ret = near_p;

			if (!POINT_EQUAL(p, *ret))
				*ret_pol = *it;

			return PF_OK;
		}

		return PF_FATAL;
	}

	// p is not contained in any polygon
	*ret = p;
	return PF_OK;
}

static Vertex *merge_point(PathfindingState *s, Common::Point v) {
	// Merges a point into the polygon set. A new vertex is allocated for this
	// point, unless a matching vertex already exists. If the point is on an
	// already existing edge that edge is split up into two edges connected by
	// the new vertex
	// Parameters: (PathfindingState *) s: The pathfinding state
	//             (Common::Point) v: The point to merge
	// Returns   : (Vertex *) The vertex corresponding to v
	Vertex *vertex;
	Vertex *v_new;
	Polygon *polygon;

	// Check for already existing vertex
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		CLIST_FOREACH(vertex, &polygon->vertices, entries)
		if (POINT_EQUAL(vertex->v, v))
			return vertex;
	}

	v_new = vertex_new(v);

	// Check for point being on an edge
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		// Skip single-vertex polygons
		if (VERTEX_HAS_EDGES(polygon->vertices.first()))
			CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			Vertex *next = CLIST_NEXT(vertex, entries);
	
			if (between(vertex->v, next->v, v)) {
				// Split edge by adding vertex
				polygon->vertices.insertAfter(vertex, v_new);
				return v_new;
			}
		}
	}

	// Add point as single-vertex polygon
	polygon = polygon_new(POLY_BARRED_ACCESS);
	polygon->vertices.insertHead(v_new);
	s->polygons.push_front(polygon);

	return v_new;
}

static Polygon *convert_polygon(EngineState *s, reg_t polygon) {
	// Converts an SCI polygon into a Polygon
	// Parameters: (EngineState *) s: The game state
	//             (reg_t) polygon: The SCI polygon to convert
	// Returns   : (Polygon *) The converted polygon
	int i;
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	unsigned char *list = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
	Polygon *poly = polygon_new(KP_UINT(GET_SEL32(polygon, type)));
	int is_reg_t = polygon_is_reg_t(list, size);

	for (i = 0; i < size; i++) {
		Vertex *vertex = vertex_new(read_point(list, is_reg_t, i));
		poly->vertices.insertHead(vertex);
	}

	fix_vertex_order(poly);

	return poly;
}

static void free_polygon(Polygon *polygon) {
	// Frees a polygon and its vertices
	// Parameters: (Polygon *) polygons: The polygon
	// Returns   : (void)
	while (!polygon->vertices.empty()) {
		Vertex *vertex = polygon->vertices.first();
		polygon->vertices.remove(vertex);
		free(vertex);
	}

	delete polygon;
}

static void free_pf_state(PathfindingState *p) {
	// Frees a pathfinding state
	// Parameters: (PathfindingState *) p: The pathfinding state
	// Returns   : (void)
	free(p->vertex_index);
	free(p->vis_matrix);

	for (PolygonList::iterator it = p->polygons.begin(); it != p->polygons.end(); ++it) {
		free_polygon(*it);
	}

	delete p;
}

static void change_polygons_opt_0(PathfindingState *s) {
	// Changes the polygon list for optimization level 0 (used for keyboard
	// support). Totally accessible polygons are removed and near-point
	// accessible polygons are changed into totally accessible polygons.
	// Parameters: (PathfindingState *) s: The pathfinding state
	// Returns   : (void)

	PolygonList::iterator it = s->polygons.begin();
	while (it != s->polygons.end()) {
		Polygon *polygon = *it;
		assert(polygon);

		if (polygon->type == POLY_TOTAL_ACCESS) {
			free_polygon(polygon);
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
	Polygon *polygon;
	int err;
	int count = 0;
	PathfindingState *pf_s = new PathfindingState(start, end);

	// Convert all polygons
	if (poly_list.segment) {
		list_t *list = LOOKUP_LIST(poly_list);
		node_t *node = LOOKUP_NODE(list->first);

		while (node) {
			polygon = convert_polygon(s, node->value);
			pf_s->polygons.push_front(polygon);
			count += KP_UINT(GET_SEL32(node->value, size));
			node = LOOKUP_NODE(node->succ);
		}
	}

	if (opt == 0) {
		// Keyboard support
		change_polygons_opt_0(pf_s);

		// Find nearest intersection
		err = nearest_intersection(pf_s, start, end, &start);

		if (err == PF_FATAL) {
			sciprintf("[avoidpath] Error: fatal error finding nearest intersecton\n");
			free_pf_state(pf_s);
			return NULL;
		} else if (err == PF_OK)
			// Keep original start position if intersection was found
			pf_s->keep_start = 1;
	} else {
		if (fix_point(pf_s, start, &start, &polygon) != PF_OK) {
			sciprintf("[avoidpath] Error: couldn't fix start position for pathfinding\n");
			free_pf_state(pf_s);
			return NULL;
		} else if (polygon) {
			// Start position has moved
			pf_s->keep_start = 1;
			if ((polygon->type != POLY_NEAREST_ACCESS))
				sciprintf("[avoidpath] Warning: start position at unreachable location\n");
		}
	}

	if (fix_point(pf_s, end, &end, &polygon) != PF_OK) {
		sciprintf("[avoidpath] Error: couldn't fix end position for pathfinding\n");
		free_pf_state(pf_s);
		return NULL;
	} else {
		// Keep original end position if it is contained in a
		// near-point accessible polygon
		if (polygon && (polygon->type == POLY_NEAREST_ACCESS))
			pf_s->keep_end = 1;
	}

	// Merge start and end points into polygon set
	pf_s->vertex_start = merge_point(pf_s, start);
	pf_s->vertex_end = merge_point(pf_s, end);

	// Allocate and build vertex index
	pf_s->vertex_index = (Vertex**)sci_malloc(sizeof(Vertex *) * (count + 2));

	count = 0;

	for (PolygonList::iterator it = pf_s->polygons.begin(); it != pf_s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			vertex->idx = count;
			pf_s->vertex_index[count++] = vertex;
		}
	}

	pf_s->vertices = count;

	// Allocate and clear visibility matrix
	pf_s->vis_matrix = (char *)sci_calloc(pf_s->vertices * VIS_MATRIX_ROW_SIZE(pf_s->vertices), 1);

	return pf_s;
}

static void visibility_graph(PathfindingState *s) {
	// Computes the visibility graph
	// Parameters: (PathfindingState *) s: The pathfinding state
	// Returns   : (void)
	Polygon *polygon;

	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries)
		visible_vertices(s, vertex);
	}
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
			if ((CLIST_NEXT(v1, entries) == v2)
			        || CLIST_PREV(v1, entries) == v2)
				continue;

			if (intersect(v1->v, CLIST_NEXT(v1, entries)->v,
			              v2->v, CLIST_NEXT(v2, entries)->v))
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
	// Returns   : (void)
	Polygon *polygon;
	// Vertices of which the shortest path is known
	LIST_HEAD(done_head, Vertex) done;
	// The remaining vertices
	LIST_HEAD(remain_head, Vertex) remain;

	LIST_INIT(remain);
	LIST_INIT(done);

	// Start out with all vertices in set remain
	for (PolygonList::iterator it = s->polygons.begin(); it != s->polygons.end(); ++it) {
		polygon = *it;
		Vertex *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			LIST_INSERT_HEAD(&remain, vertex, dijkstra);
		}
	}

	s->vertex_start->dist = 0.0f;

	// Loop until we find vertex_end
	while (1) {
		int i;
		Vertex *vertex, *vertex_min = 0;
		float min = HUGE_DISTANCE;

		// Find vertex at shortest distance from set done
		LIST_FOREACH(vertex, &remain, dijkstra) {
			if (vertex->dist < min) {
				vertex_min = vertex;
				min = vertex->dist;
			}
		}

		if (min == HUGE_DISTANCE) {
			sciprintf("[avoidpath] Warning: end point (%i, %i) is unreachable\n", s->vertex_end->v.x, s->vertex_end->v.y);
			return;
		}

		// If vertex_end is at shortest distance we can stop
		if (vertex_min == s->vertex_end)
			return;

		// Move vertex from set remain to set done
		LIST_REMOVE(vertex_min, dijkstra);
		LIST_INSERT_HEAD(&done, vertex_min, dijkstra);

		for (i = 0; i < s->vertices; i++) {
			// Adjust upper bound for all vertices that are visible from vertex_min
			if (IS_VISIBLE(s, vertex_min->idx, i)) {
				float new_dist;

				// Avoid plotting path along screen edge
				if ((s->vertex_index[i] != s->vertex_end) && point_on_screen_border(s->vertex_index[i]->v))
					continue;

				new_dist = vertex_min->dist + distance(toFloatPoint(vertex_min->v), toFloatPoint(s->vertex_index[i]->v));
				if (new_dist < s->vertex_index[i]->dist) {
					s->vertex_index[i]->dist = new_dist;
					s->vertex_index[i]->path_prev = vertex_min;
				}
			}
		}
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
	int i;
	int unreachable = vertex->path_prev == NULL;

	if (unreachable) {
		// If pathfinding failed we only return the path up to vertex_start
		oref = s->seg_manager->allocDynmem(POLY_POINT_SIZE * 3, AVOIDPATH_DYNMEM_STRING, &output);

		if (p->keep_start)
			POLY_SET_POINT(oref, 0, p->start.x, p->start.y);
		else
			POLY_SET_POINT(oref, 0, p->vertex_start->v.x, p->vertex_start->v.y);

		POLY_SET_POINT(oref, 1, p->vertex_start->v.x, p->vertex_start->v.y);
		POLY_SET_POINT(oref, 2, POLY_LAST_POINT, POLY_LAST_POINT);

		return output;
	}

	while (vertex) {
		// Compute path length
		path_len++;
		vertex = vertex->path_prev;
	}

	oref = s->seg_manager->allocDynmem(POLY_POINT_SIZE * (path_len + 1 + p->keep_start + p->keep_end), AVOIDPATH_DYNMEM_STRING, &output);

	// Sentinel
	POLY_SET_POINT(oref, path_len + p->keep_start + p->keep_end, POLY_LAST_POINT, POLY_LAST_POINT);

	// Add original start and end points if needed
	if (p->keep_end)
		POLY_SET_POINT(oref, path_len + p->keep_start, p->end.x, p->end.y);
	if (p->keep_start)
		POLY_SET_POINT(oref, 0, p->start.x, p->start.y);

	i = path_len + p->keep_start - 1;

	if (unreachable) {
		// Return straight trajectory from start to end
		POLY_SET_POINT(oref, i - 1, p->vertex_start->v.x, p->vertex_start->v.y);
		POLY_SET_POINT(oref, i, p->vertex_end->v.x, p->vertex_end->v.y);
		return output;
	}

	vertex = p->vertex_end;
	while (vertex) {
		POLY_SET_POINT(oref, i, vertex->v.x, vertex->v.y);
		vertex = vertex->path_prev;
		i--;
	}

	if (s->debug_mode & (1 << SCIkAVOIDPATH_NR)) {
		sciprintf("[avoidpath] Returning path:");
		for (i = 0; i < path_len + p->keep_start + p->keep_end; i++) {
			Common::Point pt;
			POLY_GET_POINT(oref, i, pt.x, pt.y);
			sciprintf(" (%i, %i)", pt.x, pt.y);
		}
		sciprintf("\n");
	}

	return output;
}

reg_t kAvoidPath(EngineState *s, int funct_nr, int argc, reg_t *argv) {
	Common::Point start = Common::Point(SKPV(0), SKPV(1));

	if (s->debug_mode & (1 << SCIkAVOIDPATH_NR)) {
		gfxw_port_t *port = s->picture_port;

		if (!port->decorations) {
			port->decorations = gfxw_new_list(gfx_rect(0, 0, 320, 200), 0);
			port->decorations->set_visual(GFXW(port->decorations), port->visual);
		} else {
			port->decorations->free_contents(port->decorations);
		}
	}

	switch (argc) {

	case 3 : {
		reg_t retval;
		Polygon *polygon = convert_polygon(s, argv[2]);

		if (polygon->type == POLY_CONTAINED_ACCESS) {
			sciprintf("[avoidpath] Warning: containment test performed on contained access polygon\n");

			// Semantics unknown, assume barred access semantics
			polygon->type = POLY_BARRED_ACCESS;
		}

		retval = make_reg(0, contained(start, polygon) != CONT_OUTSIDE);
		free_polygon(polygon);
		return retval;
	}
	case 6 :
	case 7 : {
		Common::Point end = Common::Point(SKPV(2), SKPV(3));
		reg_t poly_list = argv[4];
		//int poly_list_size = UKPV(5);
		int opt = UKPV_OR_ALT(6, 1);
		reg_t output;
		PathfindingState *p;

		if (s->debug_mode & (1 << SCIkAVOIDPATH_NR)) {
			sciprintf("[avoidpath] Pathfinding input:\n");
			draw_point(s, start, 1);
			draw_point(s, end, 0);

			if (poly_list.segment) {
				print_input(s, poly_list, start, end, opt);
				draw_input(s, poly_list, start, end, opt);
			}
		}

		p = convert_polygon_set(s, poly_list, start, end, opt);

		if (intersecting_polygons(p)) {
			sciprintf("[avoidpath] Error: input set contains (self-)intersecting polygons\n");
			free_pf_state(p);
			p = NULL;
		}

		if (!p) {
			byte *oref;
			sciprintf("[avoidpath] Error: pathfinding failed for following input:\n");
			print_input(s, poly_list, start, end, opt);
			sciprintf("[avoidpath] Returning direct path from start point to end point\n");
			oref = s->seg_manager->allocDynmem(POLY_POINT_SIZE * 3,
			                                   AVOIDPATH_DYNMEM_STRING, &output);

			POLY_SET_POINT(oref, 0, start.x, start.y);
			POLY_SET_POINT(oref, 1, end.x, end.y);
			POLY_SET_POINT(oref, 2, POLY_LAST_POINT, POLY_LAST_POINT);

			return output;
		}

		visibility_graph(p);
		dijkstra(p);

		output = output_path(p, s);
		free_pf_state(p);

		// Memory is freed by explicit calls to Memory
		return output;
	}

	default:
		warning("Unknown AvoidPath subfunction %d",
		         argc);
		return NULL_REG;
		break;
	}
}

} // End of namespace Sci
