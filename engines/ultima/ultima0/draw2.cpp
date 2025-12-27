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
/*							Draw Dungeon View Part						*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

static void _DRAWRect(int, int, int, int);
static void _DRAWSetRange(int, int, int, int, int);
static void _DRAWConvert(int *, int *);
static void _DRAWWall(int);
static void _DRAWPit(RECT *, int);

int xLeft, xRight, yBottom,					/* Slanted drawing constants */
yDiffLeft, yDiffRight;

/************************************************************************/
/*																		*/
/*						Draw part of dungeon							*/
/*																		*/
/************************************************************************/

void DRAWDungeon(RECT *rOut, RECT *rIn,
	int Left, int Centre, int Right,
	int Room, int Monster) {
	int x1, y1, x, y, y2;
	RECT r;
	double Scale;

	HWColour(COL_WALL);						/* Start on the walls */

	if (ISDRAWOPEN(Left))					/* Do we draw the left edge */
	{
		HWLine(rOut->left, rIn->top, rIn->left, rIn->top);
		HWLine(rOut->left, rIn->bottom, rIn->left, rIn->bottom);
		HWLine(rOut->left, rOut->top, rOut->left, rOut->bottom);
	} else									/* If closed, draw left diags */
	{
		HWLine(rOut->left, rOut->top, rIn->left, rIn->top);
		HWLine(rOut->left, rOut->bottom, rIn->left, rIn->bottom);
	}

	if (ISDRAWOPEN(Right))					/* Do we draw the right edge */
	{
		HWLine(rOut->right, rIn->top, rIn->right, rIn->top);
		HWLine(rOut->right, rIn->bottom, rIn->right, rIn->bottom);
		HWLine(rOut->right, rOut->top, rOut->right, rOut->bottom);
	} else									/* If closed draw right diags */
	{
		HWLine(rOut->right, rOut->top, rIn->right, rIn->top);
		HWLine(rOut->right, rOut->bottom, rIn->right, rIn->bottom);
	}

	if (!ISDRAWOPEN(Centre))				/* Back wall ? */
	{
		HWLine(rIn->left, rIn->top, rIn->right, rIn->top);
		HWLine(rIn->left, rIn->bottom, rIn->right, rIn->bottom);
		if (!ISDRAWOPEN(Left))				/* Corner if left,right closed */
			HWLine(rIn->left, rIn->top, rIn->left, rIn->bottom);
		if (!ISDRAWOPEN(Right))
			HWLine(rIn->right, rIn->top, rIn->right, rIn->bottom);
	}

	_DRAWSetRange(rOut->left, rIn->left,		/* Set up for left side */
		rOut->bottom,
		rOut->bottom - rOut->top,
		rIn->bottom - rIn->top);
	_DRAWWall(Left);
	_DRAWSetRange(rIn->right, rOut->right,	/* Set up for right side */
		rIn->bottom,
		rIn->bottom - rIn->top,
		rOut->bottom - rOut->top);
	_DRAWWall(Right);						/* Set up for centre */
	_DRAWSetRange(rIn->left, rIn->right,
		rIn->bottom,
		rIn->bottom - rIn->top,
		rIn->bottom - rIn->top);
	_DRAWWall(Centre);

	if (Room == DT_LADDERUP)
	{
		DRAWSetRect(&r, rOut->left, rOut->top, rOut->right, rIn->top);
		_DRAWPit(&r, 1);
	}
	if (Room == DT_LADDERDN || Room == DT_PIT)
	{
		DRAWSetRect(&r, rOut->left, rIn->bottom, rOut->right, rOut->bottom);
		_DRAWPit(&r, -1);
	}

	DRAWSetRect(&r,                       	/* Get the object area */
		(rIn->left + rOut->left) / 2,
		(rIn->top + rOut->top) / 2,
		(rIn->right + rOut->right) / 2,
		(rIn->bottom + rOut->bottom) / 2);

	if (Room == DT_LADDERUP ||				/* Ladder here ? */
		Room == DT_LADDERDN)
	{
		HWColour(COL_LADDER);
		y1 = r.top; y2 = r.bottom;
		x = (r.right - r.left) * 3 / 10;
		HWLine(r.left + x, y1, r.left + x, y2);
		HWLine(r.right - x, y1, r.right - x, y2);
		x1 = (y1 - y2) / 5;
		y = y2 + x1 / 2;
		while (y < y1)
		{
			HWLine(r.left + x, y, r.right - x, y);
			y = y + x1;
		}
	}

	Scale = 0.1;						/* Scale (trial and error this :)) */
	Scale = Scale / (r.right - r.left) * 1059.0;

	if (Monster > 0)					/* Monster here ? */
	{
		HWColour(COL_MONSTER);
		DRAWMonster((r.left + r.right) / 2, r.bottom, Monster, Scale);
	}

	if (Room == DT_GOLD)				/* Draw the gold (as a mimic) */
	{
		HWColour(COL_MONSTER);
		DRAWMonster((r.left + r.right) / 2, r.bottom, MN_MIMIC, Scale);
	}

}

