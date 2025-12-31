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

#include "ultima/ultima0/gfx/dungeon.h"
#include "ultima/ultima0/gfx/monster.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {
namespace Gfx {

int Dungeon::xLeft;
int Dungeon::xRight;
int Dungeon::yBottom;
int Dungeon::yDiffLeft;
int Dungeon::yDiffRight;

#define HWColour(IDX)   color = IDX
#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define HWLine(X1, Y1, X2, Y2) s->drawLine(X1, Y1, X2, Y2, color)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void Dungeon::draw(Graphics::ManagedSurface *s) {
	const auto &player = g_engine->_player;
	const auto &dungeon = g_engine->_dungeon;

	s->clear();

	double Level = 0;
	Common::Rect rIn, rOut;
	COORD Dir, Pos, Next;
	int Monster, Front, Left, Right;
	_DDRAWCalcRect(s, &rOut, 0);
	Pos = player.Dungeon;						// Get position

	// Iterate through drawing successively distinct tiles in the facing direction
	do {
		Level++;								// Next level
		_DDRAWCalcRect(s, &rIn, Level);
		Next.x = Pos.x + player.DungDir.x;		// Next position
		Next.y = Pos.y + player.DungDir.y;

		Dir = player.DungDir; MOVERotLeft(&Dir);	// To the left
		Left = dungeon.Map[Pos.x + Dir.x][Pos.y + Dir.y];
		MOVERotLeft(&Dir); MOVERotLeft(&Dir);		// To the right
		Right = dungeon.Map[Pos.x + Dir.x][Pos.y + Dir.y];
		Front = dungeon.Map[Next.x][Next.y];		// What's in front ?

		// Check for monster present
		Monster = dungeon.findMonster(Pos);
		if (Monster >= 0)					
			Monster = dungeon.Monster[Monster].Type;

		// Draw the dungeon
		DRAWDungeon(s, &rOut, &rIn, Left, Front, Right,
			dungeon.Map[Pos.x][Pos.y], Monster);

		Pos = Next;							// Next position down
		rOut = rIn;							// Last in is new out
	} while (Level < MAX_VIEW_DEPTH && ISDRAWOPEN(Front));
}

void Dungeon::_DDRAWCalcRect(Graphics::ManagedSurface *s, Common::Rect *r, double Level) {
	const int centerX = s->w / 2, centerY = s->h / 2;
	int xWidth = s->w / (Level + 1);
	int yWidth = s->h / (Level + 1);

	// Set bounding box
	r->left = centerX - xWidth / 2;
	r->right = centerX + xWidth / 2;
	r->top = centerY - yWidth / 2;
	r->bottom = centerY + yWidth / 2;
}

void Dungeon::MOVERotLeft(COORD *Dir) {
	int t;
	if (Dir->y == 0) Dir->x = -Dir->x;
	t = Dir->x;
	Dir->x = Dir->y;
	Dir->y = t;
}

