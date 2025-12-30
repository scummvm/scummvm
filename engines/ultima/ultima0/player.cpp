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
/*							Manage Player Structures					*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/ultima0.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*					Initialise the player character						*/
/*																		*/
/************************************************************************/

void PLAYERInit(PLAYER *p) {
	p->init();
}

/************************************************************************/
/*																		*/
/*								Player Inventory						*/
/*																		*/
/************************************************************************/

void PLAYERInv(PLAYER *p) {
	int i;
	HWStatus(p->Object[OB_FOOD],			/* Refresh the status */
		p->Attr[AT_HP], p->Attr[AT_GOLD]);
	if (p->Name[0] != '\0')					/* Display the name */
		DRAWText("\n%s the %s.\n\n", p->Name, GLOClassName(p->Class));
	else
		DRAWText("\nA %s.\n\n", GLOClassName(p->Class));
	for (i = 0; i < p->Attributes; i++)		/* Display the attributes */
		if (i != AT_HP && i != AT_GOLD)
			DRAWText("%s: %d\n", GLOAttribName(i), p->Attr[i]);
	DRAWText("\n");
	for (i = 0; i < p->Objects; i++)			/* Display the objects */
		if (i != OB_FOOD)
			DRAWText("%s: %u\n", GLOObjName(i), (int)p->Object[i]);
	DRAWText("\n");
}

/************************************************************************/
/*																		*/
/*						Create a player character						*/
/*																		*/
/************************************************************************/

void PLAYERCharacter(PLAYER *p) {
	int i;
	DRAWText("Select skill level.\n");
	while (i = HWGetKey(), !Common::isDigit(i)) {
	};
	i = i & 15; if (i == 0) i = 10;
	DRAWText("Level %d.\n\n", i);

	DRAWText("Hit your lucky key.\n\n");
	p->LuckyNumber = HWGetKey();
	DRAWText("Game Number %d.\n\n", p->LuckyNumber);

	do
	{
		for (i = 0; i < p->Attributes; i++)
		{
			p->Attr[i] = floor(sqrt(RND()) * 21 + 4);
			DRAWText("%s : %d.\n", GLOAttribName(i), p->Attr[i]);
		}
		DRAWText("\nWilt thou play with\nthese qualities ?\n\n");
	} while (DEADGetYesNo() == 0);
	DRAWText("And shalt thou be a\nFighter or a Mage ?\n\n");
	do
	{
		p->Class = HWGetKey();
	} while (p->Class != 'M' && p->Class != 'F');
}

} // namespace Ultima0
} // namespace Ultima
