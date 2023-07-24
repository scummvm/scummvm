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

#include "crab/ui/ImageData.h"
#include "crab/ui/ParagraphData.h"
#include "crab/ui/button.h"
#include "crab/common_header.h"
#include "crab/event/quest.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class QuestText : public ParagraphData {
protected:
	// How much the text and bullet positions change per line
	Vector2i _inc;

	// Color of the highlighted quest
	int _colS;

	// The coordinates for drawing image, which is like bullet points in the form of <Bullet> <Text>
	ImageData _img;

	// The lines per page, we split the quest text into multiple pages if we have to draw more than that
	unsigned int _linesPerPage;

	// Keep track of which page we are at, and total pages
	unsigned int _currentPage, _totalPage;

	// The quest entries we start and stop the drawing at
	int _start, _stop;

	// The buttons for cycling between pages of the menu
	Button _prev, _next;

	// Display "Page 1 of 3" style information for the menu
	HoverInfo _status;

public:
	QuestText() {
		_colS = 0;
		_currentPage = 0;
		_start = 0;
		_stop = 0;
		_totalPage = 1;
		_linesPerPage = 10;
	}

	void load(rapidxml::xml_node<char> *node);

	// Reset the value of current page
	void reset() {
		_currentPage = 0;
	}

	void handleEvents(pyrodactyl::event::Quest &q, const Common::Event &event);

	void draw(pyrodactyl::event::Quest &q);

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_QUESTTEXT_H
