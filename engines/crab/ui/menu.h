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

//=============================================================================
// Author:   Arvind
// Purpose:  Menu class
//=============================================================================
#ifndef CRAB_MENU_H
#define CRAB_MENU_H

#include "crab/crab.h"
#include "crab/metaengine.h"
#include "crab/ui/button.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
template<typename T>
class Menu {
protected:
	// The index of current selected option and highlighted option
	int _hoverIndex;

	// The order in which a keyboard or gamepad traverses the menu
	Common::Array<uint> _path;

	// Are keyboard buttons enabled?
	bool _useKeyboard;

	// Has a key been pressed?
	enum InputDevice {
		KEYBOARD,
		MOUSE
	} _latestInput;

	// Do the paths use horizontal, vertical or both types of input for keyboard traversal
	enum PathType {
		PATH_DEFAULT,
		PATH_HORIZONTAL,
		PATH_VERTICAL
	} _pathType;

	//------------------------------------------------------------------------
	// Purpose: Find the next element in our path
	//------------------------------------------------------------------------
	void next() {
		if (_hoverIndex == -1) {
			for (uint pos = 0; pos < _path.size(); pos++)
				if (_element[_path[pos]]._visible == true) {
					_hoverIndex = _path[pos];
					break;
				}
		} else {
			uint curpos = 0;
			for (; curpos < _path.size(); curpos++)
				if ((int)_path[curpos] == _hoverIndex)
					break;

			for (uint nextloc = (curpos + 1) % _element.size(); nextloc != curpos; nextloc = (nextloc + 1) % _element.size())
				if (_element[nextloc]._visible == true) {
					_hoverIndex = _path[nextloc];
					break;
				}
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Find the previous element in our path
	//------------------------------------------------------------------------
	void prev() {
		if (_hoverIndex == -1) {
			for (uint pos = 0; pos < _path.size(); pos++)
				if (_element[_path[pos]]._visible == true) {
					_hoverIndex = _path[pos];
					break;
				}
		} else {
			uint curpos = 0;
			for (; curpos < _path.size(); curpos++)
				if ((int)_path[curpos] == _hoverIndex)
					break;

			if (curpos == 0)
				return;			// There is no previous element

			int nextloc = curpos - 1;
			while (nextloc != (int)curpos) {
				if (nextloc < 0)
					nextloc = _element.size() - 1;

				if (_element[nextloc]._visible == true) {
					_hoverIndex = _path[nextloc];
					break;
				}

				nextloc--;
			}
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Handle keyboard input
	//------------------------------------------------------------------------
	int handleKeyboard(const Common::Event &event) {
		using namespace pyrodactyl::input;

		if (g_engine->_inputManager->getKeyBindingMode() != KBM_UI) {
			g_engine->_inputManager->setKeyBindingMode(KBM_UI);
		}

		if (!_element.empty()) {
			if (_pathType != PATH_HORIZONTAL) {
				if (g_engine->_inputManager->state(IU_DOWN)) {
					next();
					_latestInput = KEYBOARD;
				} else if (g_engine->_inputManager->state(IU_UP)) {
					prev();
					_latestInput = KEYBOARD;
				}
			}

			if (_pathType != PATH_VERTICAL) {
				if (g_engine->_inputManager->state(IU_RIGHT)) {
					next();
					_latestInput = KEYBOARD;
				} else if (g_engine->_inputManager->state(IU_LEFT)) {
					prev();
					_latestInput = KEYBOARD;
				}
			}

			if (g_engine->_inputManager->state(IU_ACCEPT) && _hoverIndex != -1)
				return _hoverIndex;

			// We pressed a key, which means we have to update the hovering status
			if (_latestInput == KEYBOARD) {
				// Update hover status of keys according to the current index
				int i = 0;
				for (auto it = _element.begin(); it != _element.end(); ++it, ++i)
					it->_hoverKey = (i == _hoverIndex);
			}
		}

		return -1;
	}

public:
	// The collection of buttons in the menu
	Common::Array<T> _element;

	Menu() {
		_hoverIndex = -1;
		_useKeyboard = false;
		_latestInput = MOUSE;
		_pathType = PATH_DEFAULT;
	}
	~Menu() {}

	void reset() {
		_latestInput = MOUSE;
		_hoverIndex = -1;
		for (auto &b : _element)
			b.reset();
	}

	void setUI() {
		for (auto &i : _element)
			i.setUI();
	}

	//------------------------------------------------------------------------
	// Purpose: Load the menu from a file
	//------------------------------------------------------------------------
	void load(rapidxml::xml_node<char> *node) {
		if (nodeValid(node)) {
			for (auto n = node->first_node(); n != nullptr; n = n->next_sibling()) {
				T b;
				b.load(n);
				_element.push_back(b);
			}

			loadBool(_useKeyboard, "keyboard", node, false);
			assignPaths();
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Event Handling
	// The reason this function doesn't declare its own Event object is because
	// a menu might not be the only object in a game state
	//------------------------------------------------------------------------
	int handleEvents(const Common::Event &event, const int &xOffset = 0, const int &yOffset = 0) {
		// The keyboard/joystick event handling bit
		if (_useKeyboard) {
			int result = handleKeyboard(event);

			// We have accepted a menu option using the keyboard
			if (result != -1) {
				// Reset the menu state
				reset();
				g_engine->_inputManager->setKeyBindingMode(pyrodactyl::input::KBM_GAME);
				return result;
			}
		}

		// Check if we have moved or clicked the mouse
		if (Common::isMouseEvent(event)) {
			// Since the player is moving the mouse, we have to recalculate hover index at every opportunity
			_hoverIndex = -1;
			_latestInput = MOUSE;
		}

		// The mouse and hotkey event handling bit
		int i = 0;
		for (auto it = _element.begin(); it != _element.end(); ++it, ++i) {
			// We clicked on a button using the mouse
			if (it->handleEvents(event, xOffset, yOffset) == BUAC_LCLICK) {
				// Reset the menu state
				reset();
				g_engine->_inputManager->setKeyBindingMode(pyrodactyl::input::KBM_GAME);
				return i;
			}

			// We did not click a button, however we did hover over the button
			// However if we are use keyboard to browse through the menu, hovering is forgotten until we move the mouse again
			if (it->_hoverMouse && _latestInput == MOUSE) {
				_hoverIndex = i;

				// The latest input is the mouse, which means we have to forget the keyboard hover states
				for (auto &e : _element)
					e._hoverKey = false;
			}
		}

		if (_latestInput == KEYBOARD) {
			// The latest input is the keyboard, which means we have to forget the mouse hover states
			for (auto &it : _element)
				it._hoverMouse = false;
		}
		return -1;
	}

	//------------------------------------------------------------------------
	// Purpose: Draw the menu
	//------------------------------------------------------------------------
	void draw(const int &XOffset = 0, const int &YOffset = 0) {
		for (auto &it : _element)
			it.draw(XOffset, YOffset);
	}

	//------------------------------------------------------------------------
	// Purpose: Get info about the menu
	//------------------------------------------------------------------------
	void useKeyboard(const bool &val) {
		_useKeyboard = val;
	}

	void clear() {
		_element.clear();
	}

	int hoverIndex() {
		return _hoverIndex;
	}

	//------------------------------------------------------------------------
	// Purpose: Assign traversal paths
	//------------------------------------------------------------------------
	void assignPaths() {
		_path.clear();

		// These variables are used to see if the X and Y values of buttons are the same or not
		// Used to determine the path type of the menu
		bool sameX = true, sameY = true;

		if (!_element.empty()) {
			_path.push_back(0);

			for (uint i = 1; i < _element.size(); i++) {
				_path.push_back(i);

				int prevX = _element[i - 1].x;
				int prevY = _element[i - 1].y;

				if (sameX && _element[i].x != prevX)
					sameX = false;

				if (sameY && _element[i].y != prevY)
					sameY = false;
			}
		}

		if (sameX) {
			if (sameY)
				_pathType = PATH_DEFAULT;
			else
				_pathType = PATH_VERTICAL;
		} else if (sameY)
			_pathType = PATH_HORIZONTAL;
		else
			_pathType = PATH_DEFAULT;
	}
};

// A menu with simple buttons
typedef Menu<Button> ButtonMenu;
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_MENU_H
