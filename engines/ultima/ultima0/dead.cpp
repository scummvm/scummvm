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
/*								Dead Code								*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*								You have died							*/
/*																		*/
/************************************************************************/

void DEADCode(PLAYER *p) {
	const char *s = p->Name;
	if (*s == '\0') s = "the peasant";
	DRAWText("\nWe mourn the passing of %s and his computer.\n\n", s);
	DRAWText("Dost thou wish resurrection ?\n\n");

	if (DEADGetYesNo())
	{
		DRAWText("Yes.\n");
		MAINSetup();
	}
}

/************************************************************************/
/*																		*/
/*							Get a yes or no								*/
/*																		*/
/************************************************************************/

int DEADGetYesNo(void) {
	char c;
	do
		c = HWGetKey();
	while (c != 'Y' && c != 'N');
	return (c == 'Y');
}

} // namespace Ultima0
} // namespace Ultima
