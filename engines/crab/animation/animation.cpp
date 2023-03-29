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

#include "crab/animation.h"

namespace Crab {

using namespace pyrodactyl::image;
using namespace pyrodactyl::anim;

Animation::Animation(rapidxml::xml_node<char> *node) {
	LoadNum(length, "length", node);
	for (auto n = node->first_node("frame"); n != NULL; n = n->next_sibling("frame"))
		frame.push_back(n);
}

void Animation::Draw() {
	Uint32 timestamp = timer.Ticks();
	for (auto i = frame.begin(); i != frame.end(); ++i)
		i->Draw(timestamp);
}

bool Animation::InternalEvents(DrawType &game_draw) {
	Uint32 timestamp = timer.Ticks();
	for (auto i = frame.begin(); i != frame.end(); ++i) {
		DrawType result = i->InternalEvents(timestamp);
		// if (result != DRAW_SAME)
		game_draw = result;
	}

	return timer.Ticks() >= length;
}

void Animation::Reset() {
	for (auto i = frame.begin(); i != frame.end(); ++i)
		i->Reset();
}

} // End of namespace Crab
