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

#include "crab/level/talknotify.h"

namespace Crab {

using namespace pyrodactyl::text;
using namespace pyrodactyl::level;
using namespace pyrodactyl::image;

void TalkNotify::load(rapidxml::xml_node<char> *node) {
	_offset.load(node);
	loadNum(_font, "font", node);
	loadAlign(_align, node);
	loadNum(_col, "col", node);
}

void TalkNotify::draw(pyrodactyl::event::Info &info, pyrodactyl::anim::Sprite &s, const Rect &camera) {
	Rect rect = s.posRect();

	// Find position to draw name (over the sprite's top edge)
	Vector2i pos;
	pos.x = rect.x + rect.w / 2 + _offset.x;
	pos.y = rect.y + _offset.y;

	// Find the sprite name
	Common::String text = info.getName(s.id());

	g_engine->_textManager->draw(pos.x - camera.x, pos.y - camera.y, text, _col, _font, _align, true);
}

} // End of namespace Crab
