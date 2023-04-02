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
#include "crab/ui/dialogbox.h"

namespace Crab {

using namespace pyrodactyl::ui;
using namespace pyrodactyl::text;
using namespace pyrodactyl::image;
using namespace pyrodactyl::input;

//------------------------------------------------------------------------
// Purpose: Load stuff
//------------------------------------------------------------------------
void GameDialogBox::Load(rapidxml::xml_node<char> *node) {
	LoadImgKey(bg, "bg", node);
	LoadImgKey(bg_p, "bg_p", node);
	pos.Load(node, bg);

	if (NodeValid("text", node))
		text.Load(node->first_node("text"), &pos);

	if (NodeValid("button", node))
		button.Load(node->first_node("button"));
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box background
//------------------------------------------------------------------------
void GameDialogBox::Draw(const bool &player) {
	if (player)
		gImageManager.Draw(pos.x, pos.y, bg_p);
	else
		gImageManager.Draw(pos.x, pos.y, bg);
}

//------------------------------------------------------------------------
// Purpose: Draw the dialog box text
//------------------------------------------------------------------------
void GameDialogBox::Draw(pyrodactyl::event::Info &info, std::string &message) {
	// Create a copy of the string
	std::string msg = message;
	info.InsertName(msg);

	text.Draw(message);
	button.Draw();
}

#if 0
//------------------------------------------------------------------------
// Purpose: Handle input
//------------------------------------------------------------------------
bool GameDialogBox::HandleEvents(const SDL_Event &Event) {
	return (button.HandleEvents(Event) == BUAC_LCLICK);
}
#endif

void GameDialogBox::SetUI() {
	pos.SetUI();
	text.SetUI(&pos);
	button.SetUI();
}

} // End of namespace Crab
