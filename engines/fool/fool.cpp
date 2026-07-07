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
#include "graphics/mactoolbox/toolbox.h"
#include "graphics/palette.h"

#include "fool/fool.h"
#include "fool/fool_game.h"
#include "fool/fool_prologue.h"
#include "fool/detection.h"

namespace Fool {

FoolEngine *g_engine;

FoolEngine::FoolEngine(OSystem *syst, const FOOLGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Fool") {
	g_engine = this;
}

FoolEngine::~FoolEngine() {
}

uint32 FoolEngine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String FoolEngine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error FoolEngine::run() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	_screen.create(SCREEN_WIDTH, SCREEN_HEIGHT, Graphics::PixelFormat::createFormatCLUT8());
	_wm.setScreen(&_screen);

	Common::String versionStr(_gameDescription->desc.extra);
	FoolVersion version = kFool20;
	if (versionStr == "v1.1") {
		version = kFool11;
	} else if (versionStr == "v2.0") {
		version = kFool20;
	} else if (versionStr == "v3.0") {
		version = kFool30;
	}

	FoolGame fg(version);
	fg.run();

	return Common::kNoError;
}

Common::Error FoolEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	int dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

} // End of namespace Fool
