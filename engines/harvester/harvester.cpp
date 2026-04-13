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

static bool shouldSkipIntroMoviesForDebug() {
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
	delete _script;
	delete _resources;
	g_engine = nullptr;
}

Common::String HarvesterEngine::getGameId() const {
	return _gameDescription->gameId;
}

bool HarvesterEngine::canLoadGameStateCurrently(Common::U32String *) {
	return _script != nullptr;
}

bool HarvesterEngine::canSaveGameStateCurrently(Common::U32String *) {
	return _script != nullptr && _currentSaveRoomState.valid;
}

EntityManager *HarvesterEngine::getRuntimeEntities() const {
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

Art *HarvesterEngine::getArt() const {
	return _media ? _media->getArt() : nullptr;
}

Text *HarvesterEngine::getText() const {
	return _media ? _media->getText() : nullptr;
}

bool HarvesterEngine::isGoreEnabled() const {
	if (_script)
		return _script->isGoreEnabled();

	return !ConfMan.hasKey("gore") || ConfMan.getBool("gore");
}

bool HarvesterEngine::shouldShowCdChangePrompts() const {
	return ConfMan.hasKey("show_cd_change_prompts") && ConfMan.getBool("show_cd_change_prompts");
}

int HarvesterEngine::getFxVolumeLevel() const {
	return _script ? _script->getFxVolumeLevel() : 9;
}

int HarvesterEngine::getMusicVolumeLevel() const {
	return _script ? _script->getMusicVolumeLevel() : 9;
}

int HarvesterEngine::getGammaLevel() const {
	return _script ? _script->getGammaLevel() : 0;
}

float HarvesterEngine::getGammaBrightnessScale() const {
	return MediaManager::mapGammaLevelToBrightnessScale(getGammaLevel());
}

void HarvesterEngine::applyMixerLevels() {
	if (_media)
		_media->applyMixerLevels(getFxVolumeLevel(), getMusicVolumeLevel());
}

void HarvesterEngine::setFxVolumeLevel(int level) {
	if (_script)
		_script->setFxVolumeLevel(level);
	applyMixerLevels();
}

void HarvesterEngine::setMusicVolumeLevel(int level) {
	if (_script)
		_script->setMusicVolumeLevel(level);
	applyMixerLevels();
}

void HarvesterEngine::setGammaLevel(int level) {
	if (_script)
		_script->setGammaLevel(level);
}

const Common::String &HarvesterEngine::getMusicPath() const {
	return _media ? _media->getMusicPath() : _emptyMusicPath;
}

bool HarvesterEngine::isMusicPlaying() const {
	return _media && _media->isMusicPlaying();
}

bool HarvesterEngine::playMusic(const Common::String &path) {
	if (!_media)
		return false;

	const bool started = _media->playMusic(path);
	if (started)
		applyMixerLevels();
	return started;
}

void HarvesterEngine::pauseMusic(bool paused) {
	if (_media)
		_media->pauseMusic(paused);
}

void HarvesterEngine::stopMusic() {
	if (_media)
		_media->stopMusic();
}

bool HarvesterEngine::executeAudioCommand(const AudioCommand &command) {
	return _media && _media->executeAudioCommand(command);
}

bool HarvesterEngine::playSound(const Common::String &path) {
	return _media && _media->playSound(path);
}

bool HarvesterEngine::playSingleSound(const Common::String &path) {
	return _media && _media->playSingleSound(path);
}

void HarvesterEngine::stopSingleSound() {
	if (_media)
		_media->stopSingleSound();
}

bool HarvesterEngine::isSingleSoundPlaying() const {
	return _media && _media->isSingleSoundPlaying();
}

bool HarvesterEngine::playSpeech(const Common::String &path) {
	return _media && _media->playSpeech(path);
}

void HarvesterEngine::stopSpeech() {
	if (_media)
		_media->stopSpeech();
}

bool HarvesterEngine::isSpeechPlaying() const {
	return _media && _media->isSpeechPlaying();
}

bool HarvesterEngine::loadSound(int slot, const Common::String &path) {
	return _media && _media->loadSound(slot, path);
}

bool HarvesterEngine::playLoadedSound(int slot) {
	return _media && _media->playLoadedSound(slot);
}

bool HarvesterEngine::deleteLoadedSound(int slot) {
	return _media && _media->deleteLoadedSound(slot);
}

void HarvesterEngine::stopSound() {
	if (_media)
		_media->stopSound();
}

bool HarvesterEngine::activateDisc(int discNumber) {
	if (!_resources || discNumber <= 0)
		return false;

	const int previousDisc = _resources->getCurrentDisc();
	const bool discChanged = previousDisc > 0 && previousDisc != discNumber;
	if (_media && discChanged) {
		// Loaded audio streams can still be backed by the currently mounted archive.
		// Stop them before the resource manager swaps the active disc out.
		_media->stopMusic();
		_media->stopSound();
	}

	if (!_resources->setCurrentDisc(discNumber))
		return false;

	if (_media && discChanged) {
		if (!_media->loadText()) {
			warning("Harvester: unable to reload text resources after disc switch %d -> %d",
				previousDisc, discNumber);
			return false;
		}

		debugC(1, kDebugResources,
			"Harvester: flushed transient startup media state and reloaded text after disc switch %d -> %d",
			previousDisc, discNumber);
	}

	return true;
}

bool HarvesterEngine::toggleCombatDebugEnabled() {
	_combatDebugEnabled = !_combatDebugEnabled;
	return _combatDebugEnabled;
}

bool HarvesterEngine::toggleRoomDebugEnabled() {
	_roomDebugEnabled = !_roomDebugEnabled;
	return _roomDebugEnabled;
}

bool HarvesterEngine::togglePathfindingDebugEnabled() {
	_pathfindingDebugEnabled = !_pathfindingDebugEnabled;
	return _pathfindingDebugEnabled;
}

bool HarvesterEngine::toggleTimerDebugEnabled() {
	_timerDebugEnabled = !_timerDebugEnabled;
	return _timerDebugEnabled;
}

bool HarvesterEngine::requestDebugCommand(const CommandRecord &command) {
	if (!_activeFlow || !_script || !_currentSaveRoomState.valid)
		return false;
	if (_activeFlow->hasQueuedDebugInteraction())
		return false;

	InteractionResult interaction;
	if (!_script->executeDebugCommand(command, interaction))
		return false;

	// Defer application to the active room loop instead of mutating state from the console thread.
	return _activeFlow->queueDebugInteraction(interaction);
}

bool HarvesterEngine::requestDebugRoomChange(const Common::String &roomName) {
	return _activeFlow &&
		_currentSaveRoomState.valid &&
		_activeFlow->requestDebugRoomChange(roomName);
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
	_script = new Script();
	if (!_script->load(*_resources))
		return Common::kReadingFailed;
	applyMixerLevels();

	// The intro FST files play on the narrower startup movie surface.
	setDisplayMode(320, 200);

	// Set the engine's debugger console
	setDebugger(new Console());

	if (shouldSkipIntroMoviesForDebug()) {
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

	Flow flow(*this);
	_activeFlow = &flow;
	if (!flow.load()) {
		_activeFlow = nullptr;
		return Common::kReadingFailed;
	}

	const Common::Error error = flow.run();
	_activeFlow = nullptr;
	return error;
}

bool HarvesterEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsLoadingDuringRuntime) ||
	       (f == kSupportsSavingDuringRuntime) ||
	       (f == kSupportsReturnToLauncher);
}

} // End of namespace Harvester
