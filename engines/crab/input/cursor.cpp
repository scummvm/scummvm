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

//=============================================================================
// Author:   Arvind
// Purpose:  Cursor class
//=============================================================================
#define FORBIDDEN_SYMBOL_ALLOW_ALL
#include "crab/input/cursor.h"
#include "crab/crab.h"

namespace Crab {
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Reset all values
//------------------------------------------------------------------------
void Cursor::Reset() {
	motion.x = 0;
	motion.y = 0;

	button.x = 0;
	button.y = 0;

	rel.x = 0;
	rel.y = 0;

	// set to -1, so its set to 0 on first update
	state = -1;
}

//------------------------------------------------------------------------
// Purpose: Reset all values
//------------------------------------------------------------------------
void Cursor::Load(rapidxml::xml_node<char> *node) {
	if (NodeValid("normal", node)) {
		rapidxml::xml_node<char> *nornode = node->first_node("normal");
		img.Load(nornode, "img");
		img_s.Load(nornode, "img_s");
	}

	if (NodeValid("hover", node)) {
		rapidxml::xml_node<char> *hovnode = node->first_node("hover");
		img_hover.Load(hovnode, "img");
		img_hover_s.Load(hovnode, "img_s");

		if (NodeValid("offset", hovnode))
			hover_offset.Load(hovnode->first_node("offset"));
	}
}

//------------------------------------------------------------------------
// Purpose: Handle Events
//------------------------------------------------------------------------
void Cursor::HandleEvents(const Common::Event &event) {
	g_engine->_mouse->hover = false;
	byte oldState = state;

#if 0
	if (event.type == SDL_MOUSEMOTION) {
		motion.x = Event.motion.x;
		motion.y = Event.motion.y;

		rel.x = Event.motion.xrel;
		rel.y = Event.motion.yrel;
	} else if (Event.type == SDL_MOUSEBUTTONDOWN) {
		pressed = true;
		button.x = Event.button.x;
		button.y = Event.button.y;
	} else if (Event.type == SDL_MOUSEBUTTONUP) {
		pressed = false;
		button.x = Event.button.x;
		button.y = Event.button.y;
	}
#endif

	if (event.type == Common::EVENT_MOUSEMOVE) {
		motion.x = event.mouse.x;
		motion.y = event.mouse.y;

		rel.x = event.relMouse.x;
		rel.y = event.relMouse.y;
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		pressed = true;

		button.x = event.mouse.x;
		button.y = event.mouse.y;
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		pressed = false;

		button.x = event.mouse.x;
		button.y = event.mouse.y;
	}

	state = (hover ? 1 : 0) | (pressed << 1);

	if (state != oldState) {
		if (hover) {
			if (pressed)
				CursorMan.replaceCursor(img_hover_s.texture->rawSurface(), 0, 0, 0);
			else
				CursorMan.replaceCursor(img_hover.texture->rawSurface(), 0, 0, 0);
		} else {
			if (pressed)
				CursorMan.replaceCursor(img_s.texture->rawSurface(), 0, 0, 0);
			else
				CursorMan.replaceCursor(img.texture->rawSurface(), 0, 0, 0);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void Cursor::Draw() {
/*
	//warning("Drawing cursor: %d %d", motion.x, motion.y);
	if (hover) {
		if (pressed)
			img_hover_s.Draw(motion.x + hover_offset.x, motion.y + hover_offset.y);
		else
			img_hover.Draw(motion.x + hover_offset.x, motion.y + hover_offset.y);
	} else {
		if (pressed)
			img_s.Draw(motion.x, motion.y);
		else
			img.Draw(motion.x, motion.y);
	}
*/
}

} // End of namespace Crab
