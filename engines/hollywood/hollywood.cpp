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

#include "hollywood/hollywood.h"
#include "hollywood/console.h"
#include "hollywood/font.h"
#include "hollywood/gameplay/travel_screen.h"
#include "hollywood/scenes/scene_registry.h"
#include "hollywood/scenes/intro/scene9000.h"
#include "hollywood/scenes/intro/scene9010.h"
#include "hollywood/scenes/intro/scene9030.h"
#include "hollywood/scenes/intro/scene9050.h"
#include "hollywood/scenes/intro/scene9100.h"
#include "hollywood/scenes/intro/scene9110.h"
#include "hollywood/scenes/intro/scene9120.h"
#include "hollywood/resource.h"

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/random.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "graphics/pixelformat.h"
#include "graphics/thumbnail.h"

namespace Hollywood {

const uint kOptionsMaximumLevel = 200;
const int kMaximumConfigVolume = Audio::Mixer::kMaxMixerVolume;
const int kMaximumTalkSpeed = 255;
const int kTravelScreenSelectionState = 0xffff;

void initializeRandomizedNewGameState(GameplayState &state) {
	Common::RandomSource randomSource("hollywood_new_game");
	state.scene2080DialogueBranchAIndex = (byte)randomSource.getRandomNumber(2);
	state.scene2080DialogueBranchBIndex = (byte)randomSource.getRandomNumber(2);
	state.scene2080DialogueTerminalIndex = (byte)randomSource.getRandomNumber(2);
}

bool isImplementedIntroSceneNumber(int sceneNumber) {
	return sceneNumber == 9000 || sceneNumber == 9010 || sceneNumber == 9030 || sceneNumber == 9050 ||
		sceneNumber == 9100 || sceneNumber == 9110 || sceneNumber == 9120;
}

bool shouldRunSceneForBootParam(int bootParam, int sceneNumber, bool &bootSceneReached) {
	if (bootParam == 0 || bootSceneReached)
		return true;

	if (bootParam == sceneNumber) {
		bootSceneReached = true;
		return true;
	}

	return false;
}

void prepareGameplayStateForBootParam(GameplayState &state, int bootParam) {
	state.initializeRonItemResourcePages();
	state.initializeRonInventoryItems();

	switch (bootParam) {
	case 2070:
		// Direct B07 boots bypass the chamber/maze setup. Match the normal
		// path where Ron solved the seal puzzle and reached the maze exit.
		state.egyptSealPuzzleProgress = 1;
		state.egyptLabyrinthPositionIndex = 0x47;
		state.scene2050LabyrinthLampReady = true;
		break;
	case 2080:
	case 2090:
	case 2100:
	case 2110:
		state.egyptSealPuzzleProgress = 2;
		state.egyptLabyrinthPositionIndex = 0x47;
		state.scene2050LabyrinthLampReady = true;
		state.scene2070EntryProgress = 2;
		state.scene2070SealExitPatchState = 1;
		state.scene2070InnerPassagePatchState = 1;
		state.scene2080ForegroundState = 0;
		state.scene2090EntryLineSeen = true;
		break;
	case 5130:
		state.scene1070SpencerCocktailRecipeLearned = true;
		state.scene5110SalonTransformState = 2;
		break;
	case 9140:
		state.scene9140VariantIndex = 0;
		state.scene9140ReturnStateId = 1000;
		break;
	default:
		break;
	}
}

byte configVolumeToOptionsLevel(int volume) {
	const int clippedVolume = CLIP<int>(volume, 0, kMaximumConfigVolume);
	return (byte)((clippedVolume * kOptionsMaximumLevel +
		kMaximumConfigVolume / 2) / kMaximumConfigVolume);
}

int optionsLevelToConfigVolume(byte level) {
	const uint clippedLevel = MIN<uint>(level, kOptionsMaximumLevel);
	return (clippedLevel * kMaximumConfigVolume +
		kOptionsMaximumLevel / 2) / kOptionsMaximumLevel;
}

byte configTalkSpeedToOptionsLevel(int talkSpeed) {
	const int clippedTalkSpeed = CLIP<int>(talkSpeed, 0, kMaximumTalkSpeed);
	return (byte)((clippedTalkSpeed * kOptionsMaximumLevel +
		kMaximumTalkSpeed / 2) / kMaximumTalkSpeed);
}

int optionsLevelToConfigTalkSpeed(byte level) {
	const uint clippedLevel = MIN<uint>(level, kOptionsMaximumLevel);
	return (clippedLevel * kMaximumTalkSpeed +
		kOptionsMaximumLevel / 2) / kOptionsMaximumLevel;
}

HollywoodEngine::HollywoodEngine(OSystem *syst, const ADGameDescription *gameDesc) :
		Engine(syst),
		_gameDescription(gameDesc),
		_resources(new ResourceManager()),
		_font(new HollywoodFont()),
		_introMusic(),
		_gameplayMusic(),
		_gameState(),
		_lastGameplayThumbnail(),
		_lastGameplayThumbnailValid(false),
		_sceneRestartRequested(false),
		_canSave(false) {
}

HollywoodEngine::~HollywoodEngine() {
	_lastGameplayThumbnail.free();
	delete _font;
	delete _resources;
}

void HollywoodEngine::captureLastGameplayThumbnail() {
	Graphics::Surface thumbnail;
	if (!Graphics::createThumbnail(thumbnail))
		return;

	_lastGameplayThumbnail.copyFrom(thumbnail);
	_lastGameplayThumbnailValid = true;
	thumbnail.free();
}

bool HollywoodEngine::copyLastGameplayThumbnail(Graphics::Surface &thumbnail) const {
	if (!_lastGameplayThumbnailValid || !_lastGameplayThumbnail.getPixels())
		return false;

	thumbnail.copyFrom(_lastGameplayThumbnail);
	return true;
}

bool HollywoodEngine::subtitlesEnabled() const {
	return _gameState.actorSpeechTextMode != 0;
}

bool HollywoodEngine::restoredContentEnabled() const {
	return ConfMan.getBool("restored_content");
}

Common::Error HollywoodEngine::run() {
	setDebugger(new Console(this));

	const Graphics::PixelFormat screenFormat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(kScreenWidth, kScreenHeight, &screenFormat);

	syncSoundSettings();
	_font->load();

	debugC(1, kDebugGeneral, "Hollywood Monsters engine initialized");

	const int startupLoadSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
	const bool startupLoad = startupLoadSlot >= 0;
	if (!startupLoad)
		initializeRandomizedNewGameState(_gameState);
	if (startupLoad) {
		Common::Error loadError = loadGameState(startupLoadSlot);
		if (loadError.getCode() != Common::kNoError)
			return loadError;
		clearSceneRestartRequest();
		debugC(1, kDebugGeneral, "Loaded startup save slot %d", startupLoadSlot);
	}

	const int bootParam = startupLoad ? 0 : ConfMan.getInt("boot_param");
	const bool bootToIntroScene = isImplementedIntroSceneNumber(bootParam);
	const bool bootToGameplayScene = isImplementedGameplayState(bootParam);
	bool bootSceneReached = bootParam == 0;

	if (bootParam != 0) {
		if (bootToIntroScene || bootToGameplayScene) {
			debugC(1, kDebugGeneral, "Boot parameter requested scene/state %d", bootParam);
		} else {
			warning("Unsupported Hollywood boot parameter %d; starting normally", bootParam);
			bootSceneReached = true;
		}
	}

	if (!startupLoad && !bootToGameplayScene) {
		if (shouldRunSceneForBootParam(bootParam, 9000, bootSceneReached)) {
			Scene9000 scene9000(this);
			if (!scene9000.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9010, bootSceneReached)) {
			Scene9010 scene9010(this);
			if (!scene9010.play())
				return Common::kReadingFailed;

			if (!scene9010.skipRequested() &&
					shouldRunSceneForBootParam(bootParam, 9030, bootSceneReached)) {
				Scene9030 scene9030(this, &scene9010.transitionFramebuffer(), &scene9010.transitionPalette());
				if (!scene9030.play())
					return Common::kReadingFailed;
			}
		} else if (shouldRunSceneForBootParam(bootParam, 9030, bootSceneReached)) {
			Scene9030 scene9030(this);
			if (!scene9030.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9100, bootSceneReached)) {
			Scene9100 scene9100(this);
			if (!scene9100.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9050, bootSceneReached)) {
			Scene9050 scene9050(this);
			if (!scene9050.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9120, bootSceneReached)) {
			Scene9120 scene9120(this);
			if (!scene9120.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9110, bootSceneReached)) {
			Scene9110 scene9110(this);
			if (!scene9110.play())
				return Common::kReadingFailed;
		}
	}

	if (!startupLoad) {
		if (runGameplayScene(this, 7000) != kGameplaySceneCompleted)
			return Common::kReadingFailed;
		if (bootToGameplayScene && bootParam != 7000) {
			gameState().mainFlowStateId = (uint16)gameplayStateForBootParam(bootParam);
			prepareGameplayStateForBootParam(gameState(), bootParam);
		}
	}

	bool handledState = true;
	while (!Engine::shouldQuit() && handledState) {
		handledState = false;
		clearSceneRestartRequest();
		const uint16 stateId = gameState().mainFlowStateId;

		if (stateId == kTravelScreenSelectionState) {
			handledState = true;
			TravelScreen travelScreen(this);
			uint16 selectedStateId = stateId;
			const byte currentChapterId = gameState().travelScreenCurrentChapterId != 0 ?
				gameState().travelScreenCurrentChapterId : 1;
			if (!travelScreen.runSelection(currentChapterId, selectedStateId))
				return Common::kReadingFailed;
			if (!Engine::shouldQuit() && !isSceneRestartRequested()) {
				gameState().mainFlowStateId = selectedStateId;
				gameState().travelScreenCurrentChapterId = 0;
			}
			continue;
		}

		const GameplaySceneResult sceneResult = runGameplayScene(this, stateId);
		if (sceneResult == kGameplaySceneFailed)
			return Common::kReadingFailed;
		if (sceneResult == kGameplaySceneCompleted) {
			handledState = true;
			continue;
		}
	}

	debugC(1, kDebugGeneral, "Gameplay preview completed; current main-flow state is 0x%04x", gameState().mainFlowStateId);
	return Common::kNoError;
}

bool HollywoodEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher ||
		f == kSupportsSubtitleOptions ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime;
}

void HollywoodEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	const bool globalMute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	const bool musicMuted = ConfMan.getBool("music_mute");
	const bool sfxMuted = ConfMan.getBool("sfx_mute");
	const bool speechMuted = ConfMan.getBool("speech_mute");
	bool subtitlesEnabled = !ConfMan.hasKey("subtitles") || ConfMan.getBool("subtitles");
	if (speechMuted && !subtitlesEnabled) {
		ConfMan.setBool("subtitles", true);
		subtitlesEnabled = true;
	}

	_gameState.musicEnabled = !musicMuted;
	_gameState.soundEffectsEnabled = !sfxMuted;
	_gameState.musicVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("music_volume"));
	_gameState.soundEffectsVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("sfx_volume"));
	_gameState.voiceVolumeLevel = configVolumeToOptionsLevel(ConfMan.getInt("speech_volume"));
	_gameState.speechTextSpeedLevel = configTalkSpeedToOptionsLevel(ConfMan.getInt("talkspeed"));
	if (speechMuted)
		_gameState.actorSpeechTextMode = 2;
	else
		_gameState.actorSpeechTextMode = subtitlesEnabled ? 1 : 0;

	_mixer->muteSoundType(Audio::Mixer::kMusicSoundType, globalMute || musicMuted);
	_mixer->muteSoundType(Audio::Mixer::kSFXSoundType, globalMute || sfxMuted);
	_mixer->muteSoundType(Audio::Mixer::kSpeechSoundType, globalMute || speechMuted);
}

void HollywoodEngine::syncSoundSettingsFromGameState() {
	const GameplayState &state = _gameState;
	ConfMan.setBool("music_mute", !state.musicEnabled);
	ConfMan.setBool("sfx_mute", !state.soundEffectsEnabled);
	ConfMan.setInt("music_volume", optionsLevelToConfigVolume(state.musicVolumeLevel));
	ConfMan.setInt("sfx_volume", optionsLevelToConfigVolume(state.soundEffectsVolumeLevel));
	ConfMan.setInt("speech_volume", optionsLevelToConfigVolume(state.voiceVolumeLevel));
	ConfMan.setInt("talkspeed", optionsLevelToConfigTalkSpeed(state.speechTextSpeedLevel));
	ConfMan.setBool("subtitles", state.actorSpeechTextMode != 0);
	ConfMan.setBool("speech_mute", state.actorSpeechTextMode == 2);

	syncSoundSettings();
}

const char *HollywoodEngine::getGameId() const {
	return _gameDescription->gameId;
}

Common::Language HollywoodEngine::getLanguage() const {
	return _gameDescription->language;
}

Common::Platform HollywoodEngine::getPlatform() const {
	return _gameDescription->platform;
}

} // End of namespace Hollywood
