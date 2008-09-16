/*
    SEL - Simple DirectMedia Layer Extension Library
    Copyright (C) 2002 Matej Knopp <knopp@users.sf.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* microtile arrays implementation */

/* 
 * This is a microtile array implementation similiar to the one
 * from LibArt_LGPL. First, I wanted to use that one but unfortunately
 * I fount out that it suffered from a bad design and simply didn't meet
 * my requirements so I decided to write the implementation on my own.
 */

#include "picture/microtiles.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"

namespace Picture {

typedef struct _GemUta GemUta;
typedef struct _GemRect GemRect;

typedef uint32 GemUtaBBox;

#define GEM_UTA_BBOX_CONS(x0, y0, x1, y1)  (((x0) << 24) | ((y0) << 16) | \
                                            ((x1) << 8) | (y1))

#define GEM_UTILE_EMPTY   0x00000000
#define GEM_UTILE_FULL    0x01012020

#define GEM_UTA_BBOX_X0(ub) ((ub) >> 24)
#define GEM_UTA_BBOX_Y0(ub) (((ub) >> 16) & 0xff)
#define GEM_UTA_BBOX_X1(ub) (((ub) >> 8) & 0xff)
#define GEM_UTA_BBOX_Y1(ub) ((ub) & 0xff)

#define GEM_UTILE_SHIFT     5
#define GEM_UTILE_SIZE      (1 << GEM_UTILE_SHIFT)

#define GEM_MIN(a,b) ((a) < (b) ? (a) : (b))
#define GEM_MAX(a,b) ((a) > (b) ? (a) : (b))

struct _GemUta {
    int x;
    int y;
    int width;    /* number of tiles in one line */
    int height;   /* number of tiles in one row */
    GemUtaBBox *tiles;
};

struct _GemRect {
    int x0;
	int x1;
	int y0;
	int y1;
};


static GemUta    * gem_uta_new        (int x, int y, int width, int height);
static GemUta    * gem_uta_new_coords (int x, int y, int width, int height);
static void        gem_uta_free       (GemUta *uta);
static void        gem_uta_union      (GemUta *uta1, const GemUta *uta2);
static void        gem_uta_add_rect   (GemUta *uta, const GemRect * rect);

#define GEM_UTA_QUERY_CONTAIN		0
#define GEM_UTA_QUERY_INTERSECT		1

static int         gem_uta_query_rect (const GemUta *uta, const GemRect * rect, int query);

static Common::Rect * gem_uta_get_rects  (const GemUta *uta, int * num_rects,
	int min_x, int min_y, int max_x, int max_y);

/************** UTA IMPLEMENTATION **************/

#define GEM_UTILE_UNION(b1, b2) \
	{ \
		if (b1 == GEM_UTILE_FULL || b2 == GEM_UTILE_EMPTY) \
			{ } \
		else if (b1 == GEM_UTILE_EMPTY || b2 == GEM_UTILE_FULL) \
			{ b1 = b2; } \
		else { \
			b1 = GEM_UTA_BBOX_CONS ( \
				GEM_MIN(GEM_UTA_BBOX_X0(b1), \
					GEM_UTA_BBOX_X0(b2)), \
				GEM_MIN(GEM_UTA_BBOX_Y0(b1), \
					GEM_UTA_BBOX_Y0(b2)), \
				GEM_MAX(GEM_UTA_BBOX_X1(b1), \
					GEM_UTA_BBOX_X1(b2)), \
				GEM_MAX(GEM_UTA_BBOX_Y1(b1), \
					GEM_UTA_BBOX_Y1(b2))); \
		} \
	}
	
static GemUta * gem_uta_new (int x, int y, int width, int height) {
	GemUta *uta;
	
	uta = (GemUta *) malloc (sizeof (GemUta));
	uta->x = x;
	uta->y = y;
	uta->width = width;
	uta->height = height;

	uta->tiles = (GemUtaBBox *) 
		malloc (sizeof (GemUtaBBox) * width * height);

	memset (uta->tiles, 0, sizeof (GemUtaBBox) * width * height);
	
	return uta;
}

static GemUta * gem_uta_new_coords (int x, int y, int width, int height) {
	return gem_uta_new (x >> GEM_UTILE_SHIFT, y >> GEM_UTILE_SHIFT,
			    (width >> GEM_UTILE_SHIFT) + 1,
			    (height >> GEM_UTILE_SHIFT) + 1);
}

