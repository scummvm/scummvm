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

#pragma once

#include "Caption.h"
#include "ImageData.h"
#include "ImageManager.h"
#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
class Slider {
	// The value of the slider and the backup
	int value, backup;

	// The slider bar position and dimensions
	ImageData bar;

	// The slider knob
	Button knob;

	// Caption for the slider
	Caption caption;

	// The maximum and minimum values for the slider
	int max, min;

public:
	Slider() {
		max = 100;
		min = 0;
		Value((max - min) / 2);
		backup = value;
	}
	~Slider() {}

	void Load(rapidxml::xml_node<char> *node, const int &Min, const int &Max, const int &Val);

	// Return true if volume changed
	bool HandleEvents(const SDL_Event &Event);
	void Draw();

	int Value() { return value; }
	void Value(const int val);

	void CreateBackup() { backup = value; }
	void RestoreBackup() { Value(backup); }

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl
