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

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* CLOSING		The closing screen and error handler. */

#ifndef AVALANCHE_CLOSING_H
#define AVALANCHE_CLOSING_H

#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Closing {
public:
	Closing(AvalancheEngine *vm);
	void exitGame();

private:
	// Will be needed during implementation of Closing.
	enum ScreenType {
		kScreenBugAlert = 1,
		kScreenRamCram = 2,
		kScreenNagScreen = 3,
		kScreenTwoCopies = 5
	};

	AvalancheEngine *_vm;

	void getScreen(ScreenType which);
	void showScreen();
	void putIn(Common::String str, uint16 where);

};

} // End of namespace Avalanche.

#endif // AVALANCHE_CLOSING_H
