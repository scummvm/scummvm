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
// Purpose:  TextArea functions
//=============================================================================
#include "crab/crab.h"
#include "crab/ui/textarea.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::input;

void TextArea::load(rapidxml::xml_node<char> *node) {
	if (TextData::load(node)) {
		loadStr(_text, "text", node);
		loadNum(_size, "size", node);

		loadNum(_seEntry, "entry", node);
		loadNum(_seErase, "erase", node);
		loadNum(_seAccept, "accept", node);

		if (nodeValid("caption", node))
			_title.load(node->first_node("caption"), this);
	}
}

bool TextArea::handleEvents(const Common::Event &event, bool numbersOnly) {
	warning("STUB: TextArea::handleEvents()");
	if (event.type == Common::EVENT_KEYDOWN && event.kbd.ascii == Common::ASCII_BACKSPACE && _text.size() != 0) {
		// Now play the text erase sound
		g_engine->_musicManager->PlayEffect(_seErase, 0);

		// If backspace was pressed and the string isn't blank, remove a character from the end
		_text.erase(_text.size() - 1);
	} else if (event.type == Common::EVENT_KEYDOWN) {
		// If the string less than maximum size and does not contain invalid characters \ / : * ? " < > |
		if (_text.size() < _size && event.kbd.ascii != '\\' \
			&& event.kbd.ascii != '/' && event.kbd.ascii != ':' \
			&& event.kbd.ascii != '*' && event.kbd.ascii != '?' \
			&& event.kbd.ascii != '\"' && event.kbd.ascii != '<' \
			&& event.kbd.ascii != '>' && event.kbd.ascii != '|') {
			// Should we only accept numbers?
			if (numbersOnly && (event.kbd.ascii < '0' || event.kbd.ascii > '9'))
				return false;

			// Now play the text input sound
			g_engine->_musicManager->PlayEffect(_seEntry, 0);

			// Append the character to string
			_text += event.kbd.ascii;
		}
	} else if (g_engine->_inputManager->state(IU_ACCEPT) && _text.size() != 0) {
		// Now play the accept sound
		g_engine->_musicManager->PlayEffect(_seAccept, 0);

		return true;
	}

	return false;
}


#if 0
bool TextArea::handleEvents(const SDL_Event &Event, bool numbers_only) {
	// If a key was pressed
	if (Event.type == SDL_TEXTINPUT) {
		// If the string less than maximum size and does not contain invalid characters \ / : * ? " < > |
		if (text.length() < size && Event.kbd.ascii != '\\' && Event.kbd.ascii != '/' && Event.kbd.ascii != ':' && Event.kbd.ascii != '*' && Event.kbd.ascii != '?' && Event.kbd.ascii != '\"' && Event.kbd.ascii != '<' && Event.kbd.ascii != '>' && Event.kbd.ascii != '|') {
			// Should we only accept numbers?
			if (numbers_only && !isdigit(Event.kbd.ascii))
				return false;

			// Now play the text input sound
			g_engine->_musicManager->PlayEffect(se_entry, 0);

			// Append the character to string
			text += Event.text.text;
		}
	} else if (Event.type == SDL_KEYDOWN && Event.key.keysym.scancode == SDL_SCANCODE_BACKSPACE && text.length() != 0) {
		// Now play the text erase sound
		g_engine->_musicManager->PlayEffect(se_erase, 0);

		// If backspace was pressed and the string isn't blank, remove a character from the end
		text.erase(text.length() - 1);
	} else if (g_engine->_inputManager->Equals(IU_ACCEPT, Event) == SDL_RELEASED && text.length() != 0) {
		// Now play the accept sound
		g_engine->_musicManager->PlayEffect(se_accept, 0);

		return true;
	}

	return false;
}
#endif

//------------------------------------------------------------------------
// Purpose: Draw
//------------------------------------------------------------------------
void TextArea::draw() {
	_title.draw();
	TextData::draw(_text + "_");
}

} // End of namespace Crab