void Dungeon::DRAWDungeon(Graphics::ManagedSurface *s, Common::Rect *rOut,
		Common::Rect *rIn, int Left, int Centre, int Right, int Room, int Monster) {
	int x1, y1, x, y, y2;
	Common::Rect r;
	double Scale;
	int color;

	HWColour(COL_WALL);						// Start on the walls

	// Do we draw the left edge
	if (ISDRAWOPEN(Left)) {
		HWLine(rOut->left, rIn->top, rIn->left, rIn->top);
		HWLine(rOut->left, rIn->bottom, rIn->left, rIn->bottom);
		HWLine(rOut->left, rOut->top, rOut->left, rOut->bottom);
	} else {
		// If closed, draw left diags
		HWLine(rOut->left, rOut->top, rIn->left, rIn->top);
		HWLine(rOut->left, rOut->bottom, rIn->left, rIn->bottom);
	}

	// Do we draw the right edge
	if (ISDRAWOPEN(Right)) {
		HWLine(rOut->right, rIn->top, rIn->right, rIn->top);
		HWLine(rOut->right, rIn->bottom, rIn->right, rIn->bottom);
		HWLine(rOut->right, rOut->top, rOut->right, rOut->bottom);
	} else {
		// If closed draw right diags
		HWLine(rOut->right, rOut->top, rIn->right, rIn->top);
		HWLine(rOut->right, rOut->bottom, rIn->right, rIn->bottom);
	}

	// Back wall ?
	if (!ISDRAWOPEN(Centre)) {
		HWLine(rIn->left, rIn->top, rIn->right, rIn->top);
		HWLine(rIn->left, rIn->bottom, rIn->right, rIn->bottom);
		if (!ISDRAWOPEN(Left))				// Corner if left,right closed
			HWLine(rIn->left, rIn->top, rIn->left, rIn->bottom);
		if (!ISDRAWOPEN(Right))
			HWLine(rIn->right, rIn->top, rIn->right, rIn->bottom);
	}

	// Set up for left side
	_DRAWSetRange(rOut->left, rIn->left,
		rOut->bottom,
		rOut->bottom - rOut->top,
		rIn->bottom - rIn->top);
	_DRAWWall(s, Left);

	// Set up for right side
	_DRAWSetRange(rIn->right, rOut->right,
		rIn->bottom,
		rIn->bottom - rIn->top,
		rOut->bottom - rOut->top);
	_DRAWWall(s, Right);

	// Set up for centre
	_DRAWSetRange(rIn->left, rIn->right,
		rIn->bottom,
		rIn->bottom - rIn->top,
		rIn->bottom - rIn->top);
	_DRAWWall(s, Centre);

	if (Room == DT_LADDERUP) {
		r = Common::Rect(rOut->left, rOut->top, rOut->right, rIn->top);
		_DRAWPit(s, &r, 1);
	}
	if (Room == DT_LADDERDN || Room == DT_PIT) {
		r = Common::Rect(rOut->left, rIn->bottom, rOut->right, rOut->bottom);
		_DRAWPit(s, &r, -1);
	}

	// Get the object area
	r = Common::Rect(
		(rIn->left + rOut->left) / 2,
		(rIn->top + rOut->top) / 2,
		(rIn->right + rOut->right) / 2,
		(rIn->bottom + rOut->bottom) / 2);

	// Ladder here ?
	if (Room == DT_LADDERUP || Room == DT_LADDERDN) {
		HWColour(COL_LADDER);
		y1 = r.top; y2 = r.bottom;
		x = (r.right - r.left) * 3 / 10;

		// Vertical lines
		HWLine(r.left + x, y1, r.left + x, y2);
		HWLine(r.right - x, y1, r.right - x, y2);
		x1 = (y2 - y1) / 5;

		// Horizontal ladder rungs
		for (y = y1 + x1 / 2; y < y2; y += x1)
			HWLine(r.left + x, y, r.right - x, y);
	}

	// Scale (trial and error this :))
	Scale = 0.1;
	Scale = Scale / (r.right - r.left) * 1059.0;

	// Monster here ?
	if (Monster > 0) {
		HWColour(COL_MONSTER);
		Monster::draw(s, (r.left + r.right) / 2, r.bottom, Monster, Scale);
	}

	// Draw the gold (as a mimic)
	if (Room == DT_GOLD) {
		HWColour(COL_MONSTER);
		Monster::draw(s, (r.left + r.right) / 2, r.bottom, MN_MIMIC, Scale);
	}
}

void Dungeon::_DRAWSetRange(int x1, int x2, int y, int yd1, int yd2) {
	xLeft = x1; xRight = x2;				// Set x ranges
	yBottom = y;							// Set lower left y value
	yDiffLeft = yd1; yDiffRight = yd2;		// Set difference for either end
}

void Dungeon::_DRAWWall(Graphics::ManagedSurface *s, int n) {
	int x1, y1, x2, y2;
	int color = 0;

	if (n == DT_DOOR) {
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

void Dungeon::_DRAWConvert(int *px, int *py) {
	long x, y, yd;							// Longs for overflow in 16 bit
	x = (xRight - xLeft);					// Calculate width
	x = x * (*px) / 100 + xLeft;			// Work out horiz value
	yd = (yDiffRight - yDiffLeft);			// Work out height of vert for x
	yd = yd * (*px) / 100;
	y = yBottom +							// Half of the distance
		yd / 2 -							// + Scaled total size
		(yd + yDiffLeft) * (*py) / 100;

	*px = (int)x;							// Write back, casting to int
	*py = (int)y;
}

void Dungeon::_DRAWPit(Graphics::ManagedSurface *s, Common::Rect *r, int Dir) {
	int x1, x2, y1;
	int color;

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

} // namespace Views
} // namespace Ultima0
} // namespace Ultima
