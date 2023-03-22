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

#ifndef CRAB_QUESTTEXT_H
#define CRAB_QUESTTEXT_H

#include "ImageData.h"
#include "ParagraphData.h"
#include "button.h"
#include "common_header.h"
#include "quest.h"

namespace pyrodactyl {
namespace ui {
class QuestText : public ParagraphData {
protected:
	// How much the text and bullet positions change per line
	Vector2i inc;

	// Color of the highlighted quest
	int col_s;

	// The coordinates for drawing image, which is like bullet points in the form of <Bullet> <Text>
	ImageData img;

	// The lines per page, we split the quest text into multiple pages if we have to draw more than that
	unsigned int lines_per_page;

	// Keep track of which page we are at, and total pages
	unsigned int current_page, total_page;

	// The quest entries we start and stop the drawing at
	int start, stop;

	// The buttons for cycling between pages of the menu
	Button prev, next;

	// Display "Page 1 of 3" style information for the menu
	HoverInfo status;

public:
	QuestText() {
		col_s = 0;
		current_page = 0;
		start = 0;
		stop = 0;
		total_page = 1;
		lines_per_page = 10;
	}

	void Load(rapidxml::xml_node<char> *node);

	// Reset the value of current page
	void Reset() { current_page = 0; }

	void HandleEvents(pyrodactyl::event::Quest &q, const SDL_Event &Event);
	void Draw(pyrodactyl::event::Quest &q);

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_QUESTTEXT_H
