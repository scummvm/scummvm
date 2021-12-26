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

#include "ags/lib/allegro/flood.h"
#include "ags/lib/allegro/gfx.h"
#include "ags/shared/core/types.h"
#include "common/array.h"

namespace AGS3 {

struct FLOODED_LINE {
	short flags;                  /* status of the segment */
	short lpos, rpos;             /* left and right ends of segment */
	short y;                      /* y coordinate of the segment */
	int next;                     /* linked list if several per line */
};

#define FLOOD_IN_USE             1
#define FLOOD_TODO_ABOVE         2
#define FLOOD_TODO_BELOW         4

#define FLOOD_LINE(c)            (&scratchMem[c])

template<class SIZE>
static bool scanLine(BITMAP *bmp, int x, int y, int src_color, int &left, int &right) {
	// check start pixel
	const SIZE *pixel = (const SIZE *)bmp->getBasePtr(x, y);
	if (*pixel != src_color)
		return false;

	// Work left from starting point
	const SIZE *pixelLeft = pixel - 1;
	for (left = x - 1; left >= bmp->cl; left--, pixelLeft--) {
		if (*pixelLeft != src_color)
			break;
	}

	// Work right from starting point
	const SIZE *pixelRight = pixel + 1;
	for (right = x + 1; right < bmp->cr; right++, pixelRight++) {
		if (*pixelRight != src_color)
			break;
	}

	return true;
}

/**
 *  Fills a horizontal line around the specified position, and adds it
 *  to the list of drawn segments. Returns the first x coordinate after
 *  the part of the line which it has dealt with.
 */
static int flooder(BITMAP *bmp, int x, int y, int src_color, int dest_color,
                   Common::Array<FLOODED_LINE> &scratchMem) {
	FLOODED_LINE *p;
	int left = 0, right = 0;
	int c;

	assert(bmp);

	bool ret;
	switch (bmp->format.bytesPerPixel) {
	case 1:
		ret = scanLine<byte>(bmp, x, y, src_color, left, right);
		break;
	case 2:
		ret = scanLine<byte>(bmp, x, y, src_color, left, right);
		break;
	case 4:
		ret = scanLine<byte>(bmp, x, y, src_color, left, right);
		break;
	default:
		error("Unknown format");
	}
	if (!ret)
		return x + 1;

	left++;
	right--;

	/* draw the line */
	bmp->hLine(left, y, right, dest_color);

	/* store it in the list of flooded segments */
	c = y;
	p = FLOOD_LINE(c);

	if (p->flags) {
		while (p->next) {
			c = p->next;
			p = FLOOD_LINE(c);
		}

		p->next = c = scratchMem.size();
		scratchMem.resize(scratchMem.size() + 1);
		p = FLOOD_LINE(c);
	}

	p->flags = FLOOD_IN_USE;
	p->lpos = left;
	p->rpos = right;
	p->y = y;
	p->next = 0;

	if (y > bmp->ct)
		p->flags |= FLOOD_TODO_ABOVE;

	if (y + 1 < bmp->cb)
		p->flags |= FLOOD_TODO_BELOW;

	return right + 2;
}



/**
 *  Checks a line segment, using the scratch buffer is to store a list of
 *  segments which have already been drawn in order to minimise the required
 *  number of tests.
 */
static bool check_flood_line(BITMAP *bmp, int y, int left, int right, int src_color, int dest_color,
                             Common::Array<FLOODED_LINE> &scratchMem) {
	int c;
	FLOODED_LINE *p;
	bool ret = false;

	while (left <= right) {
		c = y;

		for (;;) {
			p = FLOOD_LINE(c);

			if ((left >= p->lpos) && (left <= p->rpos)) {
				left = p->rpos + 2;
				break;
			}

			c = p->next;

			if (!c) {
				left = flooder(bmp, left, y, src_color, dest_color, scratchMem);
				ret = true;
				break;
			}
		}
	}

	return ret;
}

void floodfill(BITMAP *bmp, int x, int y, int color) {
	int src_color;
	int c;
	bool done;
	FLOODED_LINE *p;
	Common::Array<FLOODED_LINE> scratchMem;

	/* make sure we have a valid starting point */
	if ((x < bmp->cl) || (x >= bmp->cr) || (y < bmp->ct) || (y >= bmp->cb))
		return;

	/* what color to replace? */
	src_color = getpixel(bmp, x, y);
	if (src_color == color) {
		return;
	}

	/* set up the list of flooded segments */
	scratchMem.resize(bmp->cb);

	p = FLOOD_LINE(0);
	for (c = 0; c < (int)scratchMem.size(); c++) {
		p[c].flags = 0;
		p[c].lpos = INT16_MAX;
		p[c].rpos = INT16_MIN;
		p[c].y = y;
		p[c].next = 0;
	}

	/* start up the flood algorithm */
	flooder(bmp, x, y, src_color, color, scratchMem);

	/* continue as long as there are some segments still to test */
	do {
		done = true;

		/* for each line on the screen */
		for (c = 0; c < (int)scratchMem.size(); c++) {
			p = FLOOD_LINE(c);

			/* check below the segment? */
			if (p->flags & FLOOD_TODO_BELOW) {
				p->flags &= ~FLOOD_TODO_BELOW;
				if (check_flood_line(bmp, p->y + 1, p->lpos, p->rpos, src_color, color, scratchMem)) {
					done = false;
					p = FLOOD_LINE(c);
				}
			}

			/* check above the segment? */
			if (p->flags & FLOOD_TODO_ABOVE) {
				p->flags &= ~FLOOD_TODO_ABOVE;
				if (check_flood_line(bmp, p->y - 1, p->lpos, p->rpos, src_color, color, scratchMem)) {
					done = false;
					/* special case shortcut for going backwards */
					if ((c < bmp->cb) && (c > 0))
						c -= 2;
				}
			}
		}

	} while (!done);
}

} // namespace AGS3
