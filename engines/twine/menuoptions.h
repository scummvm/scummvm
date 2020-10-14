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

#ifndef TWINE_MENUOPTIONS_H
#define TWINE_MENUOPTIONS_H

#include "common/scummsys.h"
#include "twine/actor.h"

namespace TwinE {

class MenuOptions {
private:
	TwinEEngine *_engine;

	int32 enterPlayerName(int32 textIdx);
	void drawSelectableCharacters();
	void drawPlayerName(int32 centerx, int32 top, int8 *playerName, int32 type);
	void drawSelectableCharacter(int32 x, int32 y, int32 arg);
	void showCredits();
	void newGame();

public:
	MenuOptions(TwinEEngine *engine) : _engine(engine) {}

	int32 canShowCredits = 0;

	int8 playerName[256] = "";
	int8 enterPlayerNameVar1 = 0;
	int32 enterPlayerNameVar2 = 0;

	/** Main menu new game options */
	void newGameMenu();

	/** Main menu continue game options */
	void continueGameMenu();
};

} // namespace TwinE

#endif
