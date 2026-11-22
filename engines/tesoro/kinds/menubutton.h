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

#ifndef TESORO_KINDS_MENUBUTTON_H
#define TESORO_KINDS_MENUBUTTON_H

#include "tesoro/kinds/interactive.h"

namespace Tesoro {

class World;

/**
 * Object kind 22: a button of the save and load screen. The first data byte is the action: 0 and 1 choose the
 * previous or next saved game, 2 loads, 3 saves, 5 fades out and clears the main list (the frame ends), 6 scrolls the
 * interface out, 7 switches the subtitles.
 */
class MenuButton : public InteractiveObject {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now, World &world);

	bool acceptsPointer() override;
	int showHoverText() override { return 1; }
	int clearHoverText() override;
	int setVerbAndClick(int verb) override;

	int action() const { return _action; }

private:
	uint _action = 0;
};

} // End of namespace Tesoro

#endif
