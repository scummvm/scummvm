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
	loadImgKey(bg, "bg", node);
	loadImgKey(bg_p, "bg_p", node);
	pos.load(node, bg);

	if (nodeValid("text", node))
		text.load(node->first_node("text"), &pos);

	if (nodeValid("button", node))
		button.load(node->first_node("button"));
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box background
//------------------------------------------------------------------------
void GameDialogBox::draw(const bool &player) {
	if (player)
		g_engine->_imageManager->draw(pos.x, pos.y, bg_p);
	else
		g_engine->_imageManager->draw(pos.x, pos.y, bg);
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box text
//------------------------------------------------------------------------
void GameDialogBox::draw(pyrodactyl::event::Info &info, Common::String &message) {
	// Create a copy of the string
	Common::String msg = message;
	info.InsertName(msg);

	text.draw(message);
	button.draw();
}

//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
bool GameDialogBox::HandleEvents(const Common::Event &Event) {
	return (button.HandleEvents(Event) == BUAC_LCLICK);
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
bool GameDialogBox::HandleEvents(const SDL_Event &Event) {
	return (button.HandleEvents(Event) == BUAC_LCLICK);
}
#endif



void GameDialogBox::setUI() {
	pos.setUI();
	text.setUI(&pos);
	button.setUI();
}

} // End of namespace Crab
