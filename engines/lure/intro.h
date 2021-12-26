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

#ifndef LURE_INTRO_H
#define LURE_INTRO_H

#include "lure/screen.h"

namespace Lure {

class Introduction {
public:
	Introduction() : _currentSound(0xFF) { }

	bool show();
private:
	uint8 _currentSound;

	bool showScreen(uint16 screenId, uint16 paletteId, uint16 delaySize, bool fadeOut = true);
	bool interruptableDelay(uint32 milliseconds);
	bool playMusic(uint8 soundNumber, bool fadeOut);
};

} // End of namespace Lure

#endif
