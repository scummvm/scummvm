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

#ifndef CRAB_PROGRESSBAR_H
#define CRAB_PROGRESSBAR_H

#include "crab/ui/ClipButton.h"
#include "crab/common_header.h"
#include "crab/timer.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class ProgressBar : public ClipButton {
	// Whenever the progress bar value is changed, we display a glowing effect
	Timer _timer;

	// The total time for which the change effect must be shown
	uint32 _notifyRate;

	// Are we currently displaying the effect?
	bool _changed;

	// The effect also depends on if the change was positive or negative, so store the previous value
	int _old;

	// If we are drawing an animation, we need to smoothly transition from old->value
	// This stores the current progress
	int _cur;

	// The type of effect being drawn
	enum {
		NONE,
		INCREASE,
		DECREASE
	} _type;

	// We reuse the button images for the 2 types of effect
	ImageKey _inc, _dec;

	// Where to draw the effect
	Vector2i _offset;

	// The caption text changes depending on the value of the progress bar - we store all possible text here
	struct CaptionText {
		// The text to be drawn
		Common::String _text;

		// The above text is drawn only if the progress bar value is greater than this val
		int _val;

		CaptionText() {
			_val = 0;
		}

		CaptionText(rapidxml::xml_node<char> *node) {
			if (!loadNum(_val, "val", node))
				_val = 0;

			if (!loadStr(_text, "text", node))
				_text = "";
		}
	};

	Common::Array<CaptionText> _ct;

public:
	ProgressBar() {
		_old = 0;
		_cur = 0;
		_inc = 0;
		_dec = 0;
		_notifyRate = 5;
		reset();
	}
	~ProgressBar() {}

	// Reset the effect
	void reset() {
		_changed = false;
		_type = NONE;
	}
	void load(rapidxml::xml_node<char> *node);

	void draw(const int &value, const int &max);
	void effect(const int &value, const int &prev);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PROGRESSBAR_H
