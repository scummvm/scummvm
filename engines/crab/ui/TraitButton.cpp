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

void TraitButton::Init(const TraitButton &ref, const int &XOffset, const int &YOffset) {
	StateButton::Init(ref, XOffset, YOffset);
	offset = ref.offset;
}

void TraitButton::load(rapidxml::xml_node<char> *node, const bool &echo) {
	StateButton::load(node, echo);

	if (nodeValid("offset", node))
		offset.load(node->first_node("offset"), echo);
}

void TraitButton::draw(const int &XOffset, const int &YOffset, Rect *clip) {
	if (trait_img != 0)
		g_engine->_imageManager->draw(x + offset.x, y + offset.y, trait_img);

	StateButton::draw(XOffset, YOffset, clip);
}

void TraitButton::Cache(const pyrodactyl::people::Trait &trait) {
	trait_img = trait.img;
	caption.text = trait.name;
}

void TraitButton::Empty() {
	trait_img = 0;
	caption.text = "";
}

} // End of namespace Crab
