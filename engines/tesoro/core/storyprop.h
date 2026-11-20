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

#ifndef TESORO_CORE_STORYPROP_H
#define TESORO_CORE_STORYPROP_H

#include "tesoro/core/interactive.h"

namespace Tesoro {

class World;

class Level;

/**
 * Object kinds 12, 17 and 20 of the level files (the subtype picks which): a generic selectable hotspot of the scenery.
 */
class StoryProp : public InteractiveObject {
public:

	void init(int kind, int subtype, const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &host);

	int verb(int verb) override;
	int useItem(int item) override;

	int kind() const { return _propKind; }
	int subtype() const { return _subtype; }

private:
	int verb12(int verb);
	int verb17(int verb);
	int verb20(int verb);
	int use12(int item);
	int use17(int item);
	int use20(int item);

	// start scripts with block name and label
	int run(const char *block, const char *label, bool withState);
	int runWith(const char *block, const char *label, byte *variables);

	byte stateOf(int level, const char *name, uint index);
	void setItemActive(int level, const char *name, bool active);

	void pickUp(const char *item);
	byte *state() { return _stateBlob.empty() ? nullptr : _stateBlob.data(); }

	int _propKind = 0;
	int _subtype = 0;
};

} // End of namespace Tesoro

#endif
