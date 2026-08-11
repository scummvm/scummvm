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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"

#include "bolt/bolt.h"
#include "bolt/detection.h"
#include "bolt/xplib/xplib.h"

namespace Bolt {

BoltEngine *g_engine;

BoltEngine::BoltEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Bolt") {
	g_engine = this;
	_xp = new XpLib(this);
}

BoltEngine::~BoltEngine() {
	delete _xp;
}

uint32 BoltEngine::getFeatures() const {
	return _gameDescription->flags;
}

Common::String BoltEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Error BoltEngine::run() {
	const Common::FSNode gameDataDir(ConfMan.getPath("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "assets");

	ConfMan.registerDefault("extended_viewport", false);
	if (ConfMan.hasKey("extended_viewport", _targetName)) {
		_extendedViewport = ConfMan.getBool("extended_viewport");
	}

	// Initialize paletted graphics mode
	if (!_extendedViewport) {
		initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	} else {
		initGraphics(EXTENDED_SCREEN_WIDTH, EXTENDED_SCREEN_HEIGHT);
	}

	if ((getFeatures() & ADGF_DEMO) != 0)
		_isDemo = true;

	_xp->initialize();
	boltMain();
	_xp->terminate();

	return Common::kNoError;
}

void BoltEngine::setCursorPict(byte *sprite) {
	byte cursorBitmap[32]; // 16x16
	byte *srcPtr = getResolvedPtr(sprite, 0x12);

	// Convert 8bpp pixel data to 1bpp monochrome bitmap
	for (int i = 0; i < 32; i++) {
		cursorBitmap[i] = 0x00;
		int16 mask = 0x80;
		while (mask != 0x00) {
			if (*srcPtr++ != 0x00)
				cursorBitmap[i] |= (byte)mask;

			mask >>= 1;
		}
	}

	_xp->setCursorImage(cursorBitmap, 7, 7);
}

} // End of namespace Bolt
