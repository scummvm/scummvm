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

#include "crab/slider.h"

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

void Slider::Load(rapidxml::xml_node<char> *node, const int &Min, const int &Max, const int &Val) {
	if (NodeValid(node)) {
		knob.Load(node->first_node("knob"), false);
		bar.Load(node->first_node("bar"));

		knob.y = bar.y;
		knob.w = gImageManager.GetTexture(knob.img.normal).W();
		knob.h = gImageManager.GetTexture(knob.img.normal).H();
		knob.canmove = true;

		min = Min;
		max = Max;
		Value(Val);

		caption.Load(node->first_node("caption"), &bar);
	}

	CreateBackup();
}

bool Slider::HandleEvents(const SDL_Event &Event) {
	// A person is moving the knob
	if (knob.HandleEvents(Event) == BUAC_GRABBED) {
		int dx = gMouse.motion.x - bar.x;

		if (dx < 0)
			dx = 0;
		else if (dx > (bar.w - knob.w))
			dx = (bar.w - knob.w);

		knob.x = bar.x + dx;
		knob.y = bar.y;

		value = min + (((max - min) * (knob.x - bar.x)) / (bar.w - knob.w));
		return true;
	}

	// If a person clicks on the slider bar, the knob needs to travel there
	if (Event.type == SDL_MOUSEBUTTONDOWN && bar.Contains(gMouse.button.x, gMouse.button.y)) {
		knob.x = gMouse.button.x;
		knob.y = bar.y;

		value = min + (((max - min) * (knob.x - bar.x)) / (bar.w - knob.w));
		return true;
	}

	return false;
}

void Slider::Draw() {
	bar.Draw();
	caption.Draw(false);
	knob.Draw();
}

void Slider::Value(const int val) {
	value = val;

	if (value < min)
		value = min;
	else if (value > max)
		value = max;

	knob.x = bar.x + ((bar.w - knob.w) * (value - min)) / (max - min);
}

void Slider::SetUI() {
	bar.SetUI();
	knob.SetUI();
	caption.SetUI(&bar);

	knob.x = bar.x + ((bar.w - knob.w) * value / (max - min));
	knob.y = bar.y;
	knob.w = gImageManager.GetTexture(knob.img.normal).W();
	knob.h = gImageManager.GetTexture(knob.img.normal).H();
}
