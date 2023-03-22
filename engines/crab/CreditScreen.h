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

#include "crab/ImageData.h"
#include "crab/TextData.h"
#include "crab/TextManager.h"
#include "crab/button.h"
#include "crab/common_header.h"
#include "crab/timer.h"

namespace pyrodactyl {
namespace ui {
class CreditScreen {
	// Text and formatting information
	struct CreditText {
		// Title of the section (stuff like programmer, designer etc)
		std::string text;

		// The style it should be drawn in
		bool heading;
	};

	// The background image and company logo
	pyrodactyl::ui::ImageData bg, logo;

	// The names displayed in the credits
	std::vector<CreditText> list;

	// The starting position
	Vector2i start;

	// The current position
	Vector2D<long> cur;

	// Text parameters
	struct TextParam {
		int inc, color;
		FontKey font;
		Align align;

		TextParam() {
			inc = 30;
			color = 0;
			font = 1;
			align = ALIGN_CENTER;
		}

		void Load(rapidxml::xml_node<char> *node) {
			LoadNum(inc, "inc", node);
			LoadNum(color, "color", node);
			LoadFontKey(font, "font", node);
			LoadAlign(align, node);
		}
	} heading, paragraph;

	// All speed levels at which credits can scroll through
	struct ScrollSpeed {
		// The current speed
		float cur;

		// Various levels
		float slow, fast, reverse;

		ScrollSpeed() {
			slow = 1.0f;
			fast = 4.0f;
			reverse = -2.0f;
			cur = slow;
		}
	} speed;

	// Speed controls for credits
	Button fast, slow, reverse, pause;

	// The back button, website and twitter buttons
	Button back, website, twitter;

public:
	CreditScreen() { Reset(); }
	~CreditScreen() {}

	void Load(const std::string &filename);
	void Reset();

	bool HandleEvents(SDL_Event &Event);
	void Draw();

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_CREDITSCREEN_H
