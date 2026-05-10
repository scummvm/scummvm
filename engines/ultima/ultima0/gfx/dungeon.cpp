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

int Dungeon::_xLeft;
int Dungeon::_xRight;
int Dungeon::_yBottom;
int Dungeon::_yDiffLeft;
int Dungeon::_yDiffRight;

#define HWColour(IDX)   color = IDX
#define	X(n)			(x1 + w * (n)/10)
#define	Y(n)			(y1 + h * (n)/10)
#define HWLine(X1, Y1, X2, Y2) s->drawLine(X1, Y1, X2, Y2, color)
#define BOX(x1,y1,x2,y2) { HWLine(X(x1),Y(y1),X(x2),Y(y1));HWLine(X(x1),Y(y1),X(x1),Y(y2));HWLine(X(x2),Y(y2),X(x2),Y(y1));HWLine(X(x2),Y(y2),X(x1),Y(y2)); }

void Dungeon::draw(Graphics::ManagedSurface *s) {
	const auto &player = g_engine->_player;
	const auto &dungeon = g_engine->_dungeon;

	s->clear();

	double level = 0;
	Common::Rect rIn, rOut;
	Common::Point dir, pos, next;
	int monster, front, left, right;
	calcRect(s, &rOut, 0);
	pos = player._dungeonPos;						// Get position

	// Iterate through drawing successively distinct tiles in the facing direction
	do {
		level++;									// Next level
		calcRect(s, &rIn, level);

		next.x = pos.x + player._dungeonDir.x;		// Next position
		next.y = pos.y + player._dungeonDir.y;

		dir = player._dungeonDir; rotateLeft(&dir);	// To the left
		left = dungeon._map[pos.x + dir.x][pos.y + dir.y];
		rotateLeft(&dir); rotateLeft(&dir);		// To the right
		right = dungeon._map[pos.x + dir.x][pos.y + dir.y];
		front = dungeon._map[next.x][next.y];		// What's in front ?

		// Check for monster present
		monster = dungeon.findMonster(pos);
		if (monster >= 0)					
			monster = dungeon._monsters[monster]._type;

		// Draw the dungeon
		drawDungeon(s, &rOut, &rIn, left, front, right,
			dungeon._map[pos.x][pos.y], monster);

		pos = next;							// Next position down
		rOut = rIn;							// Last in is new out
	} while (level < MAX_VIEW_DEPTH && ISDRAWOPEN(front));
}

void Dungeon::calcRect(Graphics::ManagedSurface *s, Common::Rect *r, double level) {
	const int centerX = s->w / 2, centerY = s->h / 2;
	int xWidth = s->w / (level + 1);
	int yWidth = s->h / (level + 1);

	// Set bounding box
	r->left = centerX - xWidth / 2;
	r->right = centerX + xWidth / 2;
	r->top = centerY - yWidth / 2;
	r->bottom = centerY + yWidth / 2;
}

void Dungeon::rotateLeft(Common::Point *dir) {
	int t;
	if (dir->y == 0) dir->x = -dir->x;
	t = dir->x;
	dir->x = dir->y;
	dir->y = t;
}

void Dungeon::drawDungeon(Graphics::ManagedSurface *s, Common::Rect *rOut,
		Common::Rect *rIn, int left, int centre, int right, int room, int monster) {
	int x1, y1, x, y, y2;
	Common::Rect r;
	double Scale;
	int color;

	HWColour(COL_WALL);						// Start on the walls

	// Do we draw the left edge
	if (ISDRAWOPEN(left)) {
		HWLine(rOut->left, rIn->top, rIn->left, rIn->top);
		HWLine(rOut->left, rIn->bottom, rIn->left, rIn->bottom);
		HWLine(rOut->left, rOut->top, rOut->left, rOut->bottom);
	} else {
		// If closed, draw left diags
		HWLine(rOut->left, rOut->top, rIn->left, rIn->top);
		HWLine(rOut->left, rOut->bottom, rIn->left, rIn->bottom);
	}

	// Do we draw the right edge
	if (ISDRAWOPEN(right)) {
		HWLine(rOut->right, rIn->top, rIn->right, rIn->top);
		HWLine(rOut->right, rIn->bottom, rIn->right, rIn->bottom);
		HWLine(rOut->right, rOut->top, rOut->right, rOut->bottom);
	} else {
		// If closed draw right diags
		HWLine(rOut->right, rOut->top, rIn->right, rIn->top);
		HWLine(rOut->right, rOut->bottom, rIn->right, rIn->bottom);
	}

	// Back wall ?
	if (!ISDRAWOPEN(centre)) {
		HWLine(rIn->left, rIn->top, rIn->right, rIn->top);
		HWLine(rIn->left, rIn->bottom, rIn->right, rIn->bottom);
		if (!ISDRAWOPEN(left))				// Corner if left,right closed
			HWLine(rIn->left, rIn->top, rIn->left, rIn->bottom);
		if (!ISDRAWOPEN(right))
			HWLine(rIn->right, rIn->top, rIn->right, rIn->bottom);
	}

	// Set up for left side
	setRange(rOut->left, rIn->left,
		rOut->bottom,
		rOut->bottom - rOut->top,
		rIn->bottom - rIn->top);
	drawWall(s, left);

	// Set up for right side
	setRange(rIn->right, rOut->right,
		rIn->bottom,
		rIn->bottom - rIn->top,
		rOut->bottom - rOut->top);
	drawWall(s, right);

	// Set up for centre
	setRange(rIn->left, rIn->right,
		rIn->bottom,
		rIn->bottom - rIn->top,
		rIn->bottom - rIn->top);
	drawWall(s, centre);

	if (room == DT_LADDERUP) {
		r = Common::Rect(rOut->left, rOut->top, rOut->right, rIn->top);
		_DRAWPit(s, &r, 1);
	}
	if (room == DT_LADDERDN || room == DT_PIT) {
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
	if (room == DT_LADDERUP || room == DT_LADDERDN) {
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

	// Scale for monsters/gold. Scale factor has been empirically chosen
	Scale = 0.35 / (r.right - r.left) * s->w;

	// Monster here ?
	if (monster > 0) {
		HWColour(COL_MONSTER);
		Monster::draw(s, (r.left + r.right) / 2, r.bottom, monster, Scale);
	}

	// Draw the gold (as a mimic)
	if (room == DT_GOLD) {
		HWColour(COL_MONSTER);
		Monster::draw(s, (r.left + r.right) / 2, r.bottom, MN_MIMIC, Scale);
	}
}

void Dungeon::setRange(int x1, int x2, int y, int yd1, int yd2) {
	_xLeft = x1; _xRight = x2;				// Set x ranges
	_yBottom = y;							// Set lower left y value
	_yDiffLeft = yd1; _yDiffRight = yd2;	// Set difference for either end
}

void Dungeon::drawWall(Graphics::ManagedSurface *s, int n) {
	int x1, y1, x2, y2;
	int color = 0;

	if (n == DT_DOOR) {
		HWColour(COL_DOOR);
		x1 = 35; y1 = 0; x2 = 35; y2 = 60;
		drawConvert(&x1, &y1);
		drawConvert(&x2, &y2);
		HWLine(x1, y1, x2, y2);
		x1 = 65; y1 = 60; drawConvert(&x1, &y1);
		HWLine(x1, y1, x2, y2);
		x2 = 65; y2 = 0; drawConvert(&x2, &y2);
		HWLine(x1, y1, x2, y2);
	}
}

void Dungeon::drawConvert(int *px, int *py) {
	long x, y, yd;							// Longs for overflow in 16 bit
	x = (_xRight - _xLeft);					// Calculate width
	x = x * (*px) / 100 + _xLeft;			// Work out horiz value
	yd = (_yDiffRight - _yDiffLeft);		// Work out height of vert for x
	yd = yd * (*px) / 100;
	y = _yBottom +							// Half of the distance
		yd / 2 -							// + Scaled total size
		(yd + _yDiffLeft) * (*py) / 100;

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
