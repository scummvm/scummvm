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

#include "common/config-manager.h"
#include "engines/util.h"
#include "harvester/console.h"
#include "harvester/detection.h"
#include "harvester/fst_player.h"
#include "harvester/media_manager.h"
#include "harvester/resources.h"
#include "harvester/flow.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static bool shouldSkipStartupMoviesForDebug() {
	return ConfMan.hasKey("harvester_debug_skip_startup_movies") &&
		ConfMan.getBool("harvester_debug_skip_startup_movies");
}

} // End of anonymous namespace

HarvesterEngine *g_engine = nullptr;

HarvesterEngine::HarvesterEngine(OSystem *syst, const ADGameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("Harvester") {
	g_engine = this;
}

HarvesterEngine::~HarvesterEngine() {
	delete _media;
	delete _startupScript;
	delete _resources;
	g_engine = nullptr;
}

Common::String HarvesterEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool HarvesterEngine::canLoadGameStateCurrently(Common::U32String *) {
	return _startupScript != nullptr;
}

bool HarvesterEngine::canSaveGameStateCurrently(Common::U32String *) {
	return _startupScript != nullptr && _currentStartupSaveRoomState.valid;
}

RuntimeEntityManager *HarvesterEngine::getRuntimeEntities() const {
	return _media ? _media->getRuntimeEntities() : nullptr;
}

Graphics::Screen *HarvesterEngine::getScreen() const {
	return _media ? _media->getScreen() : nullptr;
}

int HarvesterEngine::getDisplayWidth() const {
	return _media ? _media->getDisplayWidth() : 0;
}

int HarvesterEngine::getDisplayHeight() const {
	return _media ? _media->getDisplayHeight() : 0;
}

Art *HarvesterEngine::getStartupArt() const {
	return _media ? _media->getArt() : nullptr;
}

Text *HarvesterEngine::getStartupText() const {
	return _media ? _media->getText() : nullptr;
}

bool HarvesterEngine::isGoreEnabled() const {
	if (_startupScript)
		return _startupScript->isGoreEnabled();

	return !ConfMan.hasKey("gore") || ConfMan.getBool("gore");
}

int HarvesterEngine::getStartupFxVolumeLevel() const {
	return _startupScript ? _startupScript->getFxVolumeLevel() : 9;
}

int HarvesterEngine::getStartupMusicVolumeLevel() const {
	return _startupScript ? _startupScript->getMusicVolumeLevel() : 9;
}

int HarvesterEngine::getStartupGammaLevel() const {
	return _startupScript ? _startupScript->getGammaLevel() : 0;
}

float HarvesterEngine::getStartupGammaBrightnessScale() const {
	return MediaManager::mapGammaLevelToBrightnessScale(getStartupGammaLevel());
}

void HarvesterEngine::applyStartupMixerLevels() {
	if (_media)
		_media->applyMixerLevels(getStartupFxVolumeLevel(), getStartupMusicVolumeLevel());
}

void HarvesterEngine::setStartupFxVolumeLevel(int level) {
	if (_startupScript)
		_startupScript->setFxVolumeLevel(level);
	applyStartupMixerLevels();
}

void HarvesterEngine::setStartupMusicVolumeLevel(int level) {
	if (_startupScript)
		_startupScript->setMusicVolumeLevel(level);
	applyStartupMixerLevels();
}

void HarvesterEngine::setStartupGammaLevel(int level) {
	if (_startupScript)
		_startupScript->setGammaLevel(level);
}

const Common::String &HarvesterEngine::getStartupMusicPath() const {
	static const Common::String kEmptyMusicPath;
	return _media ? _media->getMusicPath() : kEmptyMusicPath;
}

bool HarvesterEngine::isStartupMusicPlaying() const {
	return _media && _media->isMusicPlaying();
}

bool HarvesterEngine::playStartupMusic(const Common::String &path) {
	if (!_media)
		return false;

	const bool started = _media->playMusic(path);
	if (started)
		applyStartupMixerLevels();
	return started;
}

void HarvesterEngine::pauseStartupMusic(bool paused) {
	if (_media)
		_media->pauseMusic(paused);
}

void HarvesterEngine::stopStartupMusic() {
	if (_media)
		_media->stopMusic();
}

bool HarvesterEngine::executeStartupAudioCommand(const StartupAudioCommand &command) {
	return _media && _media->executeAudioCommand(command);
}

bool HarvesterEngine::playStartupSound(const Common::String &path) {
	return _media && _media->playSound(path);
}

bool HarvesterEngine::playStartupSingleSound(const Common::String &path) {
	return _media && _media->playSingleSound(path);
}

void HarvesterEngine::stopStartupSingleSound() {
	if (_media)
		_media->stopSingleSound();
}

bool HarvesterEngine::isStartupSingleSoundPlaying() const {
	return _media && _media->isSingleSoundPlaying();
}

bool HarvesterEngine::playStartupSpeech(const Common::String &path) {
	return _media && _media->playSpeech(path);
}

void HarvesterEngine::stopStartupSpeech() {
	if (_media)
		_media->stopSpeech();
}

bool HarvesterEngine::isStartupSpeechPlaying() const {
	return _media && _media->isSpeechPlaying();
}

bool HarvesterEngine::loadStartupSound(int slot, const Common::String &path) {
	return _media && _media->loadSound(slot, path);
}

bool HarvesterEngine::playStartupLoadedSound(int slot) {
	return _media && _media->playLoadedSound(slot);
}

bool HarvesterEngine::deleteStartupLoadedSound(int slot) {
	return _media && _media->deleteLoadedSound(slot);
}

void HarvesterEngine::stopStartupSound() {
	if (_media)
		_media->stopSound();
}

bool HarvesterEngine::toggleRoomDebugEnabled() {
	_roomDebugEnabled = !_roomDebugEnabled;
	return _roomDebugEnabled;
}

void HarvesterEngine::setDisplayMode(int width, int height) {
	initGraphics(width, height);
	if (_media)
		_media->resetScreen(width, height);
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
	_media = new MediaManager(*_resources);
	_startupScript = new Script();
	if (!_startupScript->load(*_resources))
		return Common::kReadingFailed;
	applyStartupMixerLevels();

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

	if (!_media->loadArt())
		return Common::kReadingFailed;
	_media->drawWaitFrame();

	if (!_media->loadText())
		return Common::kReadingFailed;

	if (!_media->loadQuickTipsResources())
		return Common::kReadingFailed;

	// If a savegame was selected from the launcher, load it
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot != -1)
		(void)loadGameState(saveSlot);

	Flow startupFlow(*this);
	_activeFlow = &startupFlow;
	if (!startupFlow.load()) {
		_activeFlow = nullptr;
		return Common::kReadingFailed;
	}

	const Common::Error error = startupFlow.run();
	_activeFlow = nullptr;
	return error;
}

bool HarvesterEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsLoadingDuringRuntime) ||
	       (f == kSupportsSavingDuringRuntime) ||
	       (f == kSupportsReturnToLauncher);
}

} // End of namespace Harvester
