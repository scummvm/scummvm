/***************************************************************************
 kpathing.c Copyright (C) 2002-2006 Lars Skovlund, Walter van Niftrik


 This program may be modified and copied freely according to the terms of
 the GNU general public license (GPL), as long as the above copyright
 notice and the licensing information contained herein are preserved.

 Please refer to www.gnu.org for licensing details.

 This work is provided AS IS, without warranty of any kind, expressed or
 implied, including but not limited to the warranties of merchantibility,
 noninfringement, and fitness for a specific purpose. The author will not
 be held liable for any damage caused by this work or derivatives of it.

 By using this source code, you agree to the licensing terms as stated
 above.


 Please contact the maintainer for bug reports or inquiries.

 Current Maintainer:

     Walter van Niftrik [w.f.b.w.v.niftrik@stud.tue.nl]

***************************************************************************/

/* Detailed information on the implementation can be found in the report
** which can be downloaded from FIXME.
*/

#include <math.h>

#include "sci/include/engine.h"
#include "sci/include/aatree.h"
#include "sci/include/list.h"

#define POLY_LAST_POINT 0x7777
#define POLY_POINT_SIZE 4

#define POLY_GET_POINT(p, i, x, y) do { x = getInt16((p) + (i) * POLY_POINT_SIZE); \
					y = getInt16((p) + 2 + (i) * POLY_POINT_SIZE); \
} while (0)
#define POLY_SET_POINT(p, i, x, y) do { putInt16((p) + (i) * POLY_POINT_SIZE, x); \
					putInt16((p) + 2 + (i) * POLY_POINT_SIZE, y); \
} while (0)
#define POLY_GET_POINT_REG_T(p, i, x, y) do { x = KP_SINT((p)[(i) * 2]); \
					      y = KP_SINT((p)[(i) * 2 + 1]); \
} while (0)

/* SCI-defined polygon types */
#define POLY_TOTAL_ACCESS 0
#define POLY_NEAREST_ACCESS 1
#define POLY_BARRED_ACCESS 2
#define POLY_CONTAINED_ACCESS 3

/* Polygon containment types */
#define CONT_OUTSIDE 0
#define CONT_ON_EDGE 1
#define CONT_INSIDE 2

#define POINT_EQUAL(A, B) (((A).x == (B).x) && ((A).y == (B).y))

#define HUGE_DISTANCE 1e10

/* Visibility matrix */
#define VIS_MATRIX_ROW_SIZE(N) (((N) / 8) + ((N) % 8 ? 1 : 0))
#define SET_VISIBLE(S, P, Q) ((S)->vis_matrix)[(P) * VIS_MATRIX_ROW_SIZE((S)->vertices) \
                                 + (Q) / 8] |= (1 << ((Q) % 8))
#define IS_VISIBLE(S, P, Q) (((S)->vis_matrix[(P) * VIS_MATRIX_ROW_SIZE((S)->vertices) \
                                 + (Q) / 8] & (1 << ((Q) % 8))) != 0)

#define VERTEX_HAS_EDGES(V) ((V) != CLIST_NEXT(V, entries))

/* Error codes */
#define PF_OK 0
#define PF_ERROR -1
#define PF_FATAL -2

/* Floating point struct */
typedef struct pointf
{
	float x, y;
} pointf_t;

pointf_t
pointf(float x, float y)
{
	pointf_t p;

	p.x = x;
	p.y = y;

	return p;
}

pointf_t
to_pointf(point_t p)
{
	return pointf(p.x, p.y);
}

typedef struct vertex
{
	/* Location */
	point_t v;

	/* Index */
	int idx;

	/* Vertex list entry */
	CLIST_ENTRY(vertex) entries;

	/* Dijkstra list entry */
	LIST_ENTRY(vertex) dijkstra;

	/* Distance from starting vertex */
	float dist;

	/* Previous vertex in shortest path */
	struct vertex *path_prev;
} vertex_t;

typedef CLIST_HEAD(vertices_head, vertex) vertices_head_t;

typedef struct polygon
{
	/* Circular list of vertices */
	vertices_head_t vertices;

	/* Polygon list entry */
	LIST_ENTRY(polygon) entries;

	/* SCI polygon type */
	int type;
} polygon_t;

/* Pathfinding state */
typedef struct pf_state
{
	/* List of all polygons */
	LIST_HEAD(polygons_head, polygon) polygons;

	/* Original start and end points */
	point_t start, end;

	/* Flags for adding original points to final path */
	char keep_start, keep_end;

	/* Start and end points for pathfinding */
	vertex_t *vertex_start, *vertex_end;

	/* Array to quickly find vertices by idx */
	vertex_t **vertex_index;

	/* Visibility matrix */
	char *vis_matrix;

	/* Total number of vertices */
	int vertices;
} pf_state_t;

static vertex_t *vertex_cur;

/* Temporary hack to deal with points in reg_ts */
static int
polygon_is_reg_t(unsigned char *list, int size)
{
	int i;

	/* Check the first three reg_ts */
	for (i = 0; i < (size < 3 ? size : 3); i++)
		if ((((reg_t *) list) + i)->segment)
			/* Non-zero segment, cannot be reg_ts */
			return 0;

	/* First three segments were zero, assume reg_ts */
	return 1;
}

static point_t
read_point(unsigned char *list, int is_reg_t, int offset)
{
	point_t point;

	if (!is_reg_t) {
		POLY_GET_POINT(list, offset, point.x, point.y);
	} else {
		POLY_GET_POINT_REG_T((reg_t *) list, offset, point.x, point.y);
	}

	return point;
}


  /*** Debug functions ***/

static void
draw_line(state_t *s, point_t p1, point_t p2, int type)
{
	/* Colors for polygon debugging.
	** Green: Total access
	** Red : Barred access
	** Blue: Near-point access
	** Yellow: Contained access
	*/
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
	decorations->add((gfxw_container_t *) decorations, (gfxw_widget_t *) line);
}

