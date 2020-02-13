/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA_ULTIMA1_U1DIALOGS_DIALOG_H
#define ULTIMA_ULTIMA1_U1DIALOGS_DIALOG_H

#include "ultima/shared/gfx/popup.h"
#include "ultima/shared/gfx/character_input.h"
#include "ultima/shared/core/str.h"

namespace Ultima {
namespace Ultima1 {

class Ultima1Game;
namespace Maps {
	class Ultima1Map;
}

namespace U1Dialogs {

/**
 * Base class for Ultima 1 popup dialogs
 */
class Dialog : public Shared::Gfx::Popup {
protected:
	Ultima1Game *_game;
	Common::String _prompt;
protected:
	/**
	 * Jumps up through the parents to find the root game
	 */
	Ultima1Game *getGame() { return _game; }

	/**
	 * Return the game's map
	 */
	Maps::Ultima1Map *getMap();

	/**
	 * Adds a text string to the info area
	 * @param text			Text to add
	 * @param newLine		Whether to apply a newline at the end
	 * @param replaceLine	If true, replaces the current last line
	 */
	void addInfoMsg(const Common::String &text, bool newLine = true, bool replaceLine = false);

	/**
	 * Prompts for a keypress
	 */
	void getKeypress();

	/**
	 * Prompts for an input
	 */
	void getInput(bool isNumeric = true, size_t maxCharacters = 4);

	/**
	 * Write a text line to the dialog
	 */
	void centerText(const Common::String &line, int yp);

	/**
	 * Write a text line to the dialog
	 */
	void centerText(const Shared::StringArray &lines, int yp);
public:
	/**
	 * Constructor
	 */
	Dialog(Ultima1Game *game);

	/**
	 * Draws the visual item on the screen
	 */
	void draw() override;

	/**
	 * Hide the dialog
	 */
	void hide() override;
};

} // End of namespace U1Dialogs
} // End of namespace Ultima1
} // End of namespace Ultima

#endif
