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

#ifndef CRAB_GAMEOVERMENU_H
#define CRAB_GAMEOVERMENU_H

#include "crab/ImageData.h"
#include "crab/TextData.h"
#include "crab/common_header.h"
#include "crab/menu.h"

namespace Crab {

namespace pyrodactyl {
namespace ui {
class GameOverMenu {
	// Background image
	ImageData bg;

	// The selection of quotes, we pick one out of these
	std::vector<std::string> quote;

	// The current picked quote
	unsigned int cur;

	// How to draw the quote
	TextData title;

	// The menu for actions we can take
	ButtonMenu menu;

public:
	GameOverMenu(void) { cur = 0; }
	~GameOverMenu(void) {}

	void Reset() { cur = gRandom.Num() % quote.size(); }

	void Load(rapidxml::xml_node<char> *node);
	int HandleEvents(const SDL_Event &Event);
	void Draw();

	void SetUI();
};
} // End of namespace ui
} // End of namespace pyrodactyl

} // End of namespace Crab

#endif // CRAB_GAMEOVERMENU_H
