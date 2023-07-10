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
#include "crab/ui/KeyBindMenu.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::image;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void KeyBindMenu::Load(rapidxml::xml_node<char> *node) {
	if (nodeValid(node)) {
		if (nodeValid("menu", node)) {
			rapidxml::xml_node<char> *menode = node->first_node("menu");

			if (nodeValid("primary", menode))
				prim.Load(menode->first_node("primary"));

			if (nodeValid("alt", menode))
				alt.Load(menode->first_node("alt"));

			if (nodeValid("prompt", menode))
				prompt.Load(menode->first_node("prompt"));

			if (nodeValid("inc", menode))
				inc.Load(menode->first_node("inc"));

			if (nodeValid("dim", menode))
				dim.Load(menode->first_node("dim"));

			if (nodeValid("divide", menode))
				divide.Load(menode->first_node("divide"));

			if (nodeValid("desc", menode))
				desc.Load(menode->first_node("desc"));
		}

		// Initialize the menus
		InitMenu(CON_GAME);
		InitMenu(CON_UI);

		if (nodeValid("controls", node))
			sel_controls.Load(node->first_node("controls"));
	}
}

void KeyBindMenu::StartAndSize(const int &type, int &start, int &size) {
	switch (type) {
	case CON_GAME:
		start = IG_START;
		size = IG_SIZE;
		break;
	case CON_UI:
		start = IU_START;
		size = IU_SIZE;
		break;
	default:
		break;
	}
}

void KeyBindMenu::InitMenu(const int &type) {
	warning("STUB: KeyBindMenu::InitMenu()");

#if 0
	int start = 0, size = 0;
	StartAndSize(type, start, size);

	// Initialize the menu
	menu[type].element.resize(size * 2);
	for (int i = 0; i < size * 2; i += 2) {
		int xoffset = inc.x * (i / 2) + divide.x * ((i / 2) / dim.x);
		int yoffset = inc.y * ((i / 2) % dim.x) + divide.y * ((i / 2) / dim.x);

		menu[type].element[i].Init(prim, xoffset, yoffset);
		menu[type].element[i].caption.text = SDL_GetScancodeName(g_engine->_inputManager->iv[start + (i / 2)].key);

		menu[type].element[i + 1].Init(alt, xoffset, yoffset);
		menu[type].element[i + 1].caption.text = SDL_GetScancodeName(g_engine->_inputManager->iv[start + (i / 2)].alt);
	}
#endif
}

void KeyBindMenu::DrawDesc(const int &type) {
	int start = 0, size = 0;
	StartAndSize(type, start, size);

	for (int i = 0; i < size; i++) {
		int xoffset = inc.x * i + divide.x * (i / dim.x);
		int yoffset = inc.y * (i % dim.x) + divide.y * (i / dim.x);

		desc.Draw(g_engine->_inputManager->iv[i + start].name, xoffset, yoffset);
	}
}

void KeyBindMenu::Draw() {
	sel_controls.Draw();

	menu[sel_controls.cur].Draw();
	DrawDesc(sel_controls.cur);
}

void KeyBindMenu::SetCaption() {
	warning("STUB: KeyBindMenu::SetCaption()");

#if 0
	int start = 0, size = 0;
	StartAndSize(sel_controls.cur, start, size);

	for (int i = 0; i < size * 2; i += 2) {
		menu[sel_controls.cur].element[i].caption.text = SDL_GetScancodeName(g_engine->_inputManager->iv[start + (i / 2)].key);
		menu[sel_controls.cur].element[i + 1].caption.text = SDL_GetScancodeName(g_engine->_inputManager->iv[start + (i / 2)].alt);
	}
#endif
}

void KeyBindMenu::HandleEvents(const Common::Event &Event) {
	warning("STUB: KeyBindMenu::HandleEvents()");

	/*
	if (sel_controls.HandleEvents(Event))
		SetCaption();

	switch (state) {
	case STATE_NORMAL:
		choice = menu[sel_controls.cur].HandleEvents(Event);
		if (choice >= 0) {
			prompt.Swap(menu[sel_controls.cur].element[choice].caption);
			state = STATE_KEY;
			break;
		}

		break;
	case STATE_KEY:
		if (Event.type == SDL_KEYDOWN) {
			SwapKey(Event.key.keysym.scancode);
			SetCaption();
			menu[sel_controls.cur].element[choice].caption.col = prompt.col_prev;
			state = STATE_NORMAL;
		}
		break;
	default:
		break;
	}
	*/
}

#if 0
void KeyBindMenu::HandleEvents(const SDL_Event &Event) {
	if (sel_controls.HandleEvents(Event))
		SetCaption();

	switch (state) {
	case STATE_NORMAL:
		choice = menu[sel_controls.cur].HandleEvents(Event);
		if (choice >= 0) {
			prompt.Swap(menu[sel_controls.cur].element[choice].caption);
			state = STATE_KEY;
			break;
		}

		break;
	case STATE_KEY:
		if (Event.type == SDL_KEYDOWN) {
			SwapKey(Event.key.keysym.scancode);
			SetCaption();
			menu[sel_controls.cur].element[choice].caption.col = prompt.col_prev;
			state = STATE_NORMAL;
		}
		break;
	default:
		break;
	}
}

void KeyBindMenu::SwapKey(const SDL_Scancode &find) {
	int start = 0, size = 0;
	StartAndSize(sel_controls.cur, start, size);
	int pos = start + (choice / 2);

	for (int i = start; i < start + size; ++i) {
		if (g_engine->_inputManager->iv[i].key == find) {
			g_engine->_inputManager->iv[i].key = g_engine->_inputManager->iv[pos].key;
			break;
		} else if (g_engine->_inputManager->iv[i].alt == find) {
			g_engine->_inputManager->iv[i].alt = g_engine->_inputManager->iv[pos].key;
			break;
		}
	}

	if (choice % 2 == 0)
		g_engine->_inputManager->iv[pos].key = find;
	else
		g_engine->_inputManager->iv[pos].alt = find;
}
#endif

void KeyBindMenu::SetUI() {
	menu[CON_GAME].Clear();
	menu[CON_UI].Clear();

	// Initialize the menus
	prim.SetUI();
	alt.SetUI();
	InitMenu(CON_GAME);
	InitMenu(CON_UI);

	desc.SetUI();
	sel_controls.SetUI();
}

} // End of namespace Crab
