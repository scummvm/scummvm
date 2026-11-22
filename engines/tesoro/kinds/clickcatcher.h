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

#ifndef TESORO_KINDS_CLICKCATCHER_H
#define TESORO_KINDS_CLICKCATCHER_H

#include "tesoro/kinds/interactive.h"

namespace Tesoro {

class World;

/** Object kind 15: a region to catch the click and prevent it from affecting other objects.
 * used for the panel behind the interface elements to prevent clicks.
 */
class ClickCatcher : public InteractiveObject {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world);

	int setVerbAndClick(int verb) override { return 1; }
	int showHoverText() override { return 1; }

protected:
	void onAttach() override;
};

} // End of namespace Tesoro

#endif
