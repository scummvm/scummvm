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

#ifndef CRAB_CREDITSCREEN_H
#define CRAB_CREDITSCREEN_H

#include "crab/ui/ImageData.h"
#include "crab/ui/TextData.h"
#include "crab/text/TextManager.h"
#include "crab/ui/button.h"
#include "crab/timer.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class CreditScreen {
	// Text and formatting information
	struct CreditText {
		// Title of the section (stuff like programmer, designer etc)
		Common::String _text;

		// The style it should be drawn in
		bool _heading;
	};

	// The background image and company logo
	pyrodactyl::ui::ImageData _bg, _logo;

	// The names displayed in the credits
	Common::Array<CreditText> _list;

	// The starting position
	Vector2i _start;

	// The current position
	Vector2D<long> _cur;

	// Text parameters
	struct TextParam {
		int _inc, _color;
		FontKey _font;
		Align _align;

		TextParam() {
			_inc = 30;
			_color = 0;
			_font = 1;
			_align = ALIGN_CENTER;
		}

		void load(rapidxml::xml_node<char> *node) {
			loadNum(_inc, "inc", node);
			loadNum(_color, "color", node);
			LoadFontKey(_font, "font", node);
			loadAlign(_align, node);
		}
	} _heading, _paragraph;

	// All speed levels at which credits can scroll through
	struct ScrollSpeed {
		// The current speed
		float _cur;

		// Various levels
		float _slow, _fast, _reverse;

		ScrollSpeed() {
			_slow = 1.0f;
			_fast = 4.0f;
			_reverse = -2.0f;
			_cur = _slow;
		}
	} _speed;

	// Speed controls for credits
	Button _fast, _slow, _reverse, _pause;

	// The back button, website and twitter buttons
	Button _back, _website, _twitter;

public:
	CreditScreen() {
		reset();
	}

	~CreditScreen() {}

	void load(const Common::String &filename);
	void reset();

	bool handleEvents(Common::Event &event);

	void draw();

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_CREDITSCREEN_H
