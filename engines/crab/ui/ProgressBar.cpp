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

#include "crab/ui/ProgressBar.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;

void ProgressBar::load(rapidxml::xml_node<char> *node) {
	ClipButton::load(node);
	loadNum(_notifyRate, "notify", node);

	if (nodeValid("effect", node)) {
		rapidxml::xml_node<char> *effnode = node->first_node("effect");
		loadImgKey(_inc, "inc", effnode);
		loadImgKey(_dec, "dec", effnode);
		_offset.load(effnode);
	}

	if (nodeValid("desc", node)) {
		rapidxml::xml_node<char> *descnode = node->first_node("desc");
		for (rapidxml::xml_node<char> *n = descnode->first_node("above"); n != nullptr; n = n->next_sibling("above"))
			_ct.push_back(n);
	}
}

void ProgressBar::draw(const int &value, const int &max) {
	// We don't want divide by zero errors
	if (max == 0)
		return;

	// Figure out which text to draw as caption
	for (auto &i : _ct)
		if (value > i._val) {
			_caption._text = i._text;
			break;
		}

	// If we don't have to draw animations for changing value, just draw the bar
	if (!_changed) {
		_clip.w = (g_engine->_imageManager->getTexture(_img._normal).w() * value) / max;
		ClipButton::draw();
	} else {
		_clip.w = (g_engine->_imageManager->getTexture(_img._normal).w() * _cur) / max;
		ClipButton::draw();

		switch (_type) {
		case INCREASE:
			g_engine->_imageManager->draw(x + _clip.w + _offset.x, y + _offset.y, _inc);
			if (_timer.targetReached()) {
				_cur++;
				_timer.start();
			}
			break;
		case DECREASE:
			g_engine->_imageManager->draw(x + _clip.w + _offset.x, y + _offset.y, _dec);
			if (_timer.targetReached()) {
				_cur--;
				_timer.start();
			}
			break;
		default:
			break;
		}

		if (_cur == value)
			_changed = false;
	}
}

void ProgressBar::effect(const int &value, const int &prev) {
	_old = prev;
	_cur = prev;

	if (value > prev) {
		_changed = true;
		_type = INCREASE;
		_timer.target(_notifyRate * (value - prev));
	} else if (value < prev) {
		_changed = true;
		_type = DECREASE;
		_timer.target(_notifyRate * (prev - value));
	} else {
		_changed = false;
		_type = NONE;
	}
}

} // End of namespace Crab
