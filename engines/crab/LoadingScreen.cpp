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
#include "crab/LoadingScreen.h"
#include "crab/XMLDoc.h"

namespace Crab {

void LoadingScreen::Load() {
	const Common::String &filename = "res/layout/loading.xml";
	XMLDoc doc(filename);
	if (doc.ready()) {
		rapidxml::xml_node<char> *node = doc.doc()->first_node("loading");
		if (nodeValid(node)) {
			if (nodeValid("screens", node)) {
				rapidxml::xml_node<char> *scrnode = node->first_node("screens");
				for (auto n = scrnode->first_node("screen"); n != NULL; n = n->next_sibling("screen"))
					screen.push_back(n);
			}

			if (nodeValid("text", node))
				text.Load(node->first_node("text"), "img");
		}
	}
}

void LoadingScreen::Draw() {
	// Change to a random screen
	Change();

	// Draw the current screen
	if (cur < screen.size())
		screen[cur].Draw();

	// Draw the loading text
	text.Draw((g_engine->_screenSettings->cur.w - text.W()) / 2, (g_engine->_screenSettings->cur.h - text.H()) / 2);

	g_engine->_screen->update();

	// Update the screen
	//SDL_RenderPresent(gRenderer);
}

void LoadingScreen::Dim() {
#if 0
	// This is used when starting or loading a game from the main menu in order to dim the screen
	// until an actual loading screen is drawn
	SDL_SetRenderDrawBlendMode(gRenderer, SDL_BLENDMODE_BLEND);
	SDL_SetRenderDrawColor(gRenderer, 0, 0, 0, 200);
	SDL_RenderFillRect(gRenderer, NULL);

	// Update the screen
	SDL_RenderPresent(gRenderer);
#endif
}

void LoadingScreen::Quit() {
	text.Delete();

	for (auto i = screen.begin(); i != screen.end(); ++i)
		i->Delete();
}

} // End of namespace Crab
