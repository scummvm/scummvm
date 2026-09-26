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
#include "audio/softsynth/pcspk.h"
#include "ultima/ultima3/ultima3.h"
#include "ultima/ultima3/console.h"
#include "ultima/ultima3/metaengine.h"
#include "ultima/ultima3/data/data.h"
#include "ultima/ultima3/views/views.h"

namespace Ultima {
namespace Ultima3 {

Ultima3Engine *g_engine;

Ultima3Engine::Ultima3Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), Shared::Events("ultima3"), _gameDescription(gameDesc) {
	g_engine = this;
	_pcSpeaker = new Audio::PCSpeaker();
}

Ultima3Engine::~Ultima3Engine() {
	delete _pcSpeaker;
}

uint32 Ultima3Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String Ultima3Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error Ultima3Engine::run() {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	// Initialize 320x200 graphics mode
	initGraphics(320, 200);
	Data::setCGAPalette();
	_pcSpeakerReady = _pcSpeaker->init();

	// Set the engine's debugger console
	setDebugger(new Console());

	Views::Views views;
	addView("Title");
	runGame();
	(void)views;		// Suppress any warnings of unused local

	return Common::kNoError;
}

bool Ultima3Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	return false;
}

bool Ultima3Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return false;
}

Common::Error Ultima3Engine::syncGame(Common::Serializer &s) {
	// TODO

	return Common::kNoError;
}

void Ultima3Engine::queueTone(int divisor, uint32 lengthMs) {
	if (!_pcSpeakerReady)
		return;

	float freq = divisor > 0 ? 1193182.0f / divisor : 0.0f;
	_pcSpeaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, freq, lengthMs * 1000);
}

void Ultima3Engine::queueSilence(uint32 lengthMs) {
	if (!_pcSpeakerReady)
		return;

	_pcSpeaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0.0f, lengthMs * 1000);
}

bool Ultima3Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

} // namespace Ultima3
} // namespace Ultima
