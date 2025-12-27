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
/*							Dungeon Drawing Code						*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

static void _DDRAWCalcRect(RECT *, double);

/************************************************************************/
/*																		*/
/*							Draw the dungeon							*/
/*																		*/
/************************************************************************/

void DDRAWDraw(PLAYER *p, DUNGEONMAP *d) {
	double Level = 0;
	RECT rOut, rIn;
	COORD Dir, Pos, Next;
	int Monster, Front, Left, Right;
	DRAWSetRect(&rOut, 0, 1023, 1260, 10);
	_DDRAWCalcRect(&rOut, 0);
	Pos = p->Dungeon;						/* Get position */
	do
	{
		Level++;							/* Next level */
		_DDRAWCalcRect(&rIn, Level);
		Next.x = Pos.x + p->DungDir.x;		/* Next position */
		Next.y = Pos.y + p->DungDir.y;

		Dir = p->DungDir; MOVERotLeft(&Dir);	/* To the left */
		Left = d->Map[Pos.x + Dir.x][Pos.y + Dir.y];
		MOVERotLeft(&Dir); MOVERotLeft(&Dir);/* To the right */
		Right = d->Map[Pos.x + Dir.x][Pos.y + Dir.y];
		Front = d->Map[Next.x][Next.y];		/* What's in front ? */

		Monster = DDRAWFindMonster(d, &Pos);	/* Find ID of monster here */
		if (Monster >= 0)					/* Find Type if Found */
		{
			Monster = d->Monster[Monster].Type;
		}
		DRAWDungeon(&rOut, &rIn,				/* Draw the dungeon */
			Left, Front, Right,
			d->Map[Pos.x][Pos.y], Monster);

		Pos = Next;							/* Next position down */
		rOut = rIn;							/* Last in is new out */
	} while (Level < MAX_VIEW_DEPTH && ISDRAWOPEN(Front));
}

/************************************************************************/
/*																		*/
/*					Calculate display rectangle							*/
/*																		*/
/************************************************************************/

static void _DDRAWCalcRect(RECT *r, double Level) {
	int xWidth, yWidth;
	xWidth = (int)						/* Calculate frame size */
		(atan(1.0 / (Level + 1)) / atan(1.0) * 1279 + 0.5);
	xWidth = 1279 / (Level + 1);
	yWidth = xWidth * 10 / 13;
	r->left = 640 - xWidth / 2;			/* Calculate drawing rectangle */
	r->right = 640 + xWidth / 2;
	r->top = 512 + yWidth / 2;
	r->bottom = 512 - yWidth / 2;
}

/************************************************************************/
/*																		*/
/*					Find Monster ID at given location					*/
/*																		*/
/************************************************************************/

int DDRAWFindMonster(DUNGEONMAP *d, COORD *c) {
	int i, n = -1;
	for (i = 0; i < d->MonstCount; i++)
		if (c->x == d->Monster[i].Loc.x &&
			c->y == d->Monster[i].Loc.y &&
			d->Monster[i].Alive != 0) n = i;
	return n;
}

} // namespace Ultima0
} // namespace Ultima
