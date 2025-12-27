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
/*						Attacking Monsters Code							*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

static int _MONSTAttack(MONSTER *, PLAYER *);
static int _MONSTSteal(MONSTER *, PLAYER *);
static void _MONSTMove(MONSTER *, PLAYER *, DUNGEONMAP *);
static int _MONSTCanMoveTo(DUNGEONMAP *, int, int);

/************************************************************************/
/*																		*/
/*						Check Monsters Attacking						*/
/*																		*/
/************************************************************************/

void MONSTAttack(PLAYER *p, DUNGEONMAP *d) {
	int i, Attacked;
	double Dist;
	MONSTER *m;
	for (i = 0; i < d->MonstCount; i++)			/* Go through all monsters */
	{
		m = &(d->Monster[i]);					/* Pointer to monster */
		Dist = pow(m->Loc.x - p->Dungeon.x, 2);	/* Calculate Distance */
		Dist = Dist + pow(m->Loc.y - p->Dungeon.y, 2);
		Dist = sqrt(Dist);
		if (m->Alive != 0)						/* If alive */
		{
			Attacked = 0;
			if (Dist < 1.3)					/* If within range */
				Attacked = _MONSTAttack(m, p);
			if (Attacked == 0)				/* If didn't attack, then move */
			{
				if (m->Type != MN_MIMIC ||  /* Mimics only if near enough */
					Dist >= 3.0)
					_MONSTMove(m, p, d);
				if (m->Strength <			/* Recovers if didn't attack */
					p->Level * p->Skill)
					m->Strength = m->Strength + p->Level;
			}
		}
	}
}

/************************************************************************/
/*																		*/
/*							Monster Attacks								*/
/*																		*/
/************************************************************************/

static int _MONSTAttack(MONSTER *m, PLAYER *p) {
	int n;

	if (m->Type == MN_GREMLIN ||		/* Special case for Gremlin/Thief */
		m->Type == MN_THIEF)
		if (RND() > 0.5)			/* Half the time */
			return _MONSTSteal(m, p);

	DRAWText("You are being attacked by a %s !!!.\n",
		GLOMonsterName(m->Type));

	n = urand() % 20;					/* Calculate hit chance */
	if (p->Object[OB_SHIELD] > 0) n--;
	n = n - p->Attr[AT_STAMINA];
	n = n + m->Type + p->Level;
	if (n < 0)							/* Missed ! */
		DRAWText("Missed !\n");
	else								/* Hit ! */
	{
		n = urand() % (m->Type);			/* Calculate damage done. */
		n = n + p->Level;
		p->Attr[AT_HP] -= n;			/* Adjust hit points */
		DRAWText("Hit !!!\n");
	}
	return 1;
}

/************************************************************************/
/*																		*/
/*								Monster Moves							*/
/*																		*/
/************************************************************************/

static void _MONSTMove(MONSTER *m, PLAYER *p, DUNGEONMAP *d) {
	int x, y, xi, yi;

	xi = yi = 0;						/* Calculate direction */
	if (p->Dungeon.x != m->Loc.x)
		xi = (p->Dungeon.x > m->Loc.x) ? 1 : -1;
	if (p->Dungeon.y != m->Loc.y)
		yi = (p->Dungeon.y > m->Loc.y) ? 1 : -1;

	/* Running away */
	if (m->Strength < p->Level * p->Skill)
	{
		xi = -xi; yi = -yi;
	}
	x = m->Loc.x; y = m->Loc.y;			/* Get position */

	if (abs(xi) > abs(yi))				/* Check move okay */
	{
		if (_MONSTCanMoveTo(d, x + xi, yi)) yi = 0;
		else if (_MONSTCanMoveTo(d, x, y + yi)) xi = 0;
	} else
	{
		if (_MONSTCanMoveTo(d, x, y + yi)) xi = 0;
		else if (_MONSTCanMoveTo(d, x + xi, yi)) yi = 0;
	}
	if (xi == 0 && yi == 0) return;		/* No move */
	x = x + xi; y = y + yi;				/* Work out new position */
	if (_MONSTCanMoveTo(d, x, y) == 0)	/* Fail if can't move there */
		return;
	if (x == p->Dungeon.x &&			/* Can't move onto us */
		y == p->Dungeon.y) return;
	m->Loc.x = x; m->Loc.y = y;			/* Move to new position */
}

/************************************************************************/
/*																		*/
/*						Can monster move to a square					*/
/*																		*/
/************************************************************************/

static int _MONSTCanMoveTo(DUNGEONMAP *d, int x, int y) {
	COORD c;
	int t = d->Map[x][y];				/* See what's there */
	if (!ISWALKTHRU(t)) return 0;		/* Can't walk through walls */
	c.x = x; c.y = y;					/* Set up coord structure */
	return DDRAWFindMonster(d, &c) < 0;	/* True if no monster here */
}

/************************************************************************/
/*																		*/
/*							Monster Stealing							*/
/*																		*/
/************************************************************************/

static int _MONSTSteal(MONSTER *m, PLAYER *p) {
	int n;
	const char *s1, *s2;

	if (m->Type == MN_GREMLIN)			/* Gremlin */
	{
		p->Object[OB_FOOD] =			/* HALVES the food.... aargh */
			floor(p->Object[OB_FOOD]) / 2.0;
		DRAWText("A Gremlin stole some food.\n");
	}

	if (m->Type == MN_THIEF)			/* Thief */
	{
		do                       		/* Figure out what stolen */
			n = urand() % (p->Objects);
		while (p->Object[n] == 0);
		p->Object[n]--;					/* Stole one */
		s2 = GLOObjName(n); s1 = "a";
		if (strchr("aeiou", tolower(*s2))) s1 = "an";
		if (n == 0) s1 = "some";
		DRAWText("A Thief stole %s %s.\n", s1, s2);
	}
	return 1;
}

} // namespace Ultima0
} // namespace Ultima
