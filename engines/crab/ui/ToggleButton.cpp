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

#include "crab/crab.h"
#include "crab/ui/ToggleButton.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ToggleButton::load(rapidxml::xml_node<char> *node) {
	Button::load(node);
	loadImgKey(_on, "on", node);
	loadImgKey(_off, "off", node);

	if (nodeValid("offset", node))
		_offset.load(node->first_node("offset"));
}

void ToggleButton::draw(const int &xOffset, const int &yOffset, Rect *clip) {
	Button::draw(xOffset, yOffset, clip);

	if (_state)
		g_engine->_imageManager->draw(x + _offset.x, y + _offset.y, _on);
	else
		g_engine->_imageManager->draw(x + _offset.x, y + _offset.y, _off);
}

ButtonAction ToggleButton::handleEvents(const Common::Event &event, const int &xOffset, const int &yOffset) {
	ButtonAction action = Button::handleEvents(event, xOffset, yOffset);

	if (action == BUAC_LCLICK)
		_state = !_state;

	return action;
}

} // End of namespace Crab
