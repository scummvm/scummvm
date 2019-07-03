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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HDB_MENU_H
#define HDB_MENU_H

namespace HDB {

#define STARS_MONKEYSTONE_7			0xfe257d	// magic value in the config file for the unlocking of the Monkeystone secret #7
#define STARS_MONKEYSTONE_7_FAKE	0x11887e	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_14		0x3341fe	// <same> for the Monkeystone #14
#define STARS_MONKEYSTONE_14_FAKE	0x1cefd0	// fake value that means it hasn't been unlocked
#define STARS_MONKEYSTONE_21		0x77ace3	// <same> for the Monkeystone #21
#define STARS_MONKEYSTONE_21_FAKE	0x3548fe	// fake value that means it hasn't been unlocked

class Menu {
public:

	bool init();

	void startTitle();

	void fillSavegameSlots();

	int _starWarp;
};

} // End of Namespace

#endif // !HDB_SOUND_H