/************************************************************************/
/*																		*/
/*					Set the oblique drawing routine						*/
/*																		*/
/************************************************************************/

static void _DRAWSetRange(int x1, int x2, int y, int yd1, int yd2) {
	xLeft = x1; xRight = x2;					/* Set x ranges */
	yBottom = y;							/* Set lower left y value */
	yDiffLeft = yd1; yDiffRight = yd2;		/* Set difference for either end */
}

/************************************************************************/
/*																		*/
/*			Convert coordinates from oblique to logical					*/
/*																		*/
/************************************************************************/

static void _DRAWConvert(int *px, int *py) {
	long x, y, yd;							/* Longs for overflow in 16 bit */
	x = (xRight - xLeft);						/* Calculate width */
	x = x * (*px) / 100 + xLeft;			/* Work out horiz value */
	yd = (yDiffRight - yDiffLeft);			/* Work out height of vert for x */
	yd = yd * (*px) / 100;
	y = yBottom +							/* Half of the distance */
		yd / 2 -						/* + Scaled total size */
		(yd + yDiffLeft) * (*py) / 100;

	*px = (int)x;							/* Write back, casting to int */
	*py = (int)y;
}

/************************************************************************/
/*																		*/
/*							Draw a rectangle							*/
/*																		*/
/************************************************************************/

static void _DRAWRect(int x1, int y1, int x2, int y2) {
	HWLine(x1, y1, x2, y1); HWLine(x1, y1, x1, y2);
	HWLine(x2, y2, x2, y1); HWLine(x2, y2, x1, y2);
}

/************************************************************************/
/*																		*/
/*					Draw the pits/ladder hole							*/
/*																		*/
/************************************************************************/

static void _DRAWPit(RECT *r, int Dir) {
	int x1, x2, y1;
	HWColour(COL_HOLE);
	y1 = (r->top - r->bottom) / 5;
	r->bottom += y1; r->top -= y1;
	x1 = (r->right - r->left) / 5;
	r->left += x1; r->right -= x1;
	x2 = 0; x1 = x1 / 2;
	if (Dir > 0)
	{
		y1 = x1; x1 = x2; x2 = y1;
	}
	HWLine(r->left + x1, r->top, r->right - x1, r->top);
	HWLine(r->left + x1, r->top, r->left + x2, r->bottom);
	HWLine(r->left + x2, r->bottom, r->right - x2, r->bottom);
	HWLine(r->right - x1, r->top, r->right - x2, r->bottom);

}

/************************************************************************/
/*																		*/
/*				Draw wall object using current setting					*/
/*																		*/
/************************************************************************/

static void _DRAWWall(int n) {
	int x1, y1, x2, y2;
	if (n == DT_DOOR)
	{
		HWColour(COL_DOOR);
		x1 = 35; y1 = 0; x2 = 35; y2 = 60;
		_DRAWConvert(&x1, &y1);
		_DRAWConvert(&x2, &y2);
		HWLine(x1, y1, x2, y2);
		x1 = 65; y1 = 60; _DRAWConvert(&x1, &y1);
		HWLine(x1, y1, x2, y2);
		x2 = 65; y2 = 0; _DRAWConvert(&x2, &y2);
		HWLine(x1, y1, x2, y2);
	}
}

} // namespace Ultima0
} // namespace Ultima