static void gem_uta_free (GemUta *uta) {
	if (uta) {
		if (uta->tiles)
			free (uta->tiles);
		free(uta);
	}
}

static void  gem_uta_union (GemUta *uta1, const GemUta *uta2) {
	int32 x0, y0, x1, y1;
	int x, y;
	GemUtaBBox *_b1;
	const GemUtaBBox *_b2;

	if (!uta1 || !uta2)
		return;

	x0 = GEM_MAX (uta1->x, uta2->x);
	y0 = GEM_MAX (uta2->y, uta2->y);
	x1 = GEM_MIN (uta1->x + uta1->width - 1, 
		      uta2->x + uta2->width - 1);
	y1 = GEM_MIN (uta1->y + uta1->height - 1,
		      uta2->y + uta2->height - 1);

	if (x0 > x1 || y0 > y1)
		return;

	_b1 = & (uta1->tiles [(y0 - uta1->y) * uta1->width + (x0 - uta1->x)]);
	_b2 = & (uta2->tiles [(y0 - uta2->y) * uta2->width + (x0 - uta2->x)]);	

	for (y = y0; y <= y1; ++y)
	{
		GemUtaBBox *b1 = _b1;
		const GemUtaBBox *b2 = _b2;

		for (x = x0; x <= x1; ++x)
		{
			GEM_UTILE_UNION (*b1, *b2);		

			b1 ++;
			b2 ++;
		}
		_b1 += uta1->width;
		_b2 += uta2->width;
	}
}

/*
 * Add a rectangle to the microtile array.
 * The coordinates of the rectangle are considered absolute.
 */

static void gem_uta_add_rect (GemUta *uta, const GemRect * _rect) {
	int xf0, xf1, yf0, yf1;
	int width, height;
	int x0, y0, x1, y1;
	int x, y;
	GemUtaBBox *_b;
	GemUtaBBox *b;
	GemUtaBBox bbox;
	GemRect rect;

	if (!uta || !_rect)
		return;
	
	rect = *_rect;
	
	rect.x0 -= uta->x << GEM_UTILE_SHIFT;
	rect.y0 -= uta->y << GEM_UTILE_SHIFT;
	rect.x1 -= uta->x << GEM_UTILE_SHIFT;
	rect.y1 -= uta->y << GEM_UTILE_SHIFT;
	
	if (rect.x0 < 0)
		rect.x0 = 0;
	if (rect.y0 < 0)
		rect.y0 = 0;
	if (rect.x1 > (uta->width << GEM_UTILE_SHIFT))
		rect.x1 = (uta->width << GEM_UTILE_SHIFT);
	if (rect.y1 > (uta->height << GEM_UTILE_SHIFT))
		rect.y1 = (uta->height << GEM_UTILE_SHIFT);
	
	if (rect.x0 > rect.x1 || rect.y0 > rect.y1)
		return;

	width = ((rect.x1 + GEM_UTILE_SIZE) >> GEM_UTILE_SHIFT) 
		    - (rect.x0 >> GEM_UTILE_SHIFT);
	height = ((rect.y1 + GEM_UTILE_SIZE) >> GEM_UTILE_SHIFT) 
		    - (rect.y0 >> GEM_UTILE_SHIFT);
		
	xf0 = rect.x0 % (GEM_UTILE_SIZE);
	yf0 = rect.y0 % (GEM_UTILE_SIZE);
	xf1 = rect.x1 % (GEM_UTILE_SIZE);
	yf1 = rect.y1 % (GEM_UTILE_SIZE);

	x0 = rect.x0 >> GEM_UTILE_SHIFT;
	y0 = rect.y0 >> GEM_UTILE_SHIFT;
	x1 = rect.x1 >> GEM_UTILE_SHIFT;
	y1 = rect.y1 >> GEM_UTILE_SHIFT;	

	_b = & (uta->tiles [(y0 * uta->width) + x0]);

	if (width == 1 && height == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, xf1 + 1, yf1 + 1);
		GEM_UTILE_UNION (*_b, bbox);
		return;
	}
	if (width == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, xf1 + 1, 
					  GEM_UTILE_SIZE);
		GEM_UTILE_UNION (*_b, bbox);
		_b += uta->width;
		if (height > 2) {
			for (y = y0 + 1; y <= (y1 - 1); ++y) {
				bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, xf1 + 1, GEM_UTILE_SIZE);
				GEM_UTILE_UNION (*_b, bbox);
				_b += uta->width;
			}
		}
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, xf1 + 1, yf1 + 1);
		GEM_UTILE_UNION (*_b, bbox);
		return;
	} else if (height == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, GEM_UTILE_SIZE, yf1 + 1);
		GEM_UTILE_UNION (*_b, bbox);
		++_b;
		if (width > 2) {
			for (x = x0 + 1; x <= (x1 - 1); ++x) {
				bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, GEM_UTILE_SIZE, xf1 + 1);
				GEM_UTILE_UNION (*_b, bbox);
				++_b;
			}
		}
		bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, xf1 + 1, yf1 + 1);
		GEM_UTILE_UNION (*_b, bbox);
		return;
	}
	
	b = _b;

	/* top-left corner */
	bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
	GEM_UTILE_UNION (*b, bbox);
	++b;

	/* top edge */
	if (width > 2) {
		for (x = x0 + 1; x <= (x1 - 1); ++x) {
			bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
			GEM_UTILE_UNION (*b, bbox);
			++b;
		}
	}
	
	/* top-right corner */
	bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, xf1 + 1, GEM_UTILE_SIZE);
	GEM_UTILE_UNION (*b, bbox);
	
	_b += uta->width;

	/* left edge, content, right edge */
	if (height > 2) {
		for (y = y0 + 1; y <= (y1 - 1); ++y) {
			b = _b;
			
			/* left edge */
			bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
			GEM_UTILE_UNION (*b, bbox);
			++b;

			/* content */
			if (width > 2) {
				for (x = x0 + 1; x <= (x1 - 1); ++x) {
					*b = GEM_UTILE_FULL;
					++b;
				}
			}
			
			/* right edge */
			bbox = GEM_UTA_BBOX_CONS (1, 1, xf1 + 1, GEM_UTILE_SIZE);
			GEM_UTILE_UNION (*b, bbox);
			
			_b += uta->width;
		}
	}

	b = _b;

	/* bottom-left corner */
	bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, GEM_UTILE_SIZE, yf1 + 1);
	GEM_UTILE_UNION (*b, bbox);
	++b;
	
	/* bottom edge */
	if (width > 2) {
		for (x = x0 + 1; x <= (x1 - 1); ++x) {
			bbox = GEM_UTA_BBOX_CONS (1, 1, GEM_UTILE_SIZE, yf1 + 1);
			GEM_UTILE_UNION (*b, bbox);
			++b;
		}
	}
	
	/* bottom-left corner */
	bbox = GEM_UTA_BBOX_CONS (1, 1, xf1 + 1, yf1 + 1);
	GEM_UTILE_UNION (*b, bbox);

	/* done. */	
}

