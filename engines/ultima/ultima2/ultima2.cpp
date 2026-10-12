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
#include "common/config-manager.h"
#include "common/engine_data.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "audio/softsynth/pcspk.h"
#include "ultima/shared/engine/graphics_options.h"
#include "ultima/ultima2/ultima2.h"
#include "ultima/ultima2/console.h"
#include "ultima/ultima2/metaengine.h"
#include "ultima/ultima2/data/secret_archive.h"
#include "ultima/ultima2/logic/space_logic.h"
#include "ultima/ultima2/views/views.h"
#include "ultima/ultima2/views/map.h"
#include "ultima/ultima2/views/space_map.h"
#include "ultima/ultima2/views/interactions/dead.h"

namespace Ultima {
namespace Ultima2 {

Ultima2Engine *g_engine;

Ultima2Engine::Ultima2Engine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Engine(syst), Shared::Events("ultima2"), _gameDescription(gameDesc) {
	g_engine = this;
	_pcSpeaker = new Audio::PCSpeaker();
}

Ultima2Engine::~Ultima2Engine() {
	delete _pcSpeaker;
}

uint32 Ultima2Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String Ultima2Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

// The graphics renderer mode stored in the configuration, falling back to
// the default for the version of the game if it's missing or stale
static Data::RenderMode configuredRenderMode(bool enhanced) {
	const Data::RenderMode defaultMode = enhanced ? Data::RENDER_EGA : Data::RENDER_CGA;
	int mode = Shared::configuredGraphicsMode(defaultMode);
	return (mode >= 0 && mode < Data::RENDER_MODE_COUNT) ?
		(Data::RenderMode)mode : defaultMode;
}

Common::Error Ultima2Engine::run() {
	MetaEngine::setKeybindingMode(KBMODE_MINIMAL);

	// Initialize 320x200 paletted graphics mode
	initGraphics(320, 200);
	_pcSpeakerReady = _pcSpeaker->init();

	// Set the engine's debugger console
	setDebugger(new Console());

	byte seed = (byte)getRandomNumber(255);
	for (int i = 0; i < 6; ++i)
		_rngState[i] = seed;
	_rngState[0]++;
	_rngState[1]++;

	// Initialise engine data for the game
	Common::U32String errMsg;
	if (!Common::load_engine_data("ultima.dat", "ultima2", 1, 0, errMsg)) {
		GUIErrorMessage(errMsg);
		return Common::kNoError;
	}

	// Restore the player's preferred renderer
	_renderMode = configuredRenderMode((_gameDescription->features & GF_ENHANCED) != 0);

	// Set the palette now that the engine data is available, since the
	// composite renderer's colors come from a file in it
	Data::setPalette();

	// Set up secret map
	SearchMan.add("Secret", new Data::SecretMapArchive());

	// Create all the views and run the game
	Views::Views views;
	addView("Title");
	runGame();
	(void)views;		// Suppress any warnings of unused local

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
	// Only allow saving when any of the different map views are active, or
	// flying in space and not in the middle of a hyperwarp animation
	if (dynamic_cast<Views::Map *>(focusedView()) != nullptr)
		return true;

	auto *spaceMap = dynamic_cast<Views::SpaceMap *>(focusedView());
	return spaceMap != nullptr && !spaceMap->busy();
}

bool Ultima2Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return canSaveGameStateCurrently() ||
		dynamic_cast<Views::Interactions::Dead *>(focusedView()) != nullptr ||
		dynamic_cast<Views::Title *>(focusedView()) != nullptr;
}

Common::Error Ultima2Engine::syncGame(Common::Serializer &s) {
	_savegame.synchronize(s);

	if (s.isLoading()) {
		if (_G(savegame)._inFlight)
			Logic::SpaceLogic::resume();
		else
			_G(map).load(_G(savegame)._mapEra, _G(savegame)._mapType);
	}

	// While flying, there's no current map to save/restore the state of
	if (!_G(savegame)._inFlight)
		_G(map).synchronize(s, _G(savegame)._mapType >= 4);

	// The views were drawn from the freshly loaded map, before its saved state was applied
	if (s.isLoading())
		focusedView()->redraw();

	return Common::kNoError;
}

bool Ultima2Engine::savegamesExist() const {
	Common::String slotName = getSaveStateName(1);
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;

	delete saveFile;
	return result;
}

namespace {

// The original's PIT timer runs at ~1193182Hz; a note's pitch is that
// divided by the divisor value the game hands to the timer chip
constexpr double PIT_BASE_FREQUENCY = 1193182.0;

} // namespace

void Ultima2Engine::queueTone(int divisor, uint32 lengthMs) {
	if (_pcSpeakerReady && divisor > 0)
		_pcSpeaker->playQueue(Audio::PCSpeaker::kWaveFormSquare, PIT_BASE_FREQUENCY / divisor, lengthMs * 1000);
}

void Ultima2Engine::queueSilence(uint32 lengthMs) {
	if (_pcSpeakerReady)
		_pcSpeaker->playQueue(Audio::PCSpeaker::kWaveFormSilence, 0, lengthMs * 1000);
}

void Ultima2Engine::playFX(Data::SoundEffect fx) {
	if (!_pcSpeakerReady)
		return;

	_pcSpeaker->stop();

	switch (fx) {
	case Data::SFX_TICK:
		// A short, unobtrusive click - played every single turn
		queueTone(1280, 8);
		break;

	case Data::SFX_STEP:
		// An even quieter, lower thud for each step taken
		queueTone(0x4000, 12);
		break;

	case Data::SFX_ATTACK:
		// A brief falling whoosh as a weapon swings
		queueTone(1024, 60);
		queueTone(1280, 60);
		queueTone(1535, 80);
		break;

	case Data::SFX_HIT:
		// A short, buzzy warble for a connecting blow
		for (int i = 0; i < 6; ++i)
			queueTone((i & 1) ? 512 : 1400, 20);
		break;

	case Data::SFX_CANNON:
		// A quick descending-pitch blast
		queueTone(768, 40);
		queueTone(1280, 40);
		queueTone(1790, 40);
		break;

	case Data::SFX_TRAP:
		// A longer falling alarm
		for (int i = 0; i < 8; ++i)
			queueTone(256 + i * ((4351 - 256) / 7), 90);
		break;

	case Data::SFX_FAIL:
		// A short, rapidly rising buzz for a failed action
		for (int i = 0; i < 12; ++i)
			queueTone(2048 - i * 160, 15);
		break;

	case Data::SFX_BEEP:
		// A narrow, high-pitched double-blip confirmation chime
		queueTone(760, 60);
		queueTone(800, 60);
		queueTone(760, 60);
		break;

	case Data::SFX_MAGIC:
		// A narrow, softer chime for a successful spell
		queueTone(1170, 60);
		queueTone(1216, 60);
		queueTone(1170, 60);
		break;
	}
}

void Ultima2Engine::applyGameSettings() {
	// The graphics mode can be changed from the in-game options dialog
	setRenderMode(configuredRenderMode((_gameDescription->features & GF_ENHANCED) != 0));
}

void Ultima2Engine::setRenderMode(Data::RenderMode mode) {
	if (mode == _renderMode)
		return;

	_renderMode = mode;
	ConfMan.setInt(Shared::GRAPHICS_MODE_KEY, (int)mode);
	ConfMan.flushToDisk();
	Data::setPalette();

	// OverworldMap/LocationMap are constructed once up front as part of
	// Views::Views, so they need to be explicitly told to re-read their
	// tile graphics now rather than noticing this on their own
	g_engine->send(Graphics::Views::GameMessage("RELOAD_TILES"));
}

void Ultima2Engine::pauseMillis(uint32 millis) {
	g_system->delayMillis(millis);
}

} // namespace Ultima2
} // namespace Ultima
