/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef CEKEYS_EVENTSBUFFER
#define CEKEYS_EVENTSBUFFER

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/system.h"
#include "common/list.h"

#include <SDL.h>

#include "gui/Key.h"

namespace CEKEYS {

	class EventsBuffer {
	public:
		static bool simulateKey(GUI::Key *key, bool pushed);
		static bool simulateMouseMove(int x, int y);
		static bool simulateMouseLeftClick(int x, int y, bool pushed);
		static bool simulateMouseRightClick(int x, int y, bool pushed);

	};	
}

#endif
