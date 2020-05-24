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

#ifndef ULTIMA4_CONTROLLERS_ALPHA_ACTION_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_ALPHA_ACTION_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/events/event_handler.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A controller to handle input for commands requiring a letter
 * argument in the range 'a' - lastValidLetter.
 */
class AlphaActionController : public WaitableController<int> {
private:
	char _lastValidLetter;
	Common::String _prompt;
public:
	AlphaActionController(char letter, const Common::String &p) :
		WaitableController<int>(-1), _lastValidLetter(letter), _prompt(p) {}

	bool keyPressed(int key) override;
	void keybinder(KeybindingAction action) override;

	static int get(char lastValidLetter, const Common::String &prompt, EventHandler *eh = nullptr);
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
