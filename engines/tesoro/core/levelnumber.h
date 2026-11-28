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

#ifndef TESORO_CORE_LEVELNUMBER_H
#define TESORO_CORE_LEVELNUMBER_H

namespace Tesoro {

/** The numbers of the levels that the objects and scripts refer to by name (-1 is the interface). */
enum LevelNumber {
	kInterface = -1,
	kOffice = 0,
	kPort = 1,
	kTavern = 2,
	kCabin = 4,
	kKitchen = 6,
	kBeach = 7,
	kMap = 8,
	kVillage = 9,
	kSwamp = 10,
	kVolcano = 12,
	kEarth = 15,
	kRisco = 16
};

} // End of namespace Tesoro

#endif
