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

#ifndef XEEN_DIALOGS_AWARDS_H
#define XEEN_DIALOGS_AWARDS_H

#include "xeen/dialogs/dialogs.h"
#include "xeen/character.h"

namespace Xeen {

class Awards : public ButtonContainer {
private:
	SpriteResource _iconSprites;
private:
	Awards(XeenEngine *vm) : ButtonContainer(vm) {}

	/**
	 * Executes the dialog
	 */
	void execute(const Character *ch);

	/**
	 * Add buttons for the dialog
	 */
	void addButtons();
public:
	static void show(XeenEngine *vm, const Character *ch);
};

} // End of namespace Xeen

#endif /* XEEN_DIALOGS_AWARDS_H */
