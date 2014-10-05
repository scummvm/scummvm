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

#ifndef WINTERMUTE_GAME_FLAGS_H
#define WINTERMUTE_GAME_FLAGS_H

namespace Wintermute {

enum WintermuteGameFlags {
	WINTERMUTE_NO_FLAGS = 0,
	/*
	 * The behaviour of BaseSprite::reset() changed in WME 1.8.6, when it
	 * started calling killAllSounds() as well; some games such as 5MA
	 * and possibly others rely on the old behaviour. See bug #6647
	 */
	WINTERMUTE_NO_KILLALLSOUNDS = 1
};

}

#endif /* WINTERMUTE_GAME_FLAGS_H_ */
