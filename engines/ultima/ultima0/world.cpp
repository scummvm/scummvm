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

/************************************************************************/
/************************************************************************/
/*																		*/
/*							  World Map Code							*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*			Create the world map, set player start position				*/
/*																		*/
/************************************************************************/

void WORLDCreate(PLAYER *p, WORLDMAP *w) {
	int c, x, y, Size;

	g_engine->setRandomSeed(p->LuckyNumber);		/* Seed the random number */
	Size = MAINSuper() ? WORLD_MAP_SIZE - 1 : 20;	/* Calculate map size */
	w->MapSize = Size;						/* Save the map size */

	// Draw the boundaries
	for (x = 0; x <= Size; x++) {
		w->Map[Size][x] = WT_MOUNTAIN;
		w->Map[0][x] = WT_MOUNTAIN;
		w->Map[x][Size] = WT_MOUNTAIN;
		w->Map[x][0] = WT_MOUNTAIN;
	}

	/* Draw the rest of it */
	for (x = 1; x < Size; x++)
		for (y = 1; y < Size; y++) {
			c = (int)(pow(RND(), 5.0) * 4.5);	/* Calculate what's there */
			if (c == WT_TOWN && RND() > .5)	/* Remove half the towns */
				c = WT_SPACE;
			w->Map[x][y] = c;				/* Save in map */
		}
	x = urand() % (Size - 1) + 1;				/* Calculate player start */
	y = urand() % (Size - 1) + 1;
	p->World.x = x; p->World.y = y;			/* Save it */
	w->Map[x][y] = WT_TOWN;					/* Make it a town */

	/* Find place for castle */
	do {
		x = urand() % Size + 1;
		y = urand() % Size + 1;
	} while (w->Map[x][y] != WT_SPACE);

	w->Map[x][y] = WT_BRITISH;				/* Put LBs castle there */
}

/************************************************************************/
/*																		*/
/*							Read world map value						*/
/*																		*/
/************************************************************************/

int WORLDRead(WORLDMAP *w, int x, int y) {
	if (x < 0 || y < 0) return WT_MOUNTAIN;
	if (x > w->MapSize) return WT_MOUNTAIN;
	if (y > w->MapSize) return WT_MOUNTAIN;
	return w->Map[x][y];
}

/************************************************************************/
/*																		*/
/*							Draw world map								*/
/*																		*/
/************************************************************************/

void WORLDDraw(PLAYER *p, WORLDMAP *m, int ShowAsMap) {
	int x, y, x1, y1, w, h, Grid;
	RECT r;
	Grid = 7;							/* Number of cells in grid */
	if (MAINSuper() == 0) Grid = 3;		/* Standard Aklabeth */
	if (ShowAsMap) Grid = m->MapSize + 1;	/* Displaying as a map ? */
	w = 1280 / Grid; h = 1024 / Grid;	/* Get grid sizes */
	for (x = 0; x < Grid; x++)			/* For all grid cells */
		for (y = 0; y < Grid; y++)
		{
			DRAWSetRect(&r, x * w, y * h,		/* Work out the drawing rect */
				x * w + w - 1, y * h + h - 1);
			x1 = p->World.x - Grid / 2 + x;	/* Which cell ? */
			y1 = p->World.y + Grid / 2 - y;
			if (ShowAsMap)				/* If map, not centred around us */
				x1 = x, y1 = Grid - 1 - y;
			DRAWTile(&r, WORLDRead(m, x1, y1));
			if (x1 == p->World.x &&		/* Draw us if we're there */
				y1 == p->World.y)
				DRAWTile(&r, WT_PLAYER);
		}
}

} // namespace Ultima0
} // namespace Ultima