static void
draw_point(state_t *s, point_t p, int start)
{
	/* Colors for starting and end point
	** Green: End point
	** Blue: Starting point
	*/
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

static void
draw_polygon(state_t *s, reg_t polygon)
{
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	int type = KP_UINT(GET_SEL32(polygon, type));
	point_t first, prev;
	unsigned char *list = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
	int is_reg_t = polygon_is_reg_t(list, size);
	int i;

	prev = first = read_point(list, is_reg_t, 0);

	for (i = 1; i < size; i++) {
		point_t point = read_point(list, is_reg_t, i);
		draw_line(s, prev, point, type);
		prev = point;
	}

	draw_line(s, prev, first, type % 3);
}

static void
draw_input(state_t *s, reg_t poly_list, point_t start, point_t end, int opt)
{
	list_t *list;
	node_t *node;

	draw_point(s, start, 1);
	draw_point(s, end, 0);

	if (!poly_list.segment)
		return;

	list = LOOKUP_LIST(poly_list);

	if (!list) {
		SCIkwarn(SCIkWARNING, "Could not obtain polygon list\n");
		return;
	}

	node = LOOKUP_NODE(list->first);

	while (node) {
		draw_polygon(s, node->value);
		node = LOOKUP_NODE(node->succ);
	}
}

static void
print_polygon(state_t *s, reg_t polygon)
{
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	int type = KP_UINT(GET_SEL32(polygon, type));
	int i;
	unsigned char *point_array = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
	int is_reg_t = polygon_is_reg_t(point_array, size);
	point_t point;

	sciprintf("%i:", type);

	for (i = 0; i < size; i++) {
		point = read_point(point_array, is_reg_t, i);
		sciprintf(" (%i, %i)", point.x, point.y);
	}

	point = read_point(point_array, is_reg_t, 0);
	sciprintf(" (%i, %i);\n", point.x, point.y);
}

static void
print_input(state_t *s, reg_t poly_list, point_t start, point_t end, int opt)
{
	list_t *list;
	node_t *node;

	sciprintf("Start point: (%i, %i)\n", start.x, start.y);
	sciprintf("End point: (%i, %i)\n", end.x, end.y);
	sciprintf("Optimization level: %i\n", opt);

	if (!poly_list.segment)
		return;

	list = LOOKUP_LIST(poly_list);

	if (!list) {
		SCIkwarn(SCIkWARNING, "Could not obtain polygon list\n");
		return;
	}

	sciprintf("Polygons:\n");
	node = LOOKUP_NODE(list->first);

	while (node) {
		print_polygon(s, node->value);
		node = LOOKUP_NODE(node->succ);
	}
}


  /*** Basic geometry functions ***/

static int
area(point_t a, point_t b, point_t c)
/* Computes the area of a triangle
** Parameters: (point_t) a, b, c: The points of the triangle
** Returns   : (int) The area multiplied by two
*/
{
	return (b.x - a.x) * (a.y - c.y) - (c.x - a.x) * (a.y - b.y);
}

static int
left(point_t a, point_t b, point_t c)
/* Determines whether or not a point is to the left of a directed line
** Parameters: (point_t) a, b: The directed line (a, b)
**             (point_t) c: The query point
** Returns   : (int) 1 if c is to the left of (a, b), 0 otherwise
*/
{
	return area(a, b, c) > 0;
}

static int
left_on(point_t a, point_t b, point_t c)
/* Determines whether or not a point is to the left of or collinear with a
** directed line
** Parameters: (point_t) a, b: The directed line (a, b)
**             (point_t) c: The query point
** Returns   : (int) 1 if c is to the left of or collinear with (a, b), 0
**                   otherwise
*/
{
	return area(a, b, c) >= 0;
}

static int
collinear(point_t a, point_t b, point_t c)
/* Determines whether or not three points are collinear
** Parameters: (point_t) a, b, c: The three points
** Returns   : (int) 1 if a, b, and c are collinear, 0 otherwise
*/
{
	return area(a, b, c) == 0;
}

static int
between(point_t a, point_t b, point_t c)
/* Determines whether or not a point lies on a line segment
** Parameters: (point_t) a, b: The line segment (a, b)
**             (point_t) c: The query point
** Returns   : (int) 1 if c lies on (a, b), 0 otherwise
*/
{
	if (!collinear(a, b, c))
		return 0;

	/* Assumes a != b. */
	if (a.x != b.x)
		return ((a.x <= c.x) && (c.x <= b.x)) || ((a.x >= c.x) && (c.x >= b.x));
	else
		return ((a.y <= c.y) && (c.y <= b.y)) || ((a.y >= c.y) && (c.y >= b.y));
}

static int
intersect_proper(point_t a, point_t b, point_t c, point_t d)
/* Determines whether or not two line segments properly intersect
** Parameters: (point_t) a, b: The line segment (a, b)
**             (point_t) c, d: The line segment (c, d)
** Returns   : (int) 1 if (a, b) properly intersects (c, d), 0 otherwise
*/
{
	int ab = (left(a, b, c) && left(b, a, d))
		  || (left(a, b, d) && left(b, a, c));
	int cd = (left(c, d, a) && left(d, c, b))
		  || (left(c, d, b) && left(d, c, a));

	return ab && cd;
}

static int
intersect(point_t a, point_t b, point_t c, point_t d)
/* Determines whether or not two line segments intersect
** Parameters: (point_t) a, b: The line segment (a, b)
**             (point_t) c, d: The line segment (c, d)
** Returns   : (int) 1 if (a, b) intersects (c, d), 0 otherwise
*/
{
	if (intersect_proper(a, b, c, d))
		return 1;

	return between(a, b, c) || between(a, b, d)
	       || between (c, d, a) || between(c, d, b);
}


  /*** Pathfinding ***/

static vertex_t *
vertex_new(point_t p)
/* Allocates and initialises a new vertex
** Parameters: (point_t) p: The position of the vertex
** Returns   : (vertex_t *) A newly allocated vertex
*/
{
	vertex_t *vertex = (vertex_t*)sci_malloc(sizeof(vertex_t));

	vertex->v = p;
	vertex->dist = HUGE_DISTANCE;
	vertex->path_prev = NULL;

	return vertex;
}

static polygon_t *
polygon_new(int type)
/* Allocates and initialises a new polygon
** Parameters: (int) type: The SCI polygon type
** Returns   : (polygon_t *) A newly allocated polygon
*/
{
	polygon_t *polygon = (polygon_t*)sci_malloc(sizeof(polygon_t));

	CLIST_INIT(&polygon->vertices);
	polygon->type = type;

	return polygon;
}

static int
contained(point_t p, polygon_t *polygon)
/* Polygon containment test
** Parameters: (point_t) p: The point
**             (polygon_t *) polygon: The polygon
** Returns   : (int) CONT_INSIDE if p is strictly contained in polygon,
**                   CONT_ON_EDGE if p lies on an edge of polygon,
**                   CONT_OUTSIDE otherwise
*/
{
	/* Number of ray crossing left and right */
	int lcross = 0, rcross = 0;
	vertex_t *vertex;

	/* Iterate over edges */
	CLIST_FOREACH(vertex, &polygon->vertices, entries) {
		point_t v1 = vertex->v;
		point_t v2 = CLIST_NEXT(vertex, entries)->v;

		/* Flags for ray straddling left and right */
		int rstrad, lstrad;

		/* Check if p is a vertex */
		if (POINT_EQUAL(p, v1))
			return CONT_ON_EDGE;

		/* Check if edge straddles the ray */
		rstrad = (v1.y < p.y) != (v2.y < p.y);
		lstrad = (v1.y > p.y) != (v2.y > p.y);

		if (lstrad || rstrad) {
			/* Compute intersection point x / xq */
			int x = v2.x * v1.y - v1.x * v2.y + (v1.x - v2.x) * p.y;
			int xq = v1.y - v2.y;

			/* Multiply by -1 if xq is negative (for comparison
			** that follows)
			*/
			if (xq < 0) {
				x = -x;
				xq = -xq;
			}

			/* Avoid floats by multiplying instead of dividing */
			if (rstrad && (x > xq * p.x))
				rcross++;
			else if (lstrad && (x < xq * p.x))
				lcross++;
		}
	}

	/* If we counted an odd number of total crossings the point is on an
	** edge
	*/
	if ((lcross + rcross) % 2 == 1)
		return CONT_ON_EDGE;

	/* If there are an odd number of crossings to one side the point is
	** contained in the polygon
	*/
	if (rcross % 2 == 1) {
		/* Invert result for contained access polygons. */
		if (polygon->type == POLY_CONTAINED_ACCESS)
			return CONT_OUTSIDE;
		return CONT_INSIDE;
	}

	/* Point is outside polygon. Invert result for contained access
	** polygons
	*/
	if (polygon->type == POLY_CONTAINED_ACCESS)
		return CONT_INSIDE;

	return CONT_OUTSIDE;
}

static int
polygon_area(polygon_t *polygon)
/* Computes polygon area
** Parameters: (polygon_t *) polygon: The polygon
** Returns   : (int) The area multiplied by two
*/
{
	vertex_t *first = CLIST_FIRST(&polygon->vertices);
	vertex_t *v;
	int size = 0;

	v = CLIST_NEXT(first, entries);

	while (CLIST_NEXT(v, entries) != first) {
		size += area(first->v, v->v, CLIST_NEXT(v, entries)->v);
		v = CLIST_NEXT(v, entries);
	}

	return size;
}

static void
fix_vertex_order(polygon_t *polygon)
/* Fixes the vertex order of a polygon if incorrect. Contained access
** polygons should have their vertices ordered clockwise, all other types
** anti-clockwise
** Parameters: (polygon_t *) polygon: The polygon
** Returns   : (void)
*/
{
	int area = polygon_area(polygon);

	/* When the polygon area is positive the vertices are ordered
	** anti-clockwise. When the area is negative the vertices are ordered
	** clockwise
	*/
	if (((area > 0) && (polygon->type == POLY_CONTAINED_ACCESS))
	    || ((area < 0) && (polygon->type != POLY_CONTAINED_ACCESS))) {
		vertices_head_t vertices;

		/* Create a new circular list */
		CLIST_INIT(&vertices);

		while (!CLIST_EMPTY(&polygon->vertices)) {
			/* Put first vertex in new list */
			vertex_t *vertex = CLIST_FIRST(&polygon->vertices);
			CLIST_REMOVE(&polygon->vertices, vertex, entries);
			CLIST_INSERT_HEAD(&vertices, vertex, entries);
		}

		polygon->vertices = vertices;
	}
}

static int
vertex_compare(const void *a, const void *b)
/* Compares two vertices by angle (first) and distance (second) in relation
** to vertex_cur. The angle is relative to the horizontal line extending
** right from vertex_cur, and increases clockwise
** Parameters: (const void *) a, b: The vertices
** Returns   : (int) -1 if a is smaller than b, 1 if a is larger than b, and
**                   0 if a and b are equal
*/
{
	point_t p0 = vertex_cur->v;
	point_t p1 = (*(vertex_t **) a)->v;
	point_t p2 = (*(vertex_t **) b)->v;

	if (POINT_EQUAL(p1, p2))
		return 0;

	/* Points above p0 have larger angle than points below p0 */
	if ((p1.y < p0.y) && (p2.y >= p0.y))
		return 1;

	if ((p2.y < p0.y) && (p1.y >= p0.y))
		return -1;

	/* Handle case where all points have the same y coordinate */
	if ((p0.y == p1.y) && (p0.y == p2.y)) {
		/* Points left of p0 have larger angle than points right of
		** p0
		*/
		if ((p1.x < p0.x) && (p2.x >= p0.x))
			return 1;
		if ((p1.x >= p0.x) && (p2.x < p0.x))
			return -1;
	}

	if (collinear(p0, p1, p2)) {
		/* At this point collinear points must have the same angle,
		** so compare distance to p0
		*/
		if (abs(p1.x - p0.x) < abs(p2.x - p0.x))
			return -1;
		if (abs(p1.y - p0.y) < abs(p2.y - p0.y))
			return -1;

		return 1;
	}

	/* If p2 is left of the directed line (p0, p1) then p1 has greater
	** angle
	*/
	if (left(p0, p1, p2))
		return 1;

	return -1;
}

static void
clockwise(vertex_t *v, point_t *p1, point_t *p2)
/* Orders the points of an edge clockwise around vertex_cur. If all three
** points are collinear the original order is used
** Parameters: (vertex_t *) v: The first vertex of the edge
** Returns   : (void)
**             (point_t) *p1: The first point in clockwise order
**             (point_t) *p2: The second point in clockwise order
*/
{
	vertex_t *w = CLIST_NEXT(v, entries);

	if (left_on(vertex_cur->v, w->v, v->v)) {
		*p1 = v->v;
		*p2 = w->v;
		return;
	}

	*p1 = w->v;
	*p2 = v->v;
	return;
}

static int
edge_compare(const void *a, const void *b)
/* Compares two edges that are intersected by the sweeping line by distance
** from vertex_cur
** Parameters: (const void *) a, b: The first vertices of the edges
** Returns   : (int) -1 if a is closer than b, 1 if b is closer than a, and
**                   0 if a and b are equal
*/
{
	point_t v1, v2, w1, w2;

	/* We can assume that the sweeping line intersects both edges and
	** that the edges do not properly intersect
	*/

	if (a == b)
		return 0;

	/* Order vertices clockwise so we know vertex_cur is to the right of
	** directed edges (v1, v2) and (w1, w2)
	*/
	clockwise((vertex_t *) a, &v1, &v2);
	clockwise((vertex_t *) b, &w1, &w2);

	/* As the edges do not properly intersect one edge must lie entirely
	** to one side of another. Note that the special case where edges are
	** collinear does not need to be handled as those edges will never be
	** in the tree simultaneously
	*/

	/* b is left of a */
	if (left_on(v1, v2, w1) && left_on(v1, v2, w2))
		return -1;

	/* b is right of a */
	if (left_on(v2, v1, w1) && left_on(v2, v1, w2))
		return 1;

	/* a is left of b */
	if (left_on(w1, w2, v1) && left_on(w1, w2, v2))
		return 1;

	/* a is right of b */
	return -1;
}

static int
inside(point_t p, vertex_t *vertex)
/* Determines whether or not a line from a point to a vertex intersects the
** interior of the polygon, locally at that vertex
** Parameters: (point_t) p: The point
**             (vertex_t *) vertex: The vertex
** Returns   : (int) 1 if the line (p, vertex->v) intersects the interior of
**                   the polygon, locally at the vertex. 0 otherwise
*/
{
	/* Check that it's not a single-vertex polygon */
	if (VERTEX_HAS_EDGES(vertex)) {
		point_t prev = CLIST_PREV(vertex, entries)->v;
		point_t next = CLIST_NEXT(vertex, entries)->v;
		point_t cur = vertex->v;

		if (left(prev, cur, next)) {
			/* Convex vertex, line (p, cur) intersects the inside
			** if p is located left of both edges
			*/
			if (left(cur, next, p) && left(prev, cur, p))
				return 1;
		} else {
			/* Non-convex vertex, line (p, cur) intersects the
			** inside if p is located left of either edge
			*/
			if (left(cur, next, p) || left(prev, cur, p))
				return 1;
		}
	}

	return 0;
}

static int
visible(vertex_t *vertex, vertex_t *vertex_prev, int visible, aatree_t *tree)
/* Determines whether or not a vertex is visible from vertex_cur
** Parameters: (vertex_t *) vertex: The vertex
**             (vertex_t *) vertex_prev: The previous vertex in the sort
**                                       order, or NULL
**             (int) visible: 1 if vertex_prev is visible, 0 otherwise
**             (aatree_t *) tree: The tree of edges intersected by the
**                                sweeping line
** Returns   : (int) 1 if vertex is visible from vertex_cur, 0 otherwise
*/
{
	vertex_t *edge;
	point_t p = vertex_cur->v;
	point_t w = vertex->v;
	aatree_t *tree_n = tree;

	/* Check if sweeping line intersects the interior of the polygon
	** locally at vertex
	*/
	if (inside(p, vertex))
		return 0;

	/* If vertex_prev is on the sweeping line, then vertex is invisible
	** if vertex_prev is invisible
	*/
	if (vertex_prev && !visible && between(p, w, vertex_prev->v))
			return 0;

	/* Find leftmost node of tree */
	while ((tree_n = aatree_walk(tree_n, AATREE_WALK_LEFT)))
		tree = tree_n;
	edge = (vertex_t*)aatree_get_data(tree);

	if (edge) {
		point_t p1, p2;

		/* Check for intersection with sweeping line before vertex */
		clockwise(edge, &p1, &p2);
		if (left(p2, p1, p) && left(p1, p2, w))
			return 0;
	}

	return 1;
}

static void
visible_vertices(pf_state_t *s, vertex_t *vert)
/* Determines all vertices that are visible from a particular vertex and
** updates the visibility matrix
** Parameters: (pf_state_t *) s: The pathfinding state
**             (vertex_t *) vert: The vertex
** Returns   : (void)
*/
{
	aatree_t *tree = aatree_new();
	point_t p = vert->v;
	polygon_t *polygon;
	int i;
	int is_visible;
	vertex_t **vert_sorted = (vertex_t**)sci_malloc(sizeof(vertex_t *) * s->vertices);

	/* Sort vertices by angle (first) and distance (second) */
	memcpy(vert_sorted, s->vertex_index, sizeof(vertex_t *) * s->vertices);
	vertex_cur = vert;
	qsort(vert_sorted, s->vertices, sizeof(vertex_t *), vertex_compare);

	LIST_FOREACH(polygon, &s->polygons, entries) {
		vertex_t *vertex;

		vertex = CLIST_FIRST(&polygon->vertices);

		/* Check that there is more than one vertex. */
		if (VERTEX_HAS_EDGES(vertex))
			CLIST_FOREACH(vertex, &polygon->vertices, entries) {
				point_t high, low;

				/* Add edges that intersect the initial position of the sweeping line */
				clockwise(vertex, &high, &low);

				if ((high.y < p.y) && (low.y >= p.y) && !POINT_EQUAL(low, p))
					aatree_insert(vertex, &tree, edge_compare);
			}
	}

	is_visible = 1;

	/* The first vertex will be vertex_cur, so we skip it */
	for (i = 1; i < s->vertices; i++) {
		vertex_t *v1;

		/* Compute visibility of vertex_index[i] */
		is_visible = visible(vert_sorted[i], vert_sorted[i - 1], is_visible, tree);

		/* Update visibility matrix */
		if (is_visible)
			SET_VISIBLE(s, vert->idx, vert_sorted[i]->idx);

		/* Delete anti-clockwise edges from tree */
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

		/* Add clockwise edges of collinear vertices when sweeping line moves */
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

	sci_free(vert_sorted);

	/* Free tree */
	aatree_free(tree);
}

static float
distance(pointf_t a, pointf_t b)
/* Computes the distance between two pointfs
** Parameters: (point_t) a, b: The two pointfs
** Returns   : (int) The distance between a and b, rounded to int
*/
{
	float w = a.x - b.x;
	float h = a.y - b.y;

	return sqrt(w * w + h * h);
}

static int
point_on_screen_border(point_t p)
/* Determines if a point lies on the screen border
** Parameters: (point_t) p: The point
** Returns   : (int) 1 if p lies on the screen border, 0 otherwise
*/
{
	/* FIXME get dimensions from somewhere? */
	return (p.x == 0) || (p.x == 319) || (p.y == 0) || (p.y == 189);
}

static int
edge_on_screen_border(point_t p, point_t q)
/* Determines if an edge lies on the screen border
** Parameters: (point_t) p, q: The edge (p, q)
** Returns   : (int) 1 if (p, q) lies on the screen border, 0 otherwise
*/
{
	/* FIXME get dimensions from somewhere? */
	return ((p.x == 0 && q.x == 0)
	    || (p.x == 319 && q.x == 319)
	    || (p.y == 0 && q.y == 0)
	    || (p.y == 189 && q.y == 189));
}

static int
find_free_point(pointf_t f, polygon_t *polygon, point_t *ret)
/* Searches for a nearby point that is not contained in a polygon
** Parameters: (pointf_t) f: The pointf to search nearby
**             (polygon_t *) polygon: The polygon
** Returns   : (int) PF_OK on success, PF_FATAL otherwise
**             (point_t) *ret: The non-contained point on success
*/
{
	point_t p;

	/* Try nearest point first */
	p = gfx_point((int) floor(f.x + 0.5),
		      (int) floor(f.y + 0.5));

	if (contained(p, polygon) != CONT_INSIDE) {
		*ret = p;
		return PF_OK;
	}

	p = gfx_point((int) floor(f.x),
		      (int) floor(f.y));

	/* Try (x, y), (x + 1, y), (x , y + 1) and (x + 1, y + 1) */
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

static int
near_point(point_t p, polygon_t *polygon, point_t *ret)
/* Computes the near point of a point contained in a polygon
** Parameters: (point_t) p: The point
**             (polygon_t *) polygon: The polygon
** Returns   : (int) PF_OK on success, PF_FATAL otherwise
**             (point_t) *ret: The near point of p in polygon on success
*/
{
	vertex_t *vertex;
	pointf_t near_p;
	float dist = HUGE_DISTANCE;

	CLIST_FOREACH(vertex, &polygon->vertices, entries) {
		point_t p1 = vertex->v;
		point_t p2 = CLIST_NEXT(vertex, entries)->v;
		float w, h, l, u;
		pointf_t new_point;
		float new_dist;

		/* Ignore edges on the screen border */
		if (edge_on_screen_border(p1, p2))
			continue;

		/* Compute near point */
		w = p2.x - p1.x;
		h = p2.y - p1.y;
		l = sqrt(w * w + h * h);
		u = ((p.x - p1.x) * (p2.x - p1.x) + (p.y - p1.y) * (p2.y - p1.y)) / (l * l);

		/* Clip to edge */
		if (u < 0.0f)
			u = 0.0f;
		if (u > 1.0f)
			u = 1.0f;

		new_point.x = p1.x + u * (p2.x - p1.x);
		new_point.y = p1.y + u * (p2.y - p1.y);

		new_dist = distance(to_pointf(p), new_point);

		if (new_dist < dist) {
			near_p = new_point;
			dist = new_dist;
		}
	}

	/* Find point not contained in polygon */
	return find_free_point(near_p, polygon, ret);
}

static int
intersection(point_t a, point_t b, vertex_t *vertex, pointf_t *ret)
/* Computes the intersection point of a line segment and an edge (not
** including the vertices themselves)
** Parameters: (point_t) a, b: The line segment (a, b)
**             (vertex_t *) vertex: The first vertex of the edge
** Returns   : (int) FP_OK on success, PF_ERROR otherwise
**             (pointf_t) *ret: The intersection point
*/
{
	/* Parameters of parametric equations */
	float s, t;
	/* Numerator and denominator of equations */
	float num, denom;
	point_t c = vertex->v;
	point_t d = CLIST_NEXT(vertex, entries)->v;

	denom = a.x * (float) (d.y - c.y) +
		b.x * (float) (c.y - d.y) +
		d.x * (float) (b.y - a.y) +
		c.x * (float) (a.y - b.y);

	if (denom == 0.0)
		/* Segments are parallel, no intersection */
		return PF_ERROR;

	num = a.x * (float) (d.y - c.y) +
	      c.x * (float) (a.y - d.y) +
	      d.x * (float) (c.y - a.y);

	s = num / denom;

	num = -(a.x * (float) (c.y - b.y) +
		b.x * (float) (a.y - c.y) +
		c.x * (float) (b.y - a.y));

	t = num / denom;

	if ((0.0 <= s) && (s <= 1.0) && (0.0 < t) && (t < 1.0)) {
		/* Intersection found */
		ret->x = a.x + s * (b.x - a.x);
		ret->y = a.y + s * (b.y - a.y);
		return PF_OK;
	}

	return PF_ERROR;
}

static int
nearest_intersection(pf_state_t *s, point_t p, point_t q, point_t *ret)
/* Computes the nearest intersection point of a line segment and the polygon
** set. Intersection points that are reached from the inside of a polygon
** are ignored as are improper intersections which do not obstruct
** visibility
** Parameters: (pf_state_t *) s: The pathfinding state
**             (point_t) p, q: The line segment (p, q)
** Returns   : (int) PF_OK on success, PF_ERROR when no intersections were
**                   found, PF_FATAL otherwise
**             (point_t) *ret: On success, the closest intersection point
*/
{
	polygon_t *polygon = 0;
	pointf_t isec;
	polygon_t *ipolygon = 0;
	float dist = HUGE_DISTANCE;

	LIST_FOREACH(polygon, &s->polygons, entries) {
		vertex_t *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			float new_dist;
			pointf_t new_isec;

			/* Check for intersection with vertex */
			if (between(p, q, vertex->v)) {
				/* Skip this vertex if we hit it from the
				** inside of the polygon
				*/
				if (inside(q, vertex)) {
					new_isec.x = vertex->v.x;
					new_isec.y = vertex->v.y;
				} else
					continue;
			} else {
				/* Check for intersection with edges */

				/* Skip this edge if we hit it from the
				** inside of the polygon
				*/
				if (!left(vertex->v, CLIST_NEXT(vertex, entries)->v, q))
					continue;

				if (intersection(p, q, vertex, &new_isec) != PF_OK)
					continue;
			}

			new_dist = distance(to_pointf(p), new_isec);
			if (new_dist < dist) {
				ipolygon = polygon;
				isec = new_isec;
				dist = new_dist;
			}
		}
	}

	if (dist == HUGE_DISTANCE)
		return PF_ERROR;

	/* Find point not contained in polygon */
	return find_free_point(isec, ipolygon, ret);
}

static int
fix_point(pf_state_t *s, point_t p, point_t *ret, polygon_t **ret_pol)
/* Checks a point for containment in any of the polygons in the polygon set.
** If the point is contained in a totally accessible polygon that polygon
** is removed from the set. If the point is contained in a polygon of another
** type the near point is returned. Otherwise the original point is returned
** Parameters: (point_t) p: The point
** Returns   : (int) PF_OK on success, PF_FATAL otherwise
**             (point_t) *ret: A valid input point for pathfinding
**             (polygon_t *) *ret_pol: The polygon p was contained in if p
**                                     != *ret, NULL otherwise
*/
{
	polygon_t *polygon;
	*ret_pol = NULL;

	/* Check for polygon containment */
	LIST_FOREACH(polygon, &s->polygons, entries) {
		if (contained(p, polygon) != CONT_OUTSIDE)
			break;
	}

	if (polygon) {
		point_t near_p;

		if (polygon->type == POLY_TOTAL_ACCESS) {
			/* Remove totally accessible polygon if it contains
			** p
			*/
			LIST_REMOVE(polygon, entries);
			*ret = p;
			return PF_OK;
		}

		/* Otherwise, compute near point */
		if (near_point(p, polygon, &near_p) == PF_OK) {
			*ret = near_p;

			if (!POINT_EQUAL(p, *ret))
				*ret_pol = polygon;

			return PF_OK;
		}

		return PF_FATAL;
	}

	/* p is not contained in any polygon */
	*ret = p;
	return PF_OK;
}

static vertex_t *
merge_point(pf_state_t *s, point_t v)
/* Merges a point into the polygon set. A new vertex is allocated for this
** point, unless a matching vertex already exists. If the point is on an
** already existing edge that edge is split up into two edges connected by
** the new vertex
** Parameters: (pf_state_t *) s: The pathfinding state
**             (point_t) v: The point to merge
** Returns   : (vertex_t *) The vertex corresponding to v
*/
{
	vertex_t *vertex;
	vertex_t *v_new;
	polygon_t *polygon;

	/* Check for already existing vertex */
	LIST_FOREACH(polygon, &s->polygons, entries) {
		CLIST_FOREACH(vertex, &polygon->vertices, entries)
			if (POINT_EQUAL(vertex->v, v))
				return vertex;
	}

	v_new = vertex_new(v);

	/* Check for point being on an edge */
	LIST_FOREACH(polygon, &s->polygons, entries)
		/* Skip single-vertex polygons */
		if (VERTEX_HAS_EDGES(CLIST_FIRST(&polygon->vertices)))
			CLIST_FOREACH(vertex, &polygon->vertices, entries) {
				vertex_t *next = CLIST_NEXT(vertex, entries);

				if (between(vertex->v, next->v, v)) {
					/* Split edge by adding vertex */
					CLIST_INSERT_AFTER(vertex, v_new, entries);
					return v_new;
				}
			}

	/* Add point as single-vertex polygon */
	polygon = polygon_new(POLY_BARRED_ACCESS);
	CLIST_INSERT_HEAD(&polygon->vertices, v_new, entries);
	LIST_INSERT_HEAD(&s->polygons, polygon, entries);

	return v_new;
}

static polygon_t *
convert_polygon(state_t *s, reg_t polygon)
/* Converts an SCI polygon into a polygon_t
** Parameters: (state_t *) s: The game state
**             (reg_t) polygon: The SCI polygon to convert
** Returns   : (polygon_t *) The converted polygon
*/
{
	int i;
	reg_t points = GET_SEL32(polygon, points);
	int size = KP_UINT(GET_SEL32(polygon, size));
	unsigned char *list = kernel_dereference_bulk_pointer(s, points, size * POLY_POINT_SIZE);
	polygon_t *poly = polygon_new(KP_UINT(GET_SEL32(polygon, type)));
	int is_reg_t = polygon_is_reg_t(list, size);

	for (i = 0; i < size; i++) {
		vertex_t *vertex = vertex_new(read_point(list, is_reg_t, i));
		CLIST_INSERT_HEAD(&poly->vertices, vertex, entries);
	}

	fix_vertex_order(poly);

	return poly;
}

static void
free_polygon(polygon_t *polygon)
/* Frees a polygon and its vertices
** Parameters: (polygon_t *) polygons: The polygon
** Returns   : (void)
*/
{
	while (!CLIST_EMPTY(&polygon->vertices)) {
		vertex_t *vertex = CLIST_FIRST(&polygon->vertices);
		CLIST_REMOVE(&polygon->vertices, vertex, entries);
		sci_free(vertex);
	}

	sci_free(polygon);
}

static void
free_pf_state(pf_state_t *p)
/* Frees a pathfinding state
** Parameters: (pf_state_t *) p: The pathfinding state
** Returns   : (void)
*/
{
	if (p->vertex_index)
		sci_free(p->vertex_index);

	if (p->vis_matrix)
		sci_free(p->vis_matrix);

	while (!LIST_EMPTY(&p->polygons)) {
		polygon_t *polygon = LIST_FIRST(&p->polygons);
		LIST_REMOVE(polygon, entries);
		free_polygon(polygon);
	}

	sci_free(p);
}

static void
change_polygons_opt_0(pf_state_t *s)
/* Changes the polygon list for optimization level 0 (used for keyboard
** support). Totally accessible polygons are removed and near-point
** accessible polygons are changed into totally accessible polygons.
** Parameters: (pf_state_t *) s: The pathfinding state
** Returns   : (void)
*/
{
	polygon_t *polygon = LIST_FIRST(&s->polygons);

	while (polygon) {
		polygon_t *next = LIST_NEXT(polygon, entries);

		if (polygon->type == POLY_NEAREST_ACCESS)
			polygon->type = POLY_TOTAL_ACCESS;
		else  if (polygon->type == POLY_TOTAL_ACCESS) {
			LIST_REMOVE(polygon, entries);
			free_polygon(polygon);
		}

		polygon = next;
	}
}

static pf_state_t *
convert_polygon_set(state_t *s, reg_t poly_list, point_t start, point_t end, int opt)
/* Converts the SCI input data for pathfinding
** Parameters: (state_t *) s: The game state
**             (reg_t) poly_list: Polygon list
**             (point_t) start: The start point
**             (point_t) end: The end point
**             (int) opt: Optimization level (0, 1 or 2)
** Returns   : (pf_state_t *) On success a newly allocated pathfinding state,
**                            NULL otherwise
*/
{
	polygon_t *polygon;
	int err;
	int count = 0;
	pf_state_t *pf_s = (pf_state_t*)sci_malloc(sizeof(pf_state_t));

	LIST_INIT(&pf_s->polygons);
	pf_s->start = start;
	pf_s->end = end;
	pf_s->keep_start = 0;
	pf_s->keep_end = 0;
	pf_s->vertex_index = NULL;

	/* Convert all polygons */
	if (poly_list.segment) {
		list_t *list = LOOKUP_LIST(poly_list);
		node_t *node = LOOKUP_NODE(list->first);

		while (node) {
			polygon = convert_polygon(s, node->value);
			LIST_INSERT_HEAD(&pf_s->polygons, polygon, entries);
			count += KP_UINT(GET_SEL32(node->value, size));
			node = LOOKUP_NODE(node->succ);
		}
	}

	if (opt == 0) {
		/* Keyboard support */
		change_polygons_opt_0(pf_s);

		/* Find nearest intersection */
		err = nearest_intersection(pf_s, start, end, &start);

		if (err == PF_FATAL) {
			sciprintf("[avoidpath] Error: fatal error finding nearest intersecton\n");
			free_pf_state(pf_s);
			return NULL;
		}
		else if (err == PF_OK)
			/* Keep original start position if intersection
			** was found
			*/
			pf_s->keep_start = 1;
	} else {
		if (fix_point(pf_s, start, &start, &polygon) != PF_OK) {
			sciprintf("[avoidpath] Error: couldn't fix start position for pathfinding\n");
			free_pf_state(pf_s);
			return NULL;
		}
		else if (polygon) {
			/* Start position has moved */
			pf_s->keep_start = 1;
			if ((polygon->type != POLY_NEAREST_ACCESS))
				sciprintf("[avoidpath] Warning: start position at unreachable location\n");
		}
	}

	if (fix_point(pf_s, end, &end, &polygon) != PF_OK) {
		sciprintf("[avoidpath] Error: couldn't fix end position for pathfinding\n");
		free_pf_state(pf_s);
		return NULL;
	}
	else {
		/* Keep original end position if it is contained in a
		** near-point accessible polygon
		*/
		if (polygon && (polygon->type == POLY_NEAREST_ACCESS))
			pf_s->keep_end = 1;
	}

	/* Merge start and end points into polygon set */
	pf_s->vertex_start = merge_point(pf_s, start);
	pf_s->vertex_end = merge_point(pf_s, end);

	/* Allocate and build vertex index */
	pf_s->vertex_index = (vertex_t**)sci_malloc(sizeof(vertex_t *) * (count + 2));

	count = 0;

	LIST_FOREACH(polygon, &pf_s->polygons, entries) {
		vertex_t *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries) {
			vertex->idx = count;
			pf_s->vertex_index[count++] = vertex;
		}
	}

	pf_s->vertices = count;

	/* Allocate and clear visibility matrix */
	pf_s->vis_matrix = (char*)sci_calloc(pf_s->vertices * VIS_MATRIX_ROW_SIZE(pf_s->vertices), 1);

	return pf_s;
}

static void
visibility_graph(pf_state_t *s)
/* Computes the visibility graph
** Parameters: (pf_state_t *) s: The pathfinding state
** Returns   : (void)
*/
{
	polygon_t *polygon;

	LIST_FOREACH(polygon, &s->polygons, entries) {
		vertex_t *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries)
			visible_vertices(s, vertex);
	}
}

