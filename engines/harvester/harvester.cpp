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

#include "harvester/harvester.h"

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/config-manager.h"
#include "common/system.h"
#include "engines/util.h"
#include "graphics/paletteman.h"
#include "harvester/console.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/resources.h"
#include "harvester/runtime_entity.h"
#include "harvester/startup_art.h"
#include "harvester/startup_flow.h"
#include "harvester/startup_script.h"
#include "harvester/startup_text.h"

namespace Harvester {

HarvesterEngine *g_engine = nullptr;

HarvesterEngine::HarvesterEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Harvester") {
	g_engine = this;
}

HarvesterEngine::~HarvesterEngine() {
	stopStartupSound();
	delete _startupText;
	delete _startupArt;
	delete _startupScript;
	delete _runtimeEntities;
	delete _resources;
	delete _screen;
	g_engine = nullptr;
}

Common::String HarvesterEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool HarvesterEngine::isGoreEnabled() const {
	return !ConfMan.hasKey("gore") || ConfMan.getBool("gore");
}

bool HarvesterEngine::playStartupSound(const Common::String &path) {
	stopStartupSound();
	if (path.empty() || !_resources)
		return false;

	Common::SeekableReadStream *stream = _resources->openFile(path);
	if (!stream) {
		warning("Harvester: unable to load startup sound '%s'", path.c_str());
		return false;
	}

	Audio::SeekableAudioStream *audioStream = Audio::makeWAVStream(stream, DisposeAfterUse::YES);
	if (!audioStream) {
		warning("Harvester: unable to decode startup sound '%s'", path.c_str());
		return false;
	}

	g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &_startupSoundHandle, audioStream);
	return true;
}

void HarvesterEngine::stopStartupSound() {
	if (g_system && g_system->getMixer())
		g_system->getMixer()->stopHandle(_startupSoundHandle);
}

void HarvesterEngine::setDisplayMode(int width, int height) {
	delete _screen;
	_screen = nullptr;

	initGraphics(width, height);
	_screen = new Graphics::Screen();
	debugC(1, kDebugGeneral, "Harvester: switched display mode to %dx%d", width, height);
}

Common::Error HarvesterEngine::run() {
	static const char *const kIntroPaths[] = {
		"GRAPHIC/FST/VIRGLOGO.FST",
		"GRAPHIC/FST/FVLOGO.FST",
		"GRAPHIC/FST/INTROFIN.FST"
	};
	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(320, 200));
	modes.push_back(Graphics::Mode(640, 480));
	initGraphicsModes(modes);

	_resources = new ResourceManager();
	_resources->mountStartupArchives();
	_runtimeEntities = new RuntimeEntityManager(*_resources);
	_startupScript = new StartupScript();
	if (!_startupScript->load(*_resources))
		return Common::kReadingFailed;

	// The intro FST files play on the narrower startup movie surface.
	setDisplayMode(320, 200);

	// Set the engine's debugger console
	setDebugger(new Console());

	FstPlayer fstPlayer(*this);
	for (const char *path : kIntroPaths) {
		if (!fstPlayer.play(path))
			return Common::kReadingFailed;
	}

	// The original executable switches back to the gameplay UI surface after INTROFIN.FST.
	setDisplayMode(640, 480);

	_startupArt = new StartupArt();
	if (!_startupArt->load(*_resources))
		return Common::kReadingFailed;
	_startupArt->drawWaitFrame();

	_startupText = new StartupText();
	if (!_startupText->load(*_resources))
		return Common::kReadingFailed;

	if (!_startupArt->loadQuickTipsResources(*_resources))
		return Common::kReadingFailed;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	StartupFlow startupFlow(*this);
	if (!startupFlow.load())
		return Common::kReadingFailed;

	return startupFlow.run();
}

bool HarvesterEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsLoadingDuringRuntime) ||
	       (f == kSupportsSavingDuringRuntime) ||
	       (f == kSupportsReturnToLauncher);
}

Common::Error HarvesterEngine::syncGame(Common::Serializer &s) {
	// The Serializer has methods isLoading() and isSaving()
	// if you need to specific steps; for example setting
	// an array size after reading it's length, whereas
	// for saving it would write the existing array's length
	uint32 dummy = 0;
	s.syncAsUint32LE(dummy);

	return Common::kNoError;
}

Common::Error HarvesterEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	Common::Serializer serializer(nullptr, stream);
	return syncGame(serializer);
}

Common::Error HarvesterEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer serializer(stream, nullptr);
	return syncGame(serializer);
}

} // End of namespace Harvester
