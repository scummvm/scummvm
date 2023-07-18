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

#include "crab/ui/emotion.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::event;

void EmotionIndicator::load(rapidxml::xml_node<char> *node) {
	if (nodeValid("text", node))
		_text.load(node->first_node("text"));
}

void EmotionIndicator::draw(const int &select) {
	if (select >= 0 && (unsigned int)select < _value.size())
		if (_value[select] < g_engine->_eventStore->_tone.size()) {
			_text.draw(g_engine->_eventStore->_tone[_value[select]]._text);
		}
}

void EmotionIndicator::setUI() {
	_text.setUI();
}

} // End of namespace Crab
