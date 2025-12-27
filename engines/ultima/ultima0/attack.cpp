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
/*						Attack any nearby monsters						*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

static void _ATTACKWeapon(PLAYER *, DUNGEONMAP *, int);
static int  _ATTACKMissile(PLAYER *, DUNGEONMAP *, int);
static void _ATTACKKill(MONSTER *, PLAYER *);
static void _ATTACKAmulet(PLAYER *, DUNGEONMAP *);
static void _ATTACKHitMonster(PLAYER *, DUNGEONMAP *, int, COORD *);

/************************************************************************/
/*																		*/
/*					Attack any nearby monsters							*/
/*																		*/
/************************************************************************/

void ATTACKMonster(PLAYER *p, DUNGEONMAP *d) {
	int c, Key, i, n, Damage;
	DRAWText("With what ? ");				/* Which weapon */
	c = HWGetKey();
	n = -1;									/* Discover which. */
	for (i = 0; i < p->Objects; i++)
	{
		GLOGetInfo(i, nullptr, nullptr, &Key);		/* Get the associated key */
		if (toupper(Key) == c)				/* Found the object ? */
			if (i != OB_FOOD) n = i;/* Not food ! */
	}
	if (n < 0)           					/* Check the result */
	{
		Damage = 0; DRAWText("Hands.\n");
	} else
	{
		DRAWText("%s.\n", GLOObjName(n));
		GLOGetInfo(n, &Damage, nullptr, nullptr);
	}

	if (p->Object[n] == 0)					/* Must own an object */
	{
		DRAWText("Not owned.\n");
		return;
	}
	if (p->Class == 'M' &&					/* Mages are limited */
		(n == OB_BOW || n == OB_RAPIER))
	{
		DRAWText("Mages can't use\n%ss.\n", GLOObjName(n));
		return;
	}

	if (n == OB_AMULET)						/* Use an amulet */
	{
		_ATTACKAmulet(p, d);
		return;
	}

	if (n == OB_BOW || n == OB_AXE)			/* Check for axe work */
	{
		if (_ATTACKMissile(p, d, n) == 0)		/* Throw code */
			_ATTACKWeapon(p, d, n);	/* if not thrown, use as weapon */
	} else
		_ATTACKWeapon(p, d, n);				/* use as bashing weapon */
}

/************************************************************************/
/*																		*/
/*				Attack using object as a hand held weapon				*/
/*																		*/
/************************************************************************/

static void _ATTACKWeapon(PLAYER *p, DUNGEONMAP *d, int Weapon) {
	COORD c;
	c.x = p->Dungeon.x + p->DungDir.x;		/* Look at what's there */
	c.y = p->Dungeon.y + p->DungDir.y;
	_ATTACKHitMonster(p, d, Weapon, &c);
}

/************************************************************************/
/*																		*/
/*						Use weapon on monster							*/
/*																		*/
/************************************************************************/

static void _ATTACKHitMonster(PLAYER *p, DUNGEONMAP *d, int Weapon, COORD *c) {
	int n = 0, Monster, Damage;
	MONSTER *m = nullptr;

	Monster = DDRAWFindMonster(d, c);		/* Is there a monster there ? */
	if (Monster >= 0)						/* Set up a pointer */
	{
		m = &(d->Monster[Monster]);
		n = m->Type;
	}

	Damage = 0;								/* Get weaponry info */
	if (Weapon >= 0 && Weapon != OB_AMULET)
		GLOGetInfo(Weapon, &Damage, nullptr, nullptr);
	if (Weapon == OB_AMULET)       			/* Amulet Special Case */
		Damage = 10 + p->Level;

	if (Monster < 0 ||						/* If no, or not dexterous */
		p->Attr[AT_DEXTERITY] - ((int)urand() % 25) < n + p->Level)
	{
		DRAWText("You missed !!\n");		/* Then a miss. */
		return;
	}

	DRAWText("Hit !!!\n");					/* Scored a hit */
	n = 0;  								/* Calculate HPs lost */
	if (Damage > 0) n = (urand() % Damage);
	n = n + p->Attr[AT_STRENGTH] / 5;
	m->Strength = m->Strength - n;			/* Lose them */
	if (m->Strength < 0) m->Strength = 0;
	DRAWText("%s's Hit\nPoints now %d.\n",		/* Display the HP Value */
		GLOMonsterName(m->Type), m->Strength);
	if (m->Strength == 0)_ATTACKKill(m, p);	/* Killed it ? */

}

