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

#ifndef CRAB_SLIDER_H
#define CRAB_SLIDER_H

#include "crab/ui/Caption.h"
#include "crab/ui/ImageData.h"
#include "crab/ui/button.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class Slider {
	// The value of the slider and the backup
	int _value, _backup;

	// The slider bar position and dimensions
	ImageData _bar;

	// The slider knob
	Button _knob;

	// Caption for the slider
	Caption _caption;

	// The maximum and minimum values for the slider
	int _max, _min;

public:
	Slider() {
		_max = 100;
		_min = 0;
		_value = ((_max - _min) / 2);
		_backup = _value;
	}

	~Slider() {}

	void load(rapidxml::xml_node<char> *node, const int &min, const int &max, const int &val);

	bool handleEvents(const Common::Event &event);

	void draw();

	int Value() { return _value; }
	void value(const int val);

	void createBackup() {
		_backup = _value;
	}

	void restoreBackup() {
		_value = _backup;
	}

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_SLIDER_H
