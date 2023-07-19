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

#include "crab/crab.h"
#include "crab/ui/QuestText.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void QuestText::load(rapidxml::xml_node<char> *node) {
	ParagraphData::load(node);
	loadNum(_colS, "color_s", node);

	if (nodeValid("line", node))
		loadNum(_linesPerPage, "page", node->first_node("line"));

	if (nodeValid("inc", node))
		_inc.load(node->first_node("inc"));

	if (nodeValid("img", node))
		_img.load(node->first_node("img"));

	if (nodeValid("prev", node)) {
		_prev.load(node->first_node("prev"));
		_prev._hotkey.set(IU_PAGE_PREV);
	}

	if (nodeValid("next", node)) {
		_next.load(node->first_node("next"));
		_next._hotkey.set(IU_PAGE_NEXT);
	}

	if (nodeValid("status", node))
		_status.load(node->first_node("status"));
}

void QuestText::draw(pyrodactyl::event::Quest &q) {
	// First, we must scan and find the part of the quest text we should draw

	// Assign default values to start and stop
	_start = 0;
	_stop = q._text.size();

	// Keep count of lines and pages - remember a single entry can take more than one line
	unsigned int pageCount = 0, pageStart = 0;

	// Start from line 0, page 0 and scan the list of entries
	for (unsigned int i = 0, lineCount = 0; i < q._text.size(); ++i) {
		// Increment the number of lines by one text entry
		lineCount += (q._text[i].size() / _line.x) + 1;

		// If we go over the quota for lines per page, go to next page and reset line counter to 0
		if (lineCount > _linesPerPage) {
			// We are about to go to next page, stop at this entry
			if (pageCount == _currentPage) {
				_start = pageStart;
				_stop = i;
			}

			pageCount++;
			lineCount = 0;

			// This is the start of the next page
			pageStart = i;
		}
	}

	// Used for the final page, because the page count won't be incremented for the last one
	if (pageCount == _currentPage) {
		_start = pageStart;
		_stop = q._text.size();
	}

	// Find out how many pages the lines need
	_totalPage = pageCount + 1;

	// Update the text
	_status._text = (NumberToString(_currentPage + 1) + " of " + NumberToString(_totalPage));

	// Now, start drawing the quest
	_status.draw();

	if (_currentPage > 0)
		_prev.draw();

	if (_currentPage < _totalPage - 1)
		_next.draw();

	// Draw the current page of quest text
	if (!q._text.empty()) {
		// Count the number of lines, because a single entry can take more than one line
		int count = 0;

		for (unsigned int i = _start; i < (unsigned int)_stop; ++i) {
			_img.draw(_inc.x * count, _inc.y * count);

			// Draw first entry in selected color, and older quest entries in standard color
			if (i == 0)
				g_engine->_textManager->draw(x, y, q._text[i], _colS, _font, _align, _line.x, _line.y);
			else
				ParagraphData::draw(q._text[i], _inc.x * count, _inc.y * count);

			// Count is reduced extra by the amount of lines it takes for the message to be drawn
			count += (q._text[i].size() / _line.x) + 1;
		}
	}
}

void QuestText::handleEvents(pyrodactyl::event::Quest &q, const Common::Event &event) {
	if (_currentPage > 0 && _prev.handleEvents(event) == BUAC_LCLICK)
		_currentPage--;

	if (_currentPage < _totalPage - 1 && _next.handleEvents(event) == BUAC_LCLICK) {
		_currentPage++;

		if (_currentPage >= _totalPage)
			_currentPage = _totalPage - 1;
	}
}

#if 0
void QuestText::handleEvents(pyrodactyl::event::Quest &q, const SDL_Event &Event) {
	if (current_page > 0 && prev.handleEvents(Event) == BUAC_LCLICK) {
		current_page--;

		if (current_page < 0)
			current_page = 0;
	}

	if (current_page < total_page - 1 && next.handleEvents(Event) == BUAC_LCLICK) {
		current_page++;

		if (current_page >= total_page)
			current_page = total_page - 1;
	}
}
#endif

void QuestText::setUI() {
	ParagraphData::setUI();
	_img.setUI();
	_prev.setUI();
	_next.setUI();
	_status.setUI();
}

} // End of namespace Crab
