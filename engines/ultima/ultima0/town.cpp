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

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/************************************************************************/
/*																		*/
/*						Enter town (shopkeeper code)					*/
/*																		*/
/************************************************************************/
/************************************************************************/

void TOWNEnter(WORLDMAP *w, PLAYER *p) {
	int Key, i, Cost, Incr, Item;
	DRAWText("Welcome to the\nAdventure shop.\n");
	do      								/* Keep Buying */
	{
		HWStatus(p->Object[OB_FOOD],		/* Refresh the status bar */
			p->Attr[AT_HP], p->Attr[AT_GOLD]);
		DRAWText("\nWhat shalt thou buy?\n\n");
		for (i = 0; i < p->Objects; i++)		/* Price list */
		{
			GLOGetInfo(i, nullptr, &Cost, nullptr);
			DRAWText("%s: %.0f (%d GP)\n", GLOObjName(i), floor(p->Object[i]), Cost);
		}
		Key = HWGetKey(); Item = -1;			/* Get item to buy */
		for (i = 0; i < p->Objects; i++)		/* Find which one it was */
		{
			GLOGetInfo(i, nullptr, nullptr, &Cost);
			if (Cost == Key) Item = i;
		}
		if (p->Class == 'M')				/* Some things mages can't use */
			if (Item == OB_BOW || Item == OB_RAPIER)
			{
				DRAWText("\nI'm sorry, Mages\ncan't use that.\n");
				Item = -1;
			}

		if (Item >= 0)						/* Something selected */
		{
			GLOGetInfo(Item, nullptr, &Cost, nullptr);
			if (Cost > p->Attr[AT_GOLD])
				DRAWText("\nM'Lord thou cannot\nafford that item.\n");
			else
			{
				p->Attr[AT_GOLD] -= Cost;   /* Lose the money */
				HWStatus(p->Object[OB_FOOD],/* Refresh the status bar */
					p->Attr[AT_HP], p->Attr[AT_GOLD]);
				Incr = 1; if (Item == OB_FOOD) Incr = 10;
				p->Object[Item] += Incr;	/* Add 1 or 10 on */
			}
		}
	} while (Key != 'Q');
	DRAWText("\nBye.\n\n");
}

} // namespace Ultima0
} // namespace Ultima
