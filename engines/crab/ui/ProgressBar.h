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
	Timer timer;

	// The total time for which the change effect must be shown
	uint32 notify_rate;

	// Are we currently displaying the effect?
	bool changed;

	// The effect also depends on if the change was positive or negative, so store the previous value
	int old;

	// If we are drawing an animation, we need to smoothly transition from old->value
	// This stores the current progress
	int cur;

	// The type of effect being drawn
	enum { NONE,
		   INCREASE,
		   DECREASE } type;

	// We reuse the button images for the 2 types of effect
	ImageKey inc, dec;

	// Where to draw the effect
	Vector2i offset;

	// The caption text changes depending on the value of the progress bar - we store all possible text here
	struct CaptionText {
		// The text to be drawn
		Common::String text;

		// The above text is drawn only if the progress bar value is greater than this val
		int val;

		CaptionText() { val = 0; }
		CaptionText(rapidxml::xml_node<char> *node) {
			if (!loadNum(val, "val", node))
				val = 0;

			if (!loadStr(text, "text", node))
				text = "";
		}
	};

	Common::Array<CaptionText> ct;

public:
	ProgressBar() {
		old = 0;
		cur = 0;
		inc = 0;
		dec = 0;
		notify_rate = 5;
		Reset();
	}
	~ProgressBar() {}

	// Reset the effect
	void Reset() {
		changed = false;
		type = NONE;
	}
	void load(rapidxml::xml_node<char> *node);

	void Draw(const int &value, const int &max);
	void Effect(const int &value, const int &prev);
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PROGRESSBAR_H
