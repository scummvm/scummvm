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

#ifndef XEEN_DIALOGS_CHAR_INFO_H
#define XEEN_DIALOGS_CHAR_INFO_H

#include "xeen/dialogs/dialogs.h"
#include "xeen/party.h"
#include "xeen/window.h"

namespace Xeen {

class CharacterInfo : public ButtonContainer {
private:
	SpriteResource _iconSprites;
	DrawStruct _drawList[24];
	int _cursorCell;

	CharacterInfo(XeenEngine *vm) : ButtonContainer(vm), _cursorCell(0) {}

	void execute(int charIndex);

	/**
	 * Load the draw structure list with frame numbers and positions
	 */
	void loadDrawStructs();

	/**
	 * Set up the button list for the dialog
	 */
	void addButtons();

	/**
	 * Return a string containing the details of the character
	 */
	Common::String loadCharacterDetails(const Character &c);

	/**
	 * Cursor display handling
	 */
	void showCursor(bool flag);

	bool expandStat(int attrib, const Character &c);
public:
	static void show(XeenEngine *vm, int charIndex);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_CHAR_INFO_H */
