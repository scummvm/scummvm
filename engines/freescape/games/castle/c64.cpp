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

#include "common/file.h"

#include "freescape/freescape.h"
#include "freescape/games/castle/castle.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

void CastleEngine::initC64() {
	_viewArea = Common::Rect(32, 32, 288, 136);
}

extern byte kC64Palette[16][3];

void CastleEngine::loadAssetsC64FullGame() {
	Common::File file;
	file.open("castlemaster.c64.data");

	if (!file.isOpen())
		error("Failed to open castlemaster.c64.data");

	loadMessagesVariableSize(&file, 0x13a9, 75);
	// TODO: riddles need C64-specific parsing (embedded control bytes differ from CPC)
	//loadRiddles(&file, 0x1811, 9);
	load8bitBinary(&file, 0x9951, 16);

	// TODO: extract border from game data or add bundled image
	// TODO: title screen is in BASIC loader (file 009) - not yet extracted
}

void CastleEngine::drawC64UI(Graphics::Surface *surface) {
}

} // End of namespace Freescape
