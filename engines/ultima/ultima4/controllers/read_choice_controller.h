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

#ifndef ULTIMA4_CONTROLLERS_READ_CHOICE_CONTROLLER_H
#define ULTIMA4_CONTROLLERS_READ_CHOICE_CONTROLLER_H

#include "ultima/ultima4/controllers/controller.h"
#include "ultima/ultima4/events/event_handler.h"

namespace Ultima {
namespace Ultima4 {

/**
 * A controller to read a single key from a provided list.
 */
class ReadChoiceController : public WaitableController<int> {
public:
	ReadChoiceController(const Common::String &choices);
	bool keyPressed(int key) override;
	void keybinder(KeybindingAction action) override;

	static char get(const Common::String &choices, EventHandler *eh = nullptr);

protected:
	Common::String _choices;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
