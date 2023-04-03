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
// Purpose:  A box for entering text
//=============================================================================
#ifndef CRAB_TEXTAREA_H
#define CRAB_TEXTAREA_H

#include "crab/ui/HoverInfo.h"
#include "crab/music/MusicManager.h"
#include "crab/text/TextManager.h"
#include "crab/common_header.h"
//#include "crab/input/input.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class TextArea : public TextData {
	// The maximum number of characters allowed
	unsigned int size;

	// The name of the text area is stored here
	HoverInfo title;

	// Sound effects
	pyrodactyl::music::ChunkKey se_entry, se_erase, se_accept;

public:
	std::string text;

	TextArea(void) : text("") {
		size = 20;
		se_entry = -1;
		se_erase = -1;
		se_accept = -1;
	}
	~TextArea(void) {}

	void Load(rapidxml::xml_node<char> *node);

	bool HandleEvents(const Common::Event &Event, bool numbers_only = false);
#if 0
	bool HandleEvents(const SDL_Event &Event, bool numbers_only = false);
#endif

	void Draw();

	void SetUI() {
		title.SetUI();
		TextData::SetUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTAREA_H