#define GEM_UTILE_CONTAINS(b1, b2) \
	( !(GEM_UTA_BBOX_X0 (b1) > GEM_UTA_BBOX_X0 (b2) || \
	    GEM_UTA_BBOX_Y0 (b1) > GEM_UTA_BBOX_Y0 (b2) || \
	    GEM_UTA_BBOX_X1 (b1) < GEM_UTA_BBOX_X1 (b2) || \
	    GEM_UTA_BBOX_Y1 (b1) < GEM_UTA_BBOX_Y1 (b2)) ) 

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

static inline int gem_utile_intersects (GemUtaBBox b1, GemUtaBBox b2) {
	GemUtaBBox b3;
	b3 = GEM_UTA_BBOX_CONS (
		MAX (GEM_UTA_BBOX_X0 (b1), GEM_UTA_BBOX_X0 (b2)),
		MAX (GEM_UTA_BBOX_Y0 (b1), GEM_UTA_BBOX_Y0 (b2)),
		MIN (GEM_UTA_BBOX_X1 (b1), GEM_UTA_BBOX_X1 (b2)),
		MIN (GEM_UTA_BBOX_Y1 (b1), GEM_UTA_BBOX_Y1 (b2))
	);
	return (GEM_UTA_BBOX_X0 (b3) <= GEM_UTA_BBOX_X1 (b3) &&
		GEM_UTA_BBOX_Y0 (b3) <= GEM_UTA_BBOX_Y1 (b3));
}

#define GEM_UTILE_INTERSECTS(b1, b2) \
	gem_utile_intersects (b1, b2)
/*
 * Return if the uta contains the rectangle.
 */

