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

#ifndef CRAB_PAGEMENU_H
#define CRAB_PAGEMENU_H

#include "crab/common_header.h"
#include "crab/ui/menu.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
// Sometimes we need multiple pages in a menu, this object does that
// Used in save, load, mod and quest menu
template<typename T>
class PageMenu {
	// The buttons for cycling between pages of the menu
	Button _prev, _next;

	// Each page is stored separately in a menu object
	Common::Array<Menu<T>> _menu;

	// Keep track of which page we are at, and how many elements we keep in a page
	uint _currentPage, _elementsPerPage, _rows, _cols;

	// The image used for the elements
	Button _ref;

	// This vector stores the increments in x,y for each new button
	Vector2i _inc;

	// Display "Page 1 of 3" style information for the menu
	TextData _status;
	Common::String _info;

public:
	PageMenu() {
		_currentPage = 0;
		_elementsPerPage = 1;
		_rows = 1;
		_cols = 1;
		clear();
	}
	~PageMenu() {}

	void reset() {
		for (auto &m : _menu)
			m.reset();
	}

	void clear() {
		_menu.resize(1);
		_menu[0].clear();
		_menu[0].useKeyboard(true);
	}

	// Get the elements per page
	uint elementsPerPage() {
		return _elementsPerPage;
	}

	// This is added to the result from handleEvents to calculate the exact position
	uint index() {
		return _currentPage * _elementsPerPage;
	}

	// The end position of the elements
	uint indexPlusOne() {
		return (_currentPage + 1) * _elementsPerPage;
	}

	// Get the current page of the menu
	uint currentPage() {
		return _currentPage;
	}

	void currentPage(int &val) {
		_currentPage = val;
	}

	// Get the index of the hovered element in the menu
	int hoverIndex() {
		if (_menu[_currentPage].hoverIndex() >= 0)
			return (_currentPage * _elementsPerPage) + _menu[_currentPage].hoverIndex();

		return -1;
	}

	// Get the base position of the elements
	int baseX(const int &count) {
		return _ref.x + _inc.x * (count % _cols);
	}

	int baseY(const int &count) {
		return _ref.y + _inc.y * (count / _cols);
	}

	// This is used to get the coordinates of a button
	const int &curX(const int &count) {
		return _menu[_currentPage]._element[count].x;
	}

	const int &curY(const int &count) {
		return _menu[_currentPage]._element[count].y;
	}

	void image(const int &slot, const int &page, ButtonImage &bi) {
		_menu[page]._element[slot].img(bi);
	}

	void assignPaths() {
		for (auto &m : _menu)
			m.assignPaths();
	}

	void useKeyboard(const bool &val) {
		for (auto &m : _menu)
			m.useKeyboard(val);
	}

	void setUI() {
		_prev.setUI();
		_next.setUI();
		_ref.setUI();
		_status.setUI();

		for (auto &m : _menu)
			m.setUI();
	}

	void updateInfo() {
		_info = numberToString(_currentPage + 1);
		_info += " of ";
		_info += numberToString(_menu.size());
	}

	void load(rapidxml::xml_node<char> *node) {
		using namespace pyrodactyl::input;
		if (nodeValid(node)) {
			if (nodeValid("prev", node)) {
				_prev.load(node->first_node("prev"));
				_prev._hotkey.set(IU_PREV);
			}

			if (nodeValid("next", node)) {
				_next.load(node->first_node("next"));
				_next._hotkey.set(IU_NEXT);
			}

			if (nodeValid("reference", node))
				_ref.load(node->first_node("reference"));

			if (nodeValid("inc", node))
				_inc.load(node->first_node("inc"));

			if (nodeValid("status", node))
				_status.load(node->first_node("status"));

			if (nodeValid("dim", node)) {
				rapidxml::xml_node<char> *dimnode = node->first_node("dim");
				loadNum(_rows, "rows", dimnode);
				loadNum(_cols, "cols", dimnode);
				_elementsPerPage = _rows * _cols;
			}
		}
	}

	void add(uint &slot, uint &page) {
		if (slot >= _elementsPerPage) {
			++page;
			slot = 0;
			_menu.resize(page + 1);
			_menu[page].useKeyboard(true);
		}

		T b;
		b.init(_ref, _inc.x * (slot % _cols), _inc.y * (slot / _cols));
		_menu[page]._element.push_back(b);
		++slot;

		assignPaths();
		updateInfo();
	}

	void add() {
		uint page = _menu.size() - 1;
		uint slot = _menu[page]._element.size();
		add(slot, page);
	}

	void erase() {
		uint page = _menu.size() - 1;
		_menu[page]._element.pop_back();
		assignPaths();
		updateInfo();
	}

	int handleEvents(const Common::Event &event) {
		using namespace pyrodactyl::input;

		if (_currentPage > 0 && _prev.handleEvents(event) == BUAC_LCLICK) {
			_currentPage--;
			updateInfo();

			if ((int)_currentPage < 0)
				_currentPage = 0;
		}

		if (_currentPage < _menu.size() - 1 && _next.handleEvents(event) == BUAC_LCLICK) {
			_currentPage++;
			updateInfo();

			if (_currentPage >= _menu.size())
				_currentPage = _menu.size() - 1;
		}

		return _menu[_currentPage].handleEvents(event);
	}

	void draw() {
		_status.draw(_info);
		_menu[_currentPage].draw();

		if (_currentPage > 0)
			_prev.draw();

		if (_currentPage < _menu.size() - 1)
			_next.draw();
	}
};

typedef PageMenu<Button> PageButtonMenu;
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_PAGEMENU_H
