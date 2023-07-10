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

#include "crab/animation/animframe.h"

namespace Crab {

using namespace pyrodactyl::anim;

void AnimFrame::load(rapidxml::xml_node<char> *node, const Rect &VBOX, const uint32 &rep, const int &AX, const int &AY) {
	_clip.load(node);

	if (rep == 0)
		loadNum(_repeat, "repeat", node);
	else
		_repeat = rep;

	if (AX == 0.0f && AY == 0.0f) {
		if (nodeValid("anchor", node, false))
			_anchor.load(node->first_node("anchor"));
	} else {
		_anchor.x = AX;
		_anchor.y = AY;
	}

	if (VBOX.w == 0 || VBOX.h == 0) {
		if (nodeValid("box_v", node))
			_boxV.load(node->first_node("box_v"));
	} else
		_boxV = VBOX;
}

void AnimationFrames::load(rapidxml::xml_node<char> *node) {
	loadTextureFlipType(_flip, node);

	if (!loadNum(_repeat, "repeat", node, false))
		_repeat = 0;

	loadBool(_random, "random", node, false);

	if (nodeValid("anchor", node, false))
		_anchor.load(node->first_node("anchor"));

	if (nodeValid("box_v", node))
		_boxV.load(node->first_node("box_v"));

	if (nodeValid("shadow", node)) {
		_shadow.load(node->first_node("shadow"));
		_shadow._valid = true;
	}

	if (nodeValid("frames", node)) {
		_frame.clear();
		rapidxml::xml_node<char> *framenode = node->first_node("frames");
		for (auto n = framenode->first_node("frame"); n != NULL; n = n->next_sibling("frame")) {
			AnimFrame af;
			af.load(n, _boxV, _repeat, _anchor.x, _anchor.y);
			_frame.push_back(af);
		}
	}

	if (random)
		_currentClip = g_engine->getRandomNumber(_frame.size() - 1);
	else
		_currentClip = 0;
}

bool AnimationFrames::updateClip() {
	if (_currentClip < _frame.size()) {
		_currentClip = (_currentClip + 1) % _frame.size();
		return true;
	} else
		_currentClip = 0;

	return false;
}

const AnimFrame &AnimationFrames::currentFrame() {
	return _frame[_currentClip];
}

} // End of namespace Crab
