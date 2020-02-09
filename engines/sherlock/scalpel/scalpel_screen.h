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

#ifndef SHERLOCK_SCALPEL_SCREEN_H
#define SHERLOCK_SCALPEL_SCREEN_H

#include "sherlock/screen.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

class ScalpelScreen : public Screen {
public:
	ScalpelScreen(SherlockEngine *vm);
	~ScalpelScreen() override {}

	/**
	 * Draws a button for use in the inventory, talk, and examine dialogs.
	 * ButtonText is supposed to have its hotkey as a prefix. The hotkey will get highlighted.
	 */
	void makeButton(const Common::Rect &bounds, int textX, const Common::String &buttonText, bool textContainsHotkey = true);

	/**
	 * Prints an interface command with the first letter highlighted to indicate
	 * what keyboard shortcut is associated with it
	 * ButtonText is supposed to have its hotkey as a prefix. The hotkey will get highlighted.
	 */
	void buttonPrint(const Common::Point &pt, uint color, bool slamIt, const Common::String &buttonText, bool textContainsHotkey = true);

	/**
	 * Draw a panel in the back buffer with a raised area effect around the edges
	 */
	void makePanel(const Common::Rect &r);

	/**
	 * Draw a field in the back buffer with a raised area effect around the edges,
	 * suitable for text input.
	 */
	void makeField(const Common::Rect &r);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
