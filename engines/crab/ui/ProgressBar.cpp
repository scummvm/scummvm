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

void ProgressBar::Load(rapidxml::xml_node<char> *node) {
	ClipButton::Load(node);
	LoadNum(notify_rate, "notify", node);

	if (NodeValid("effect", node)) {
		rapidxml::xml_node<char> *effnode = node->first_node("effect");
		LoadImgKey(inc, "inc", effnode);
		LoadImgKey(dec, "dec", effnode);
		offset.Load(effnode);
	}

	if (NodeValid("desc", node)) {
		rapidxml::xml_node<char> *descnode = node->first_node("desc");
		for (rapidxml::xml_node<char> *n = descnode->first_node("above"); n != NULL; n = n->next_sibling("above"))
			ct.push_back(n);
	}
}

void ProgressBar::Draw(const int &value, const int &max) {
	// We don't want divide by zero errors
	if (max == 0)
		return;

	// Figure out which text to draw as caption
	for (auto &i : ct)
		if (value > i.val) {
			caption.text = i.text;
			break;
		}

	// If we don't have to draw animations for changing value, just draw the bar
	if (!changed) {
		clip.w = (g_engine->_imageManager->GetTexture(img.normal).W() * value) / max;
		ClipButton::Draw();
	} else {
		clip.w = (g_engine->_imageManager->GetTexture(img.normal).W() * cur) / max;
		ClipButton::Draw();

		switch (type) {
		case INCREASE:
			g_engine->_imageManager->Draw(x + clip.w + offset.x, y + offset.y, inc);
			if (timer.TargetReached()) {
				cur++;
				timer.Start();
			}
			break;
		case DECREASE:
			g_engine->_imageManager->Draw(x + clip.w + offset.x, y + offset.y, dec);
			if (timer.TargetReached()) {
				cur--;
				timer.Start();
			}
			break;
		default:
			break;
		}

		if (cur == value)
			changed = false;
	}
}

void ProgressBar::Effect(const int &value, const int &prev) {
	old = prev;
	cur = prev;

	if (value > prev) {
		changed = true;
		type = INCREASE;
		timer.Target(notify_rate * (value - prev));
	} else if (value < prev) {
		changed = true;
		type = DECREASE;
		timer.Target(notify_rate * (prev - value));
	} else {
		changed = false;
		type = NONE;
	}
}

} // End of namespace Crab
