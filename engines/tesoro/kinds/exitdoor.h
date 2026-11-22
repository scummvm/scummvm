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

#ifndef TESORO_KINDS_EXITDOOR_H
#define TESORO_KINDS_EXITDOOR_H

#include "tesoro/kinds/interactive.h"

namespace Tesoro {

class World;

/**
 * Object kind 6: An exit of a level.
 * The subtype (the data's second byte) lets the game act differently depending on the particular exit.
 */
class ExitDoor : public InteractiveObject {
public:

	void init(int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world);

	/** Verb 0 (go through) and, for one subtype, verb 4 (switch on or off). */
	int verb(int verb) override;
	int doubleClick(int verb) override;
	/** Jim comes out next to this exit. */
	int exitArrival() override;

	int subtype() const { return _subtype; }

private:
	int goThrough();
	int cabinArrival();
	int labArrival();
	/** What a double click does here: Jim is put at the exit at once. */
	int jumpThrough(int verb);
	int runWith(const char *block, const char *label, byte *variables);
	/** The door's own state bytes, or null when it keeps none. */
	byte *state() { return _stateBlob.empty() ? nullptr : _stateBlob.data(); }

	int _subtype = 0;
	int8 _destLevel = 0;
	uint64 _destKey = 0;
	int _arrivalFacing = 0;
};

} // End of namespace Tesoro

#endif
