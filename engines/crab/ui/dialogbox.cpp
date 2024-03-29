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
// Purpose:  Dialog box!
//=============================================================================
#include "crab/crab.h"
#include "crab/ui/dialogbox.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Load stuff
//------------------------------------------------------------------------
void GameDialogBox::load(rapidxml::xml_node<char> *node) {
	loadImgKey(_bg, "bg", node);
	loadImgKey(_bgP, "bg_p", node);
	_pos.load(node, _bg);

	if (nodeValid("text", node))
		_text.load(node->first_node("text"), &_pos);

	if (nodeValid("button", node))
		_button.load(node->first_node("button"));
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box background
//------------------------------------------------------------------------
void GameDialogBox::draw(const bool &player) {
	if (player)
		g_engine->_imageManager->draw(_pos.x, _pos.y, _bgP);
	else
		g_engine->_imageManager->draw(_pos.x, _pos.y, _bg);
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box text
//------------------------------------------------------------------------
void GameDialogBox::draw(pyrodactyl::event::Info &info, Common::String &message) {
	// Create a copy of the string
	Common::String msg = message;
	info.insertName(msg);

	_text.draw(message);
	_button.draw();
}

//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
bool GameDialogBox::handleEvents(const Common::Event &event) {
	// Switch to KBM_UI
	if (g_engine->_inputManager->getKeyBindingMode() != KBM_UI)
		g_engine->_inputManager->setKeyBindingMode(KBM_UI);

	bool isLeftClick = (_button.handleEvents(event) == BUAC_LCLICK);

	if (isLeftClick) {
		// Switch to KBM_GAME
		g_engine->_inputManager->setKeyBindingMode(KBM_GAME);
	}

	return isLeftClick;
}

void GameDialogBox::setUI() {
	_pos.setUI();
	_text.setUI(&_pos);
	_button.setUI();
}

} // End of namespace Crab
