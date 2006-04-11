/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __lure_game_h__
#define __lure_game_h__

#include "common/stdafx.h"
#include "base/engine.h"
#include "lure/luredefs.h"
#include "lure/menu.h"
#include "lure/palette.h"
#include "lure/disk.h"
#include "lure/memory.h"
#include "lure/screen.h"
#include "lure/events.h"

namespace Lure {

class Game {
private:
	bool _slowSpeedFlag, _soundFlag;

	void handleMenuResponse(uint8 selection);
	void handleClick();
	void handleRightClickMenu();
	void handleLeftClick();

	void playerChangeRoom();
public:
	Game();
	static Game &getReference();

	void nextFrame();
	void execute();

	// Menu item support methods
	void doDebugMenu();
	void doShowCredits();
	void doQuit();
	void doTextSpeed();
	void doSound();
};

} // End of namespace Lure

#endif
