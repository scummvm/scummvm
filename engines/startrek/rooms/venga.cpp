/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "startrek/room.h"

namespace StarTrek {

void Room::vengaUseMccoyOnDeadGuy() {
	int val = getRandomWordInRange(0, 5);

	switch (val) {
	case 0:
		// NOTE: The exact text differs from the text used in DEM3. But, in order for this
		// text indexing scheme to work, two rooms can't use the same audio with different
		// text. Original was "He's dead, jim." instead of "He's dead, Jim...".
		showText(TX_SPEAKER_MCCOY, TX_DEM3_019);
		break;

	case 1:
		showText(TX_SPEAKER_MCCOY, TX_VEN4_016);
		break;

	case 2:
	case 3:
		showText(TX_SPEAKER_MCCOY, TX_G_014);
		break;

	case 4:
	case 5:
		showText(TX_SPEAKER_MCCOY, TX_VEN0_016);
		break;
	}
}

}