/************************************************************************/
/*																		*/
/*						   Kill off a monster							*/
/*																		*/
/************************************************************************/

static void _ATTACKKill(MONSTER *m, PLAYER *p) {
	int n;
	m->Alive = 0;							/* Deaded */
	n = (m->Type + p->Level);				/* Amount of Gold */
	DRAWText("You get %d\npieces of eight.\n", n);
	p->Attr[AT_GOLD] += n;
	p->HPGain += (m->Type * p->Level) / 2;	/* Calculate Gain */
	if (m->Type == p->Task)					/* Check done LB's task */
		p->TaskCompleted = 1;
}

/************************************************************************/
/*																		*/
/*					Attack using object as a missile					*/
/*																		*/
/************************************************************************/

static int _ATTACKMissile(PLAYER *p, DUNGEONMAP *d, int Weapon) {
	int n, y, Dist;
	char ch;
	COORD c, c1;
	if (Weapon == OB_AXE)				/* Axes can be thrown or swung */
	{
		DRAWText("Throw or Swing ? ");
		while (ch = HWGetKey(), ch != 'T' && ch != 'S') {
		}
		DRAWText(ch == 'T' ? "Throw.\n" : "Swing.\n");
		if (ch == 'S') return 0;
		p->Object[OB_AXE]--;			/* Lose a thrown axe */
	}
	c.x = p->Dungeon.x;					/* See what's to hit */
	c.y = p->Dungeon.y;
	Dist = -1;
	for (y = 0; y < 5; y++)        		/* A maximum distance of 5 */
	{
		c.x = c.x + p->DungDir.x;		/* Next position */
		c.y = c.y + p->DungDir.y;
		n = DDRAWFindMonster(d, &c);		/* Monster there ? */
		if (n >= 0) {
			c1 = c; Dist = n;
		}	/* If so , record that */
		if (!ISDRAWOPEN					/* If wall, or door, stop */
		(d->Map[c.x][c.y])) y = 99;
	}
	if (Dist < 0)						/* Hit nothing */
		DRAWText("You missed !!\n");	/* Then a miss. */
	else
		_ATTACKHitMonster(p, d, Weapon, &c1);
	return 1;
}

/************************************************************************/
/*																		*/
/*							Use an amulet								*/
/*																		*/
/************************************************************************/

static void _ATTACKAmulet(PLAYER *p, DUNGEONMAP *d) {
	int i, Magic = urand() % 4;
	if (p->Class == 'M')				/* Mages use them properly ! */
	{
		DRAWText("1] Ladder Up\n");
		DRAWText("2] Ladder Down\n");
		DRAWText("3] Attack Monster\n");
		DRAWText("4] Bad Magic\n");
		do
			Magic = HWGetKey() - '1';
		while (Magic < 0 || Magic > 3);
	}
	if (urand() % 5 == 0)				/* Last charge */
	{
		DRAWText("Last charge on this Amulet.\n");
		p->Object[OB_AMULET]--;
	}
	switch (Magic)
	{
	case 0:							/* Ladder up */
		d->Map[p->Dungeon.x][p->Dungeon.y] = DT_LADDERUP;
		break;
	case 1:							/* Ladder down */
		d->Map[p->Dungeon.x][p->Dungeon.y] = DT_LADDERDN;
		break;
	case 2:							/* Amulet Attack */
		_ATTACKMissile(p, d, OB_AMULET);
		break;
	case 3:   						/* Bad Magic */
		switch (urand() % 3)
		{
		case 0:
			DRAWText("You have been turned into a Toad.\n");
			for (i = AT_STRENGTH; i <= AT_WISDOM; i++)
				p->Attr[i] = 3;
			break;
		case 1:
			DRAWText("You have been turned into a Lizard Man.\n");
			for (i = AT_HP; i <= AT_WISDOM; i++)
				p->Attr[i] = floor(p->Attr[i] * 5 / 2);
			break;
		case 2:
			DRAWText("Backfire !!\n");
			p->Attr[AT_HP] = floor(p->Attr[AT_HP]) / 2;
			break;
		}
		break;
	}
}

} // namespace Ultima0
} // namespace Ultima
