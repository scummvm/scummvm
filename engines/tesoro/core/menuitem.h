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

#ifndef TESORO_CORE_MENUITEM_H
#define TESORO_CORE_MENUITEM_H

#include "tesoro/core/object.h"

namespace Tesoro {

class TesoroEngine;

/**
 * Object kind 25: a clickable menu item
 */
class MenuItem : public Object {
public:
	void init(const ObjectRecord &record, int16 roomSortOffset, const Common::Array<AnimPlayer> &levelPlayers, int32 now);

	/** Where the clicks go. Without a host a click does nothing. */
	void setHost(TesoroEngine *host) { _host = host; }

	/** The number from the level data that picks the item's action. */
	uint32 action() const { return _action; }

	void updateAnimation(int32 now) override;
	void draw(const Camera &camera, Display &display) override;

	// menu items always take the pointer
	bool acceptsPointer() override { return true; }
	int showHoverText() override { return 1; }

	int update() override;
	int setVerbAndClick(int verb) override;
	int rightClick(int verb) override;
	int clearHoverText() override;

private:
	uint32 _action = 0;
	TesoroEngine *_host = nullptr;
};

} // End of namespace Tesoro

#endif
