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
	uint _size;

	// The name of the text area is stored here
	HoverInfo _title;

	// Sound effects
	pyrodactyl::music::ChunkKey _seEntry, _seErase, _seAccept;

public:
	Common::String _text;

	TextArea(void) : _text("") {
		_size = 20;
		_seEntry = -1;
		_seErase = -1;
		_seAccept = -1;
	}
	~TextArea(void) {}

	void load(rapidxml::xml_node<char> *node);

	bool handleEvents(const Common::Event &event, bool numbersOnly = false);

	void draw();

	void setUI() {
		_title.setUI();
		TextData::setUI();
	}
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_TEXTAREA_H
