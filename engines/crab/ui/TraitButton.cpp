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
#include "crab/ui/TraitButton.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::people;

void TraitButton::init(const TraitButton &ref, const int &xOffset, const int &yOffset) {
	StateButton::init(ref, xOffset, yOffset);
	_offset = ref._offset;
}

void TraitButton::load(rapidxml::xml_node<char> *node, const bool &echo) {
	StateButton::load(node, echo);

	if (nodeValid("offset", node))
		_offset.load(node->first_node("offset"), echo);
}

void TraitButton::draw(const int &xOffset, const int &yOffset, Rect *clip) {
	if (_traitImg != 0)
		g_engine->_imageManager->draw(x + _offset.x, y + _offset.y, _traitImg);

	StateButton::draw(xOffset, yOffset, clip);
}

void TraitButton::cache(const pyrodactyl::people::Trait &trait) {
	_traitImg = trait._img;
	_caption._text = trait._name;
}

void TraitButton::empty() {
	_traitImg = 0;
	_caption._text = "";
}

} // End of namespace Crab
