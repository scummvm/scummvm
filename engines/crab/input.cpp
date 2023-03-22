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

#include "input.h"

namespace pyrodactyl {
namespace input {
InputManager gInput;
}
} // End of namespace pyrodactyl

using namespace pyrodactyl::input;
using namespace boost::filesystem;

//------------------------------------------------------------------------
// Purpose: Return pressed/depressed state of key
//------------------------------------------------------------------------
const bool InputManager::State(const InputType &val) {
	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	if (keystate[iv[val].key] || keystate[iv[val].alt])
		return true;

	if (controller != nullptr) {
		if (iv[val].c_bu >= 0 && SDL_GameControllerGetButton(controller, iv[val].c_bu) == SDL_PRESSED)
			return true;
		else if (iv[val].c_ax.id != SDL_CONTROLLER_AXIS_INVALID) {
			int av = SDL_GameControllerGetAxis(controller, iv[val].c_ax.id);
			return (iv[val].c_ax.greater && av > iv[val].c_ax.val) || (!iv[val].c_ax.greater && av < iv[val].c_ax.val);
		}
	}

	return false;
}

//------------------------------------------------------------------------
// Purpose: Check for controller and keyboard input simultaneously
// Return value 2 is a special case used for analog stick hotkey input
//------------------------------------------------------------------------
const int InputManager::Equals(const InputType &val, const SDL_Event &Event) {
	switch (Event.type) {
	case SDL_KEYDOWN:
		if (iv[val].Equals(Event.key))
			return SDL_PRESSED;
		break;
	case SDL_KEYUP:
		if (iv[val].Equals(Event.key))
			return SDL_RELEASED;
		break;

	case SDL_CONTROLLERBUTTONDOWN:
		if (iv[val].Equals(Event.cbutton))
			return SDL_PRESSED;
		break;
	case SDL_CONTROLLERBUTTONUP:
		if (iv[val].Equals(Event.cbutton))
			return SDL_RELEASED;
		break;

	case SDL_CONTROLLERAXISMOTION:
		// HACK to make left analog stick work in menus and right analog stick work in hotkeys
		if (iv[val].Equals(Event.caxis)) {
			if (iv[val].c_ax.id == SDL_CONTROLLER_AXIS_LEFTX || iv[val].c_ax.id == SDL_CONTROLLER_AXIS_LEFTY)
				return SDL_PRESSED;
			else
				return ANALOG_PRESSED;
		}
		break;
	default:
		break;
	}

	return -1;
}

//------------------------------------------------------------------------
// Purpose: Load from file
//------------------------------------------------------------------------
void InputManager::Init() {
	const std::string DEFAULT_FILENAME = "res/controls.xml";

	std::string filename = gFilePath.appdata;
	filename += "controls.xml";

	if (!is_regular_file(filename)) {
		// The other file does not exist, just use the default file
		Load(DEFAULT_FILENAME);
	} else {
		// We are using the other file, check if it is up to date or not
		if (Version(DEFAULT_FILENAME) > Version(filename)) {
			// The game has been updated to a different control scheme, use the default file
			Load(DEFAULT_FILENAME);
		} else {
			// The version set by the player is fine, just use that
			Load(filename);
		}
	}

	CreateBackup();
}

//------------------------------------------------------------------------
// Purpose: Load key & controller binding settings from file
//------------------------------------------------------------------------
void InputManager::Load(const std::string &filename) {
	XMLDoc control_list(filename);
	if (control_list.ready()) {
		rapidxml::xml_node<char> *node = control_list.Doc()->first_node("controls");
		if (NodeValid(node)) {
			LoadNum(version, "version", node);

			int i = 0;
			for (auto n = node->first_node(); n != NULL && i < IT_TOTAL; n = n->next_sibling(), ++i)
				iv[i].LoadState(n);
		}
	}
}

//------------------------------------------------------------------------
// Purpose: Initialize the controller if it is plugged in
//------------------------------------------------------------------------
void InputManager::AddController() {
	if (SDL_NumJoysticks() > 0) {
		if (SDL_IsGameController(0))
			controller = SDL_GameControllerOpen(0);
		else
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Controller not recognized", "Unsupported device", NULL);
	}
}

void InputManager::HandleController(const SDL_Event &Event) {
	if (Event.type == SDL_CONTROLLERDEVICEREMOVED && controller != nullptr) {
		SDL_GameControllerClose(controller);
		controller = nullptr;
	} else if (Event.type == SDL_CONTROLLERDEVICEADDED && controller == nullptr)
		AddController();
}

//------------------------------------------------------------------------
// Purpose: Create and restore backup
//------------------------------------------------------------------------
void InputManager::CreateBackup() {
	for (int i = 0; i < IT_TOTAL; ++i)
		backup[i] = iv[i];
}

void InputManager::RestoreBackup() {
	for (int i = 0; i < IT_TOTAL; ++i)
		iv[i] = backup[i];
}

//------------------------------------------------------------------------
// Purpose: Save to file
//------------------------------------------------------------------------
void InputManager::Save() {
	rapidxml::xml_document<char> doc;

	std::string filename = gFilePath.appdata;
	filename += "controls.xml";

	// xml declaration
	rapidxml::xml_node<char> *decl = doc.allocate_node(rapidxml::node_declaration);
	decl->append_attribute(doc.allocate_attribute("version", "1.0"));
	decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
	doc.append_node(decl);

	// root node
	rapidxml::xml_node<char> *root = doc.allocate_node(rapidxml::node_element, "controls");
	root->append_attribute(doc.allocate_attribute("version", gStrPool.Get(version)));
	for (int i = 0; i < IT_TOTAL; i++)
		iv[i].SaveState(doc, root, "i");

	doc.append_node(root);
	std::string xml_as_string;
	rapidxml::print(std::back_inserter(xml_as_string), doc);

	std::ofstream save(filename, std::ios::out);
	if (save.is_open()) {
		save << xml_as_string;
		save.close();
	}

	doc.clear();
	CreateBackup();
}
