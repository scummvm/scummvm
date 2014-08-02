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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "access/amazon/amazon_game.h"

namespace Access {

namespace Amazon {

AmazonEngine::AmazonEngine(OSystem *syst, const AccessGameDescription *gameDesc) :
		AccessEngine(syst, gameDesc) {
}

void AmazonEngine::doTitle() {
	_screen->setDisplayScan();
	_screen->forceFadeOut();
	_events->hideCursor();

	_sound->_soundTable[0] = _sound->loadSound(98, 30);
	_sound->_soundPriority[0] = 1;
	_sound->_soundTable[1] = _sound->loadSound(98, 8);
	_sound->_soundPriority[1] = 2;

	_screen->_loadPalFlag = false;
	byte *scr = _files->loadScreen(0, 3);
	_screen->copyBuffer(scr);
}

} // End of namespace Amazon

} // End of namespace Access
