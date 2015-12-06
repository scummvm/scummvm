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
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "graphics/palette.h"
#include "common/events.h"

#include "lab/lab.h"
#include "lab/image.h"

namespace Lab {

/*****************************************************************************/
/* Sets the current page on the VGA card.                                    */
/*****************************************************************************/
void LabEngine::changeVolume(int delta) {
	warning("STUB: changeVolume()");
}


void LabEngine::waitTOF() {
	g_system->copyRectToScreen(_graphics->_displayBuffer, _graphics->_screenWidth, 0, 0, _graphics->_screenWidth, _graphics->_screenHeight);
	g_system->updateScreen();

  	_event->processInput();

  	uint32 now;

	for (now = g_system->getMillis(); now - _lastWaitTOFTicks <= 0xF; now = g_system->getMillis() )
		g_system->delayMillis(_lastWaitTOFTicks - now + 17);

	_lastWaitTOFTicks = now;
}

} // End of namespace Lab
