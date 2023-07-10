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
#ifndef CRAB_DIALOGBOX_H
#define CRAB_DIALOGBOX_H

#include "crab/event/GameEventInfo.h"
#include "crab/ui/ParagraphData.h"
#include "crab/ui/button.h"
#include "crab/common_header.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class GameDialogBox {
	// The position of the dialog box
	Element pos;

	// The area you click to skip to the next dialog
	Button button;

	// Information related to drawing the dialog box
	ParagraphData text;

	// The usual background
	ImageKey bg;

	// The background drawn when we don't want to show the opinion bars
	ImageKey bg_p;

public:
	GameDialogBox() {
		bg = 0;
		bg_p = 0;
	}
	~GameDialogBox() {}

	void load(rapidxml::xml_node<char> *node);

	void draw(pyrodactyl::event::Info &info, Common::String &message);
	void draw(const bool &player);

	bool HandleEvents(const Common::Event &Event);

#if 0
	bool HandleEvents(const SDL_Event &Event);
#endif

	void setUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_DIALOGBOX_H
