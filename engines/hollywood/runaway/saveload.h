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

#ifndef HOLLYWOOD_RUNAWAY_SAVELOAD_H
#define HOLLYWOOD_RUNAWAY_SAVELOAD_H

#include "common/serializer.h"
#include "hollywood/runaway/detection.h"

namespace Hollywood {

// Persistent state survives scene changes. Saves are made between actions;
// resources, pointers and animation clocks are rebuilt when restoring it.
struct RunawaySaveState {
	struct Hospital {
		byte glassTaken = 0, pillsTaken = 0, sheetTaken = 0, bagTaken = 0, exitMapRead = 0;
		byte markerTaken = 0, alcohol = 0;
		byte window = 0, chartReplaced = 0, chartMoved = 0, decoy = 0, attacked = 0, medicineRead = 0;
		byte storeroomVisited = 0, syringesSeen = 0, sprayTaken = 0, book = 0, blankChart = 0, headTaken = 0, pillowsTaken = 0;
	} hospital;

	uint16 scene = 5030, x = 623, y = 447, camera = 111;
	byte facing = 3, lidOpen = 0, unplugged = 0, windowClean = 0;
	byte coolerMoved = 0, coolerInspected = 0, peeked = 0, ice = 0, hatch = 0;
	byte pickedUp[20] = {}, examined[20] = {};
	byte itemCount = 0, items[26] = {}, selected = 0, cloth = 0;
	byte inventoryOpen = 0, verb = 0;
	uint32 randomSeed = 0, playTime = 0;
	byte game = kRunawayCabinDemo;
	uint16 cameraY = 0;
	byte upperFloor = 1, books = 0, recorder = 0, moneyIdentified = 0, safeOpen = 0;
	byte varnishTaken = 0, talcTaken = 0, coffeeTaken = 0, keyCount = 0;
	byte water = 0;
	byte markerInk = 0, syringeFilled = 0, chartWritten = 0, headWig = 0;

	byte chapter() const;
	bool valid() const;
	bool read(Common::SeekableReadStream &stream);
	bool write(Common::WriteStream &stream);

private:
	void sync(Common::Serializer &s);
};

} // End of namespace Hollywood

#endif
