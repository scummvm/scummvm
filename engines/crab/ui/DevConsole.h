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

/*
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_DEVCONSOLE_H
#define CRAB_DEVCONSOLE_H

#include "crab/event/GameEventInfo.h"
#include "crab/common_header.h"
#include "crab/ui/textarea.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class DebugConsole {
	// The state of the menu
	enum {
		STATE_NORMAL,
		STATE_VAR
	} _state;

	// The overarching menu that is the starting point for all functions
	ButtonMenu _menu;

	// The dialog box UI - used to check value of a variable
	ImageData _bg;
	Button _check, _back;
	TextData _value;
	TextArea _textField;

	// The variable name we're tracking
	Common::String _varName;

public:
	DebugConsole() {
		_state = STATE_NORMAL;
	}

	~DebugConsole() {}

	// Only restrict input when we're in variable state
	bool restrictInput() {
		return (_state == STATE_VAR || _menu.HoverIndex() != -1);
	}

	void load(const Common::String &filename);
	void draw(pyrodactyl::event::Info &info);

#if 0
	void handleEvents(const SDL_Event &Event);
#endif
	void internalEvents();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_DEVCONSOLE_H
