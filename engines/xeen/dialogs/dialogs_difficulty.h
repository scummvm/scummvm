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

#ifndef XEEN_DIALOGS_DIFFICULTY_H
#define XEEN_DIALOGS_DIFFICULTY_H

#include "xeen/dialogs/dialogs.h"
#include "xeen/party.h"

namespace Xeen {

class DifficultyDialog : public ButtonContainer {
private:
	SpriteResource _sprites;

	/**
	 * Constructor
	 */
	DifficultyDialog(XeenEngine *vm);

	/**
	 * Shows the dialog
	 */
	int execute();

	/**
	 * Loads buttons for the dialog
	 */
	void loadButtons();
public:
	/**
	 * Shows the difficulty selection dialog
	 * @param vm		Engine reference
	 * @returns			0=Adventurer, 1=Warrior, -1 exit
	 */
	static int show(XeenEngine *vm);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_DIFFICULTY_H */