static int gem_uta_query_rect (const GemUta *uta, const GemRect * _rect, int query) {
	int xf0, xf1, yf0, yf1;
	int width, height;
	int x0, y0, x1, y1;
	int x, y;
	GemUtaBBox *_b;
	GemUtaBBox *b;
	GemUtaBBox bbox;
	GemRect rect;

	if (!uta || !_rect)
		return 0;
	
	rect = *_rect;
	
	rect.x0 -= uta->x << GEM_UTILE_SHIFT;
	rect.y0 -= uta->y << GEM_UTILE_SHIFT;
	rect.x1 -= uta->x << GEM_UTILE_SHIFT;
	rect.y1 -= uta->y << GEM_UTILE_SHIFT;
	
	if (rect.x0 < 0)
		rect.x0 = 0;
	if (rect.y0 < 0)
		rect.y0 = 0;
	if (rect.x1 > (uta->width << GEM_UTILE_SHIFT))
		rect.x1 = (uta->width << GEM_UTILE_SHIFT);
	if (rect.y1 > (uta->height << GEM_UTILE_SHIFT))
		rect.y1 = (uta->height << GEM_UTILE_SHIFT);
	
	if (rect.x0 > rect.x1 || rect.y0 > rect.y1)
		return 0;

	width = ((rect.x1 + GEM_UTILE_SIZE) >> GEM_UTILE_SHIFT) 
		    - (rect.x0 >> GEM_UTILE_SHIFT);
	height = ((rect.y1 + GEM_UTILE_SIZE) >> GEM_UTILE_SHIFT) 
		    - (rect.y0 >> GEM_UTILE_SHIFT);
		
	xf0 = rect.x0 % (GEM_UTILE_SIZE);
	yf0 = rect.y0 % (GEM_UTILE_SIZE);
	xf1 = rect.x1 % (GEM_UTILE_SIZE);
	yf1 = rect.y1 % (GEM_UTILE_SIZE);

	x0 = rect.x0 >> GEM_UTILE_SHIFT;
	y0 = rect.y0 >> GEM_UTILE_SHIFT;
	x1 = rect.x1 >> GEM_UTILE_SHIFT;
	y1 = rect.y1 >> GEM_UTILE_SHIFT;	

	_b = & (uta->tiles [(y0 * uta->width) + x0]);

	if (width == 1 && height == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, xf1 + 1, yf1 + 1);
		if (query == GEM_UTA_QUERY_INTERSECT &&
		    GEM_UTILE_INTERSECTS (*_b, bbox))
			return 1;
		else if (query == GEM_UTA_QUERY_CONTAIN &&
			 GEM_UTILE_CONTAINS (*_b, bbox))
			return 1;
		return 0;
			
	}
#define CHECK(b1,b2) \
	{ \
		if (query == GEM_UTA_QUERY_INTERSECT && \
		    GEM_UTILE_INTERSECTS (b1, b2)) \
			return 1; \
		else if (query == GEM_UTA_QUERY_CONTAIN && \
			 !(GEM_UTILE_CONTAINS (b1, b2))) \
			return 0; \
	} 
	if (width == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, xf1 + 1, GEM_UTILE_SIZE);
		CHECK (*_b, bbox);
		_b += uta->width;
		if (height > 2) {
			for (y = y0 + 1; y <= (y1 - 1); ++y) {
				bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, xf1 + 1, GEM_UTILE_SIZE);
				CHECK (*_b, bbox);
				_b += uta->width;
			}
		}
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, xf1 + 1, yf1 + 1);
		CHECK (*_b, bbox);
		if (query == GEM_UTA_QUERY_INTERSECT)
			return 0;
		else if (query == GEM_UTA_QUERY_CONTAIN)
			return 1;
	} else if (height == 1) {
		bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, GEM_UTILE_SIZE, yf1 + 1);
		CHECK (*_b, bbox);
		++_b;
		if (width > 2) {
			for (x = x0 + 1; x <= (x1 - 1); ++x) {
				bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, GEM_UTILE_SIZE, xf1 + 1);
				CHECK (*_b, bbox);
				++_b;
			}
		}
		bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, xf1 + 1, yf1 + 1);
		CHECK (*_b, bbox);
		if (query == GEM_UTA_QUERY_INTERSECT)
			return 0;
		else if (query == GEM_UTA_QUERY_CONTAIN)
			return 1;
	}
	
	b = _b;

	/* top-left corner */
	bbox = GEM_UTA_BBOX_CONS (xf0 + 1, yf0 + 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
	CHECK (*b, bbox);
	++b;

	/* top edge */
	if (width > 2) {
		for (x = x0 + 1; x <= (x1 - 1); ++x) {
			bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
			CHECK (*b, bbox);
			++b;
		}
	}
	
	/* top-right corner */
	bbox = GEM_UTA_BBOX_CONS (1, yf0 + 1, xf1 + 1, GEM_UTILE_SIZE);
	CHECK (*b, bbox);
	
	_b += uta->width;

	/* left edge, content, right edge */
	if (height > 2) {
		for (y = y0 + 1; y <= (y1 - 1); ++y) {
			b = _b;
			
			/* left edge */
			bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, GEM_UTILE_SIZE, GEM_UTILE_SIZE);
			CHECK (*b, bbox);
			++b;

			/* content */
			if (width > 2) {
				for (x = x0 + 1; x <= (x1 - 1); ++x) {
					CHECK (*b, GEM_UTILE_FULL);
					++b;
				}
			}
			
			/* right edge */
			bbox = GEM_UTA_BBOX_CONS (1, 1, xf1 + 1, GEM_UTILE_SIZE);
			CHECK (*b, bbox);
			
			_b += uta->width;
		}
	}

	b = _b;

	/* bottom-left corner */
	bbox = GEM_UTA_BBOX_CONS (xf0 + 1, 1, GEM_UTILE_SIZE, yf1 + 1);
	CHECK (*b, bbox);
	++b;
	
	/* bottom edge */
	if (width > 2) {
		for (x = x0 + 1; x <= (x1 - 1); ++x) {
			bbox = GEM_UTA_BBOX_CONS (1, 1, GEM_UTILE_SIZE, yf1 + 1);
			CHECK (*b, bbox);
			++b;
		}
	}
	
	/* bottom-left corner */
	bbox = GEM_UTA_BBOX_CONS (1, 1, xf1 + 1, yf1 + 1);
	CHECK (*b, bbox);

