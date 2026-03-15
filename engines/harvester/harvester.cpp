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

#include <cstring>

#include "harvester/harvester.h"

#include "audio/audiostream.h"
#include "audio/decoders/adpcm.h"
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

namespace {

static bool shouldSkipStartupMoviesForDebug() {
	return ConfMan.hasKey("harvester_debug_skip_startup_movies") &&
		ConfMan.getBool("harvester_debug_skip_startup_movies");
}

static Audio::SeekableAudioStream *openStartupAudioStream(ResourceManager &resources, const Common::String &path) {
	Common::SeekableReadStream *stream = resources.openFile(path);
	if (!stream)
		return nullptr;

	if (stream->size() >= 14) {
		char magic[4];
		stream->read(magic, sizeof(magic));
		if (!memcmp(magic, "FCMP", sizeof(magic))) {
			const uint32 payloadSizeFromHeader = stream->readUint32LE();
			const uint32 sampleRate = stream->readUint32LE();
			const uint16 bitsPerSample = stream->readUint16LE();
			const uint32 availablePayloadSize = stream->size() - 14;
			const uint32 payloadSize = payloadSizeFromHeader != 0
				? MIN<uint32>(payloadSizeFromHeader, availablePayloadSize)
				: availablePayloadSize;

			if (sampleRate == 0 || bitsPerSample != 16 || payloadSize == 0) {
				delete stream;
				return nullptr;
			}

			return Audio::makeADPCMStream(stream, DisposeAfterUse::YES, payloadSize,
				Audio::kADPCMDVI, sampleRate, 1);
		}
	}

	stream->seek(0);
	return Audio::makeWAVStream(stream, DisposeAfterUse::YES);
}

} // End of anonymous namespace

HarvesterEngine *g_engine = nullptr;

HarvesterEngine::HarvesterEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Harvester") {
	g_engine = this;
}

HarvesterEngine::~HarvesterEngine() {
	stopStartupMusic();
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

bool HarvesterEngine::playStartupMusic(const Common::String &path) {
	if (path.empty() || !_resources || !g_system || !g_system->getMixer())
		return false;

	const Common::String normalizedPath = _resources->normalizeResourcePath(path);
	if (_startupMusicPath.equalsIgnoreCase(normalizedPath) &&
			g_system->getMixer()->isSoundHandleActive(_startupMusicHandle)) {
		return true;
	}

	Audio::SeekableAudioStream *audioStream = openStartupAudioStream(*_resources, path);
	if (!audioStream) {
		warning("Harvester: unable to decode startup music '%s'", path.c_str());
		return false;
	}

	stopStartupMusic();
	g_system->getMixer()->playStream(Audio::Mixer::kMusicSoundType, &_startupMusicHandle,
		Audio::makeLoopingAudioStream(audioStream, 0));
	_startupMusicPath = normalizedPath;
	return true;
}

void HarvesterEngine::stopStartupMusic() {
	if (g_system && g_system->getMixer())
		g_system->getMixer()->stopHandle(_startupMusicHandle);
	_startupMusicPath.clear();
}

bool HarvesterEngine::playStartupSound(const Common::String &path) {
	stopStartupSound();
	if (path.empty() || !_resources || !g_system || !g_system->getMixer())
		return false;

	Audio::SeekableAudioStream *audioStream = openStartupAudioStream(*_resources, path);
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

	if (shouldSkipStartupMoviesForDebug()) {
		debugC(1, kDebugGeneral, "Harvester: debug skip enabled for startup intro FST playback");
	} else {
		FstPlayer fstPlayer(*this);
		for (const char *path : kIntroPaths) {
			if (!fstPlayer.play(path))
				return Common::kReadingFailed;
		}
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
