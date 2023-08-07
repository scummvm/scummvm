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
#include "crab/input/cursor.h"
#include "crab/ui/slider.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void Slider::load(rapidxml::xml_node<char> *node, const int &min, const int &max, const int &val) {
	if (nodeValid(node)) {
		_knob.load(node->first_node("knob"), false);
		_bar.load(node->first_node("bar"));

		_knob.y = _bar.y;
		_knob.w = g_engine->_imageManager->getTexture(_knob._img._normal).w();
		_knob.h = g_engine->_imageManager->getTexture(_knob._img._normal).h();
		_knob._canmove = true;

		_min = min;
		_max = max;
		_value = val;

		_caption.load(node->first_node("caption"), &_bar);
	}

	createBackup();
}

bool Slider::handleEvents(const Common::Event &Event) {
	// A person is moving the knob
	if (_knob.handleEvents(Event) == BUAC_GRABBED) {
		int dx = g_engine->_mouse->_motion.x - _bar.x;

		if (dx < 0)
			dx = 0;
		else if (dx > (_bar.w - _knob.w))
			dx = (_bar.w - _knob.w);

		_knob.x = _bar.x + dx;
		_knob.y = _bar.y;

		_value = _min + (((_max - _min) * (_knob.x - _bar.x)) / (_bar.w - _knob.w));
		return true;
	}

	// If a person clicks on the slider bar, the knob needs to travel there
	if ((Event.type == Common::EVENT_LBUTTONDOWN || Event.type == Common::EVENT_RBUTTONDOWN) && _bar.contains(g_engine->_mouse->_button.x, g_engine->_mouse->_button.y)) {
		_knob.x = g_engine->_mouse->_button.x;
		_knob.y = _bar.y;

		_value = _min + (((_max - _min) * (_knob.x - _bar.x)) / (_bar.w - _knob.w));
		return true;
	}

	return false;
}


void Slider::draw() {
	_bar.draw();
	_caption.draw(false);
	_knob.draw();
}

void Slider::value(const int val) {
	_value = val;

	if (_value < _min)
		_value = _min;
	else if (_value > _max)
		_value = _max;

	_knob.x = _bar.x + ((_bar.w - _knob.w) * (_value - _min)) / (_max - _min);
}

void Slider::setUI() {
	_bar.setUI();
	_knob.setUI();
	_caption.setUI(&_bar);

	_knob.x = _bar.x + ((_bar.w - _knob.w) * _value / (_max - _min));
	_knob.y = _bar.y;
	_knob.w = g_engine->_imageManager->getTexture(_knob._img._normal).w();
	_knob.h = g_engine->_imageManager->getTexture(_knob._img._normal).h();
}

} // End of namespace Crab