#undef CHECK

	/* done. */	
	if (query == GEM_UTA_QUERY_CONTAIN)
		return 1;
	return 0;
}


#undef CLAMP
#define CLAMP(a,min,max) (a = (a < min ? min : (a > max ? max : a)))

static Common::Rect * gem_uta_get_rects  (const GemUta *uta, int * num_rects,
    int min_x, int min_y, int max_x, int max_y) {
    
#if 1


	GemRect *rects = new GemRect[uta->width * uta->height];
	Common::Rect *result = NULL;
	int n_rects = 0;
	int x, y;
	int x0, y0, x1, y1;

	int *glom = new int[uta->width * uta->height];
	int i; /* current */
	
	for (i = 0; i < uta->width * uta->height; ++i)
		glom [i] = -1;

	i = 0;	
	
	for (y = 0; y < uta->height; ++y) {
		for (x = 0; x < uta->width; ++x) {
#define TILE uta->tiles [i]
			int start;
			int finish = 0;
			GemUtaBBox b;

			b = uta->tiles [i];

			if (TILE == GEM_UTILE_EMPTY)
				goto next;						

			x0 = ((uta->x + x) << GEM_UTILE_SHIFT) + GEM_UTA_BBOX_X0 (TILE) - 1;
			y0 = ((uta->y + y) << GEM_UTILE_SHIFT) + GEM_UTA_BBOX_Y0 (TILE) - 1;
			y1 = ((uta->y + y) << GEM_UTILE_SHIFT) + GEM_UTA_BBOX_Y1 (TILE) - 1;
			
			CLAMP (x0, min_x, max_x);
			CLAMP (y0, min_y, max_y);
			CLAMP (y1, min_y, max_y);
			
			start = i;
			
			if (GEM_UTA_BBOX_X1 (TILE) != GEM_UTILE_SIZE || x == uta->width - 1) {
				/* the tile does not continue */
				goto done;
			}
			
			while (!finish) {
				++x;
				++i;
				
				if (x == uta->width ||
				    GEM_UTA_BBOX_Y0 (TILE) != GEM_UTA_BBOX_Y0 (b) ||
				    GEM_UTA_BBOX_Y1 (TILE) != GEM_UTA_BBOX_Y1 (b) ||
				    GEM_UTA_BBOX_X0 (TILE) != 1)
				{
					--x;
					--i;
					finish = 1;
				}
			}

		done:
			x1 = ((uta->x + x) << GEM_UTILE_SHIFT) + GEM_UTA_BBOX_X1 (TILE) - 1;
			
			CLAMP (x1, min_x, max_x);

			if (glom [start] != -1 && /* try to glom */ 
			    rects [glom [start]].x0 == x0 &&
			    rects [glom [start]].x1 == x1 &&
			    rects [glom [start]].y1 == y0 - 1)
			{
				rects [glom [start]].y1 = y1;
				if (y != uta->height - 1) {
					glom [start + uta->width] = glom [start];
				}
			} else {
				rects [n_rects].x0 = x0;
				rects [n_rects].y0 = y0;
				rects [n_rects].x1 = x1;
				rects [n_rects].y1 = y1;
				if (y != uta->height - 1) {
					glom [start + uta->width] = n_rects;
				}
				n_rects ++;
			}
		next:
			++i;
#undef TILE
		}
	}
	
	result = new Common::Rect[n_rects];
	
	for (i = 0; i < n_rects; ++i) {
		result[i].left = rects [i].x0;
		result[i].top = rects [i].y0;
		result[i].right = rects[i].x1; // CHECK
		result[i].bottom = rects[i].y1; // CHECK
	}

	*num_rects = n_rects;
	
	delete rects;
	delete glom;
	
	return result;

#else
	return NULL;
#endif
	
}

