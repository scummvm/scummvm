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

#include "common/system.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/console.h"
#include "ultima/ultima2/metaengine.h"
#include "ultima/ultima2/views/views.h"
#include "ultima/ultima2/views/map.h"
#include "ultima/ultima2/views/interactions/dead.h"

namespace Ultima {
namespace Ultima2 {

Ultima2Engine *g_engine;

Ultima2Engine::Ultima2Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) : Engine(syst),
_gameDescription(gameDesc) {
	g_engine = this;
}

Ultima2Engine::~Ultima2Engine() {
}

uint32 Ultima2Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String Ultima2Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error Ultima2Engine::run() {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);

	// Set the engine's debugger console
	setDebugger(new Console());

	byte seed = (byte)getRandomNumber(255);
	for (int i = 0; i < 6; ++i)
		_rngState[i] = seed;
	_rngState[0]++;
	_rngState[1]++;

	Views::Views views;
	addView("Startup");
	runGame(views);

	return Common::kNoError;
}

byte Ultima2Engine::randByte() {
	byte newVal = (byte)(_rngState[1] + _rngState[4] + _rngState[5] + 1);
	_rngState[5] = _rngState[4];
	_rngState[4] = _rngState[3];
	_rngState[3] = _rngState[2];
	_rngState[2] = _rngState[1];
	_rngState[1] = _rngState[0];
	_rngState[0] = newVal;
	return newVal;
}

bool Ultima2Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	// Only allow saving when any of the different map views are active
	return dynamic_cast<Views::Map *>(focusedView()) != nullptr;
}

bool Ultima2Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveGameStateCurrently() || dynamic_cast<Views::Interactions::Dead *>(focusedView()) != nullptr;
}

Common::Error Ultima2Engine::syncGame(Common::Serializer &s) {
	_savegame.synchronize(s);

	if (s.isLoading()) {
		_G(map).load(_G(savegame)._mapEra, _G(savegame)._mapType);
	}

	return Common::kNoError;
}

bool Ultima2Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

void Ultima2Engine::playFX(int num) {
	warning("TODO: playFX(%d)", num);
}

} // namespace Ultima2
} // namespace Ultima
