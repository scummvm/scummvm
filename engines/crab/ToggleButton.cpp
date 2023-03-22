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

#include "ToggleButton.h"
#include "stdafx.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ToggleButton::Load(rapidxml::xml_node<char> *node) {
	Button::Load(node);
	LoadImgKey(on, "on", node);
	LoadImgKey(off, "off", node);

	if (NodeValid("offset", node))
		offset.Load(node->first_node("offset"));
}

void ToggleButton::Draw(const int &XOffset, const int &YOffset, Rect *clip) {
	Button::Draw(XOffset, YOffset, clip);

	if (state)
		gImageManager.Draw(x + offset.x, y + offset.y, on);
	else
		gImageManager.Draw(x + offset.x, y + offset.y, off);
}

ButtonAction ToggleButton::HandleEvents(const SDL_Event &Event, const int &XOffset, const int &YOffset) {
	ButtonAction action = Button::HandleEvents(Event, XOffset, YOffset);

	if (action == BUAC_LCLICK)
		state = !state;

	return action;
}