static int
intersecting_polygons(pf_state_t *s)
/* Detects (self-)intersecting polygons
** Parameters: (pf_state_t *) s: The pathfinding state
** Returns   : (int) 1 if s contains (self-)intersecting polygons, 0 otherwise
*/
{
	int i, j;

	for (i = 0; i < s->vertices; i++) {
		vertex_t *v1 = s->vertex_index[i];
		if (!VERTEX_HAS_EDGES(v1))
			continue;
		for (j = i + 1; j < s->vertices; j++) {
			vertex_t *v2 = s->vertex_index[j];
			if (!VERTEX_HAS_EDGES(v2))
				continue;

			/* Skip neighbouring edges */
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

static void
dijkstra(pf_state_t *s)
/* Computes a shortest path from vertex_start to vertex_end. The caller can
** construct the resulting path by following the path_prev links from
** vertex_end back to vertex_start. If no path exists vertex_end->path_prev
** will be NULL
** Parameters: (pf_state_t *) s: The pathfinding state
** Returns   : (void)
*/
{
	polygon_t *polygon;
	/* Vertices of which the shortest path is known */
	LIST_HEAD(done_head, vertex) done;
	/* The remaining vertices */
	LIST_HEAD(remain_head, vertex) remain;

	LIST_INIT(&remain);
	LIST_INIT(&done);

	/* Start out with all vertices in set remain */
	LIST_FOREACH(polygon, &s->polygons, entries) {
		vertex_t *vertex;

		CLIST_FOREACH(vertex, &polygon->vertices, entries)
			LIST_INSERT_HEAD(&remain, vertex, dijkstra);
	}

	s->vertex_start->dist = 0.0f;

	/* Loop until we find vertex_end */
	while (1) {
		int i;
		vertex_t *vertex, *vertex_min = 0;
		float min = HUGE_DISTANCE;

		/* Find vertex at shortest distance from set done */
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

		/* If vertex_end is at shortest distance we can stop */
		if (vertex_min == s->vertex_end)
			return;

		/* Move vertex from set remain to set done */
		LIST_REMOVE(vertex_min, dijkstra);
		LIST_INSERT_HEAD(&done, vertex_min, dijkstra);

		for (i = 0; i < s->vertices; i++) {
			/* Adjust upper bound for all vertices that are visible from vertex_min */
			if (IS_VISIBLE(s, vertex_min->idx, i)) {
				float new_dist;

				/* Avoid plotting path along screen edge */
				if ((s->vertex_index[i] != s->vertex_end) && point_on_screen_border(s->vertex_index[i]->v))
					continue;

				new_dist = vertex_min->dist + distance(to_pointf(vertex_min->v),
								       to_pointf(s->vertex_index[i]->v));
				if (new_dist < s->vertex_index[i]->dist) {
					s->vertex_index[i]->dist = new_dist;
					s->vertex_index[i]->path_prev = vertex_min;
				}
			}
		}
	}
}

static reg_t
output_path(pf_state_t *p, state_t *s)
/* Stores the final path in newly allocated dynmem
** Parameters: (pf_state_t *) p: The pathfinding state
**             (state_t *) s: The game state
** Returns   : (reg_t) Pointer to dynmem containing path
*/
{
	int path_len = 0;
	byte *oref;
	reg_t output;
	vertex_t *vertex = p->vertex_end;
	int i;
	int unreachable = vertex->path_prev == NULL;

	if (unreachable) {
		/* If pathfinding failed we only return the path up to vertex_start */
		oref = sm_alloc_dynmem(&s->seg_manager, POLY_POINT_SIZE * 3,
				AVOIDPATH_DYNMEM_STRING, &output);

		if (p->keep_start)
			POLY_SET_POINT(oref, 0, p->start.x, p->start.y);
		else
			POLY_SET_POINT(oref, 0, p->vertex_start->v.x, p->vertex_start->v.y);
		POLY_SET_POINT(oref, 1, p->vertex_start->v.x, p->vertex_start->v.y);
		POLY_SET_POINT(oref, 2, POLY_LAST_POINT, POLY_LAST_POINT);

		return output;
	}

	while (vertex) {
		/* Compute path length */
		path_len++;
		vertex = vertex->path_prev;
	}

	oref = sm_alloc_dynmem(&s->seg_manager, POLY_POINT_SIZE * (path_len + 1 + p->keep_start + p->keep_end),
			AVOIDPATH_DYNMEM_STRING, &output);

	/* Sentinel */
	POLY_SET_POINT(oref, path_len + p->keep_start + p->keep_end, POLY_LAST_POINT, POLY_LAST_POINT);

	/* Add original start and end points if needed */
	if (p->keep_end)
		POLY_SET_POINT(oref, path_len + p->keep_start, p->end.x, p->end.y);
	if (p->keep_start)
		POLY_SET_POINT(oref, 0, p->start.x, p->start.y);

	i = path_len + p->keep_start - 1;

	if (unreachable) {
		/* Return straight trajectory from start to end */
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
			point_t pt;
			POLY_GET_POINT(oref, i, pt.x, pt.y);
			sciprintf(" (%i, %i)", pt.x, pt.y);
		}
		sciprintf("\n");
	}

	return output;
}

reg_t
kAvoidPath(state_t *s, int funct_nr, int argc, reg_t *argv)
{
	point_t start = gfx_point(SKPV(0), SKPV(1));

	if (s->debug_mode & (1 << SCIkAVOIDPATH_NR)) {
		gfxw_port_t *port= s->picture_port;

		if (!port->decorations) {
			port->decorations = gfxw_new_list(gfx_rect(0, 0, 320, 200), 0);
			port->decorations->set_visual(GFXW(port->decorations), port->visual);
		} else {
			port->decorations->free_contents(port->decorations);
		}
	}

	switch (argc) {

	case 3 :
	{
		reg_t retval;
		polygon_t *polygon = convert_polygon(s, argv[2]);

		if (polygon->type == POLY_CONTAINED_ACCESS) {
			sciprintf("[avoidpath] Warning: containment test performed on contained access polygon\n");

			/* Semantics unknown, assume barred access semantics */
			polygon->type = POLY_BARRED_ACCESS;
		}

		retval = make_reg(0, contained(start, polygon) != CONT_OUTSIDE);
		free_polygon(polygon);
		return retval;
	}
	case 6 :
	case 7 :
	{
		point_t end = gfx_point(SKPV(2), SKPV(3));
		reg_t poly_list = argv[4];
		/* int poly_list_size = UKPV(5); */
		int opt = UKPV_OR_ALT(6, 1);
		reg_t output;
		pf_state_t *p;

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
			oref = sm_alloc_dynmem(&s->seg_manager, POLY_POINT_SIZE*3,
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

		/* Memory is freed by explicit calls to Memory */
		return output;
	}

	default:
		SCIkwarn(SCIkWARNING, "Unknown AvoidPath subfunction %d\n",
			 argc);
		return NULL_REG;
		break;
	}
}
