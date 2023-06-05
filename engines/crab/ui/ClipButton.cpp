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
#include "crab/ui/ClipButton.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ClipButton::Load(rapidxml::xml_node<char> *node, const bool &echo) {
	Button::Load(node, echo);

	if (NodeValid("clip", node, false))
		clip.Load(node->first_node("clip"));
	else {
		clip.x = 0;
		clip.y = 0;
		clip.w = g_engine->_imageManager->GetTexture(img.normal).W();
		clip.h = g_engine->_imageManager->GetTexture(img.normal).H();
	}
}

void ClipButton::Draw(const int &XOffset, const int &YOffset) {
	Button::Draw(XOffset, YOffset, &clip);
}

} // End of namespace Crab
