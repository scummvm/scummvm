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
/*						Moving and Entry/Exit Code						*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*								Movement code							*/
/*																		*/
/************************************************************************/

void MOVEMove(int c, WORLDMAP *w, PLAYER *p, DUNGEONMAP *d, int xi, int yi) {
	int z, x1, y1, Moved = 0;
	const char *s1, *s2;
	double n;
	COORD New;
	if (p->Level == 0)						/* If above ground in world map */
	{
		x1 = p->World.x + xi;				/* Calculate new position */
		y1 = p->World.y + yi;
		if (WORLDRead(w, x1, y1) == WT_MOUNTAIN)
			DRAWText("You can't pass\nthe mountains.\n");
		else
		{
			p->World.x = x1;				/* Move */
			p->World.y = y1;
		}
	} else									/* If in the dungeon */
	{
		switch (c)
		{
		case 'N':	x1 = p->Dungeon.x + p->DungDir.x;
			y1 = p->Dungeon.y + p->DungDir.y;
			New.x = x1; New.y = y1;
			if (ISWALKTHRU(d->Map[x1][y1]) &&
				DDRAWFindMonster(d, &New) < 0)
			{
				Moved = 1;
				p->Dungeon = New;
			}
			break;

		case 'E':   MOVERotLeft(&(p->DungDir));
			MOVERotLeft(&(p->DungDir));
			MOVERotLeft(&(p->DungDir));
			break;
		case 'W':   MOVERotLeft(&(p->DungDir));
			break;
		case 'S':   MOVERotLeft(&(p->DungDir));
			MOVERotLeft(&(p->DungDir));
			break;
		}

		if (Moved != 0)
		{
			n = d->Map[p->Dungeon.x]		/* What's here ? */
				[p->Dungeon.y];
			if (n == DT_PIT)				/* Fell in a pit */
			{
				p->Level++;					/* Down a level */
				DRAWText("Aaarrrgghhh! A Trap !\n");
				p->Attr[AT_HP] -=			/* It hurts ! */
					(3 + urand() % (3 * p->Level));
				DRAWText("Falling to Level %d.\n", p->Level);
				DUNGEONCreate(p, d);			/* Create the new level */
			}
			if (n == DT_GOLD)				/* Gold here */
			{
				d->Map[p->Dungeon.x]		/* Remove the gold */
					[p->Dungeon.y] = 0;
				DRAWText("Gold !!!!!\n");
				z = (urand() % (5 * p->Level))	/* Calculate amount */
					+ p->Level;
				DRAWText("%d pieces of eight ", z);
				p->Attr[AT_GOLD] += z;		/* Add to total */
				if (z > 0)					/* Object ? */
				{
					z = urand() % (p->Objects);/* Decide which object */
					s2 = GLOObjName(z);		/* Get the name */
					s1 = "a";				/* Decide a,an or some */
					if (strchr("aeiou", tolower(*s2))) s1 = "an";
					if (z == 0) s1 = "some";
					DRAWText("and %s %s.\n", s1, s2);
					p->Object[z]++;			/* Bump the total */
				}
			}
		}
		MONSTAttack(p, d);					/* Monster attacks ? */
	}
	n = p->Object[OB_FOOD];
	p->Object[OB_FOOD] = p->Object[OB_FOOD]	/* Consume some food */
		- ((p->Level > 0) ? 0.1 : 1);
	p->Object[OB_FOOD] =					/* Make it integer */
		floor(p->Object[OB_FOOD] * 10) / 10;
	if (p->Object[OB_FOOD] <= 0)			/* Starved us ? */
	{
		DRAWText("You have starved...\n");
		p->Attr[AT_HP] = 0;					/* Dead */
	}
}


/************************************************************************/
/*																		*/
/*							Handle entry,exit							*/
/*																		*/
/************************************************************************/

void MOVEEnterExit(WORLDMAP *w, PLAYER *p, DUNGEONMAP *d) {
	int t, Done = 0;
	if (p->Level == 0)					/* Entry/Exit on world map */
	{
		t = WORLDRead(w, p->World.x,		/* Read tile */
			p->World.y);
		Done = 1;
		switch (t)
		{
		case WT_TOWN:					/* Enter the town */
			DRAWText("Enter Town.\n");
			TOWNEnter(w, p);
			break;
		case WT_DUNGEON:				/* Enter the dungeon */
			DRAWText("Enter Dungeon.\n");
			p->Level = 1;				/* Go to level 1 */
			p->Dungeon.x = 1;			/* Set initial position */
			p->Dungeon.y = 1;
			p->DungDir.x = 1;			/* And direction */
			p->DungDir.y = 0;
			DUNGEONCreate(p, d);			/* Create the dungeon map */
			break;
		case WT_BRITISH:				/* Enter the castle */
			DRAWText("Enter Castle.\n");
			BRITISHEnter(w, p);
			break;
		default:           				/* Nope.... */
			Done = 0; break;
		}
	} else								/* Entry/Exit in dungeons */
	{
		t = d->Map[p->Dungeon.x]		/* Identify what's there */
			[p->Dungeon.y];
		if (t == DT_LADDERUP)			/* Climbing up a ladder */
		{
			p->Level--;
			Done = 1;
			if (p->Level == 0)
			{
				DRAWText("Leave the Dungeon.\n");
				DRAWText("Thou has gained %d hit points.\n", p->HPGain);
				p->Attr[AT_HP] += p->HPGain;
				p->HPGain = 0;
			} else
				DRAWText("Go up to Level %d.\n", p->Level);
		}
		if (t == DT_LADDERDN)			/* Climbing down a ladder */
		{
			p->Level++;
			Done = 1;
			DRAWText("Go down to Level %d.\n", p->Level);
		}
		if (Done != 0 && p->Level > 0)	/* New Dungeon Map Required */
			DUNGEONCreate(p, d);
	}
	if (Done == 0)                   	/* Can't do it */
		DRAWText("Huh???\n");
}

/************************************************************************/
/*																		*/
/*						Rotate a direction left							*/
/*																		*/
/************************************************************************/

void MOVERotLeft(COORD *Dir) {
	int t;
	if (Dir->y == 0) Dir->x = -Dir->x;
	t = Dir->x;
	Dir->x = Dir->y;
	Dir->y = t;
}

} // namespace Ultima0
} // namespace Ultima
