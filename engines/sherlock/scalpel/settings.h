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

#ifndef SHERLOCK_SETTINGS_H
#define SHERLOCK_SETTINGS_H

#include "common/scummsys.h"

namespace Sherlock {

class SherlockEngine;

namespace Scalpel {

class Settings {
private:
	SherlockEngine *_vm;

	Settings(SherlockEngine *vm) : _vm(vm) {}

	/**
	 * Draws the interface for the settings window
	 */
	void drawInteface(bool flag);

	/**
	 * Draws the buttons for the settings dialog
	 */
	int drawButtons(const Common::Point &pt, int key);
public:
	/**
	 * Handles input when the settings window is being shown
	 * @remarks		Whilst this would in theory be better in the Journal class, since it displays in
	 *		the user interface, it uses so many internal UI fields, that it sort of made some sense
	 *		to put it in the UserInterface class.
	 */
	static void show(SherlockEngine *vm);
};

} // End of namespace Scalpel

} // End of namespace Sherlock

#endif