MicroTileArray::MicroTileArray()
    : m_uta (NULL)
{
}

MicroTileArray::MicroTileArray(int x, int y, int w, int h)
	: m_x (x), m_y (y), m_w (w), m_h (h)
{
	m_uta = gem_uta_new_coords (x, y, w, h);
	if (!m_uta)
		error("MicroTileArray::MicroTileArray() Out of memory");
} 

MicroTileArray::~MicroTileArray() {
	if (m_uta)
		gem_uta_free (m_uta);
}

void MicroTileArray::clear() {
	if (!m_uta)
		return;
		
	memset (m_uta->tiles, 0, m_uta->width * m_uta->height * sizeof(GemUtaBBox));
}

void MicroTileArray::init(int x, int y, int w, int h) {
	if (m_uta)
		gem_uta_free (m_uta);
	m_uta = gem_uta_new_coords (x, y, w, h);
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;
	if (!m_uta)
		error("Out of memory");
} 

int MicroTileArray::getRectangles(Common::Rect * & rects, int min_x, int max_x, int min_y, int max_y) const {
	if (!m_uta)
		return 0;

	int n_rects;
	
	rects = gem_uta_get_rects (m_uta, &n_rects, min_x, max_x, min_y, max_y);

	return n_rects;
}

int MicroTileArray::getRectangles(Common::Rect * & rects) const {
	if (!m_uta)
		return 0;

	int n_rects;
	
	rects = gem_uta_get_rects (m_uta, &n_rects, 
				   m_x, m_y, m_x + m_w - 1, m_y + m_h - 1);

	return n_rects;
}

bool  MicroTileArray::contains(const Common::Rect & rect) const {
	if (!m_uta)
		return false;

	GemRect r;
	r.x0 = rect.left;
	r.y0 = rect.top;
	r.x1 = rect.right; // CHECK
	r.y1 = rect.bottom; // CHECK

	return gem_uta_query_rect (m_uta, &r, GEM_UTA_QUERY_CONTAIN);
}
	
bool  MicroTileArray::intersects(const Common::Rect & rect) const {
	if (!m_uta)
		return false;

	GemRect r;
	r.x0 = rect.left;
	r.y0 = rect.top;
	r.x1 = rect.right; // CHECK
	r.y1 = rect.bottom; // CHECK

	return gem_uta_query_rect (m_uta, &r, GEM_UTA_QUERY_INTERSECT);
}

void MicroTileArray::unite(const Common::Rect & rect) {
	if (!m_uta || rect.width() == 0 || rect.height() == 0)
		return;

	GemRect r;
	r.x0 = rect.left;
	r.y0 = rect.top;
	r.x1 = rect.right; // CHECK
	r.y1 = rect.bottom; // CHECK

	gem_uta_add_rect (m_uta, &r);
}

MicroTileArray & MicroTileArray::operator += (const Common::Rect & rect) {
	if (!m_uta || rect.width() == 0 || rect.height() == 0)
		return *this;

	GemRect r;
	r.x0 = rect.left;
	r.y0 = rect.top;
	r.x1 = rect.right; // CHECK
	r.y1 = rect.bottom; // CHECK

	gem_uta_add_rect (m_uta, &r);

	return *this;
}

void MicroTileArray::unite (const MicroTileArray & uta) {
	if (m_uta)
		gem_uta_union (m_uta, uta.m_uta);
}

MicroTileArray & MicroTileArray::operator += (const MicroTileArray & uta) {
	if (m_uta)
		gem_uta_union (m_uta, uta.m_uta);
	return *this;
}

} // namespace Picture

