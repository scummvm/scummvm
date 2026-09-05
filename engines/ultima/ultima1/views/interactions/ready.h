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

#ifndef ULTIMA1_VIEWS_INTERACTIONS_READY_H
#define ULTIMA1_VIEWS_INTERACTIONS_READY_H

#include "ultima/ultima1/views/interactions/interaction.h"

namespace Ultima {
namespace Ultima1 {
namespace Views {
namespace Interactions {

using namespace Ultima::Shared::Messages;

/**
 * Handler for the Ready command
 */
class Ready : public Interaction {
	enum Mode {
		READY_AREA, READY_WEAPON, READY_ARMOUR, READY_SPELL
	};
private:
	Mode _mode = READY_AREA;

	void nothing();
	void ready(const char *name);

	/**
	 * Handles a letter keypress made while selecting from a list of
	 * items - validates it against the owned items array, falling back
	 * to the currently equipped index if it's out of range or refers to
	 * an item that isn't owned, then readies the result
	 */
	void selectItem(const int16 *items, const char *const *names, int maxIndex,
		int16 &equippedIndex, const char *label, Common::KeyCode keycode);

	/**
	 * Draws the centered, lettered list of items selectable in the
	 * current mode
	 */
	void drawItemList(const int16 *items, const char *const *names, int maxIndex);

public:
	Ready();
	~Ready() override {
	}

	void draw() override;
	bool msgFocus(const FocusMessage &msg) override;
	bool msgAction(const ActionMessage &msg) override;
	bool msgKeypress(const KeypressMessage &msg) override;
};

} // namespace Interactions
} // namespace Views
} // namespace Ultima1
} // namespace Ultima

#endif
