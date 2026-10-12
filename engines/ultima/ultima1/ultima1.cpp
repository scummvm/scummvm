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

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "engines/util.h"
#include "audio/decoders/raw.h"
#include "graphics/paletteman.h"
#include "ultima/ultima1/ultima1.h"
#include "ultima/ultima1/console.h"
#include "ultima/ultima1/sound_effects.h"
#include "ultima/ultima1/gfx/font.h"
#include "ultima/ultima1/views/views.h"
#include "ultima/ultima1/views/map.h"
#include "ultima/ultima1/metaengine.h"

namespace Ultima {
namespace Ultima1 {

Ultima1Engine *g_engine;

Ultima1Engine::Ultima1Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), Shared::Events("ultima1"), _gameDescription(gameDesc) {
	g_engine = this;
}


Ultima1Engine::~Ultima1Engine() {
	_mixer->stopHandle(_effectHandle);
}

uint32 Ultima1Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String Ultima1Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

Common::Error Ultima1Engine::run() {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	g_system->getPaletteManager()->setPalette(Graphics::Palette::createEGAPalette());

	// Set the engine's debugger console
	setDebugger(new Console());

	// Load needed data
	_map.init();

	Views::Views views;
	addView("Startup");
	runGame();
	(void)views;		// Suppress any warnings of unused local

	return Common::kNoError;
}

bool Ultima1Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	// Only allow saving when any of the different map views are active
	return dynamic_cast<Views::Map *>(focusedView()) != nullptr;
}

bool Ultima1Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveGameStateCurrently() ||
		dynamic_cast<Views::Interactions::Dead *>(focusedView()) != nullptr ||
		dynamic_cast<Views::Interactions::SpaceDead *>(focusedView()) != nullptr;
}

Common::Error Ultima1Engine::syncGame(Common::Serializer &s) {
	if (s.isSaving())
		_G(savegame)._moveCount = _moveCtr;

	_savegame.synchronize(s);

	if (s.isLoading()) {
		_G(map).load(_G(savegame)._mapNum);
	}

	return Common::kNoError;
}

bool Ultima1Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

void Ultima1Engine::playFX(int num) {
	if (_savegame._soundOn)
		queueEffect(SoundEffects::render(num));
}

void Ultima1Engine::queueEffect(const Common::Array<byte> &samples) {
	// Don't let effects pile up behind the action
	constexpr int MAX_QUEUED = 6;

	if (samples.empty())
		return;

	if (!_mixer->isSoundHandleActive(_effectHandle)) {
		_effectStream = Audio::makeQueuingAudioStream(SoundEffects::SAMPLE_RATE, false);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_effectHandle, _effectStream);
	}

	if (_effectStream->numQueuedStreams() >= MAX_QUEUED)
		return;

	byte *data = (byte *)malloc(samples.size());
	memcpy(data, &samples[0], samples.size());
	_effectStream->queueBuffer(data, samples.size(), DisposeAfterUse::YES, Audio::FLAG_UNSIGNED);
}

void Ultima1Engine::pauseMillis(uint32 millis) {
	g_system->delayMillis(millis);
}

Graphics::Font *Ultima1Engine::createFont() const {
	return new Gfx::Font();
}

} // namespace Ultima1
} // namespace Ultima
