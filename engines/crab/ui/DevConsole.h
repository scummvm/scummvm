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
	enum { STATE_NORMAL,
		   STATE_VAR } state;

	// The overarching menu that is the starting point for all functions
	ButtonMenu menu;

	// The dialog box UI - used to check value of a variable
	ImageData bg;
	Button check, back;
	TextData value;
	TextArea text_field;

	// The variable name we're tracking
	std::string var_name;

public:
	DebugConsole() { state = STATE_NORMAL; }
	~DebugConsole() {}

	// Only restrict input when we're in variable state
	bool RestrictInput() { return (state == STATE_VAR || menu.HoverIndex() != -1); }

	void Load(const std::string &filename);
	void Draw(pyrodactyl::event::Info &info);

#if 0
	void HandleEvents(const SDL_Event &Event);
#endif
	void InternalEvents();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_DEVCONSOLE_H
