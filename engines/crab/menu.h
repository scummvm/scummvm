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

#include "ImageManager.h"
#include "button.h"
#include "common_header.h"

namespace pyrodactyl {
namespace ui {
template<typename T>
class Menu {
protected:
	// The index of current selected option and highlighted option
	int hover_index;

	// The order in which a keyboard or gamepad traverses the menu
	std::vector<unsigned int> path;

	// Are keyboard buttons enabled?
	bool use_keyboard;

	// Has a key been pressed?
	enum InputDevice { KEYBOARD,
					   MOUSE } latest_input;

	// Do the paths use horizontal, vertical or both types of input for keyboard traversal
	enum PathType { PATH_DEFAULT,
					PATH_HORIZONTAL,
					PATH_VERTICAL } path_type;

	//------------------------------------------------------------------------
	// Purpose: Find the next element in our path
	//------------------------------------------------------------------------
	void Next() {
		if (hover_index == -1) {
			for (unsigned int pos = 0; pos < path.size(); pos++)
				if (element[path[pos]].visible == true) {
					hover_index = path[pos];
					break;
				}
		} else {
			unsigned int curpos = 0;
			for (; curpos < path.size(); curpos++)
				if (path[curpos] == hover_index)
					break;

			for (unsigned int nextloc = (curpos + 1) % element.size(); nextloc != curpos; nextloc = (nextloc + 1) % element.size())
				if (element[nextloc].visible == true) {
					hover_index = path[nextloc];
					break;
				}
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Find the previous element in our path
	//------------------------------------------------------------------------
	void Prev() {
		if (hover_index == -1) {
			for (unsigned int pos = 0; pos < path.size(); pos++)
				if (element[path[pos]].visible == true) {
					hover_index = path[pos];
					break;
				}
		} else {
			unsigned int curpos = 0;
			for (; curpos < path.size(); curpos++)
				if (path[curpos] == hover_index)
					break;

			int nextloc = curpos - 1;
			while (nextloc != curpos) {
				if (nextloc < 0)
					nextloc = element.size() - 1;

				if (element[nextloc].visible == true) {
					hover_index = path[nextloc];
					break;
				}

				nextloc--;
			}
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Handle keyboard input
	//------------------------------------------------------------------------
	int HandleKeyboard(const SDL_Event &Event) {
		using namespace pyrodactyl::input;

		if (!element.empty()) {
			if (path_type != PATH_HORIZONTAL) {
				if (gInput.Equals(IU_DOWN, Event) == SDL_PRESSED) {
					Next();
					latest_input = KEYBOARD;
				} else if (gInput.Equals(IU_UP, Event) == SDL_PRESSED) {
					Prev();
					latest_input = KEYBOARD;
				}
			}

			if (path_type != PATH_VERTICAL) {
				if (gInput.Equals(IU_RIGHT, Event) == SDL_PRESSED) {
					Next();
					latest_input = KEYBOARD;
				} else if (gInput.Equals(IU_LEFT, Event) == SDL_PRESSED) {
					Prev();
					latest_input = KEYBOARD;
				}
			}

			if (gInput.Equals(IU_ACCEPT, Event) == SDL_PRESSED && hover_index != -1)
				return hover_index;

			// We pressed a key, which means we have to update the hovering status
			if (latest_input == KEYBOARD) {
				// Update hover status of keys according to the current index
				unsigned int i = 0;
				for (auto it = element.begin(); it != element.end(); ++it, ++i)
					it->hover_key = (i == hover_index);
			}
		}

		return -1;
	}

public:
	// The collection of buttons in the menu
	std::vector<T> element;

	Menu() {
		hover_index = -1;
		use_keyboard = false;
		latest_input = MOUSE;
		path_type = PATH_DEFAULT;
	}
	~Menu() {}

	void Reset() {
		latest_input = MOUSE;
		hover_index = -1;
		for (auto b = element.begin(); b != element.end(); ++b)
			b->Reset();
	}

	void SetUI() {
		for (auto i = element.begin(); i != element.end(); ++i)
			i->SetUI();
	}

	//------------------------------------------------------------------------
	// Purpose: Load the menu from a file
	//------------------------------------------------------------------------
	void Load(rapidxml::xml_node<char> *node) {
		if (NodeValid(node)) {
			for (auto n = node->first_node(); n != NULL; n = n->next_sibling()) {
				T b;
				b.Load(n);
				element.push_back(b);
			}

			LoadBool(use_keyboard, "keyboard", node, false);
			AssignPaths();
		}
	}

	//------------------------------------------------------------------------
	// Purpose: Event Handling
	// The reason this function doesn't declare its own Event object is because
	// a menu might not be the only object in a game state
	//------------------------------------------------------------------------
	int HandleEvents(const SDL_Event &Event, const int &XOffset = 0, const int &YOffset = 0) {
		// The keyboard/joystick event handling bit
		if (use_keyboard) {
			int result = HandleKeyboard(Event);

			// We have accepted a menu option using the keyboard
			if (result != -1) {
				// Reset the menu state
				Reset();
				return result;
			}
		}

		// Check if we have moved or clicked the mouse
		if (Event.type == SDL_MOUSEMOTION || Event.type == SDL_MOUSEBUTTONDOWN || Event.type == SDL_MOUSEBUTTONUP) {
			// Since the player is moving the mouse, we have to recalculate hover index at every opportunity
			hover_index = -1;
			latest_input = MOUSE;
		}

		// The mouse and hotkey event handling bit
		int i = 0;
		for (auto it = element.begin(); it != element.end(); ++it, ++i) {
			// We clicked on a button using the mouse
			if (it->HandleEvents(Event, XOffset, YOffset) == BUAC_LCLICK) {
				// Reset the menu state
				Reset();
				return i;
			}

			// We did not click a button, however we did hover over the button
			// However if we are use keyboard to browse through the menu, hovering is forgotten until we move the mouse again
			if (it->hover_mouse && latest_input == MOUSE) {
				hover_index = i;

				// The latest input is the mouse, which means we have to forget the keyboard hover states
				for (auto e = element.begin(); e != element.end(); ++e)
					e->hover_key = false;
			}
		}

		if (latest_input == KEYBOARD) {
			// The latest input is the keyboard, which means we have to forget the mouse hover states
			for (auto it = element.begin(); it != element.end(); ++it)
				it->hover_mouse = false;
		}

		return -1;
	}

	//------------------------------------------------------------------------
	// Purpose: Draw the menu
	//------------------------------------------------------------------------
	void Draw(const int &XOffset = 0, const int &YOffset = 0) {
		for (auto it = element.begin(); it != element.end(); ++it)
			it->Draw(XOffset, YOffset);
	}

	//------------------------------------------------------------------------
	// Purpose: Get info about the menu
	//------------------------------------------------------------------------
	void UseKeyboard(const bool &val) { use_keyboard = val; }
	void Clear() { element.clear(); }
	const int HoverIndex() { return hover_index; }

	//------------------------------------------------------------------------
	// Purpose: Assign traversal paths
	//------------------------------------------------------------------------
	void AssignPaths() {
		path.clear();

		// These variables are used to see if the X and Y values of buttons are the same or not
		// Used to determine the path type of the menu
		bool same_x = true, same_y = true;

		if (!element.empty()) {
			path.push_back(0);

			for (unsigned int i = 1; i < element.size(); i++) {
				path.push_back(i);

				int prev_x = element.at(i - 1).x;
				int prev_y = element.at(i - 1).y;

				if (same_x && element.at(i).x != prev_x)
					same_x = false;

				if (same_y && element.at(i).y != prev_y)
					same_y = false;
			}
		}

		if (same_x) {
			if (same_y)
				path_type = PATH_DEFAULT;
			else
				path_type = PATH_VERTICAL;
		} else if (same_y)
			path_type = PATH_HORIZONTAL;
		else
			path_type = PATH_DEFAULT;
	}
};

// A menu with simple buttons
typedef Menu<Button> ButtonMenu;
} // End of namespace ui
} // End of namespace pyrodactyl

#endif // CRAB_MENU_H
