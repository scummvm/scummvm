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
#include "hollywood/scenes/intro/scene9000.h"
#include "hollywood/scenes/intro/scene9010.h"
#include "hollywood/scenes/intro/scene9050.h"
#include "hollywood/scenes/intro/scene9100.h"
#include "hollywood/scenes/intro/scene9110.h"
#include "hollywood/scenes/intro/scene9120.h"
#include "hollywood/scenes/playable/scene7000.h"
#include "hollywood/scenes/playable/scene7010.h"
#include "hollywood/scenes/playable/scene7020.h"
#include "hollywood/scenes/playable/scene7030.h"
#include "hollywood/scenes/playable/scene7040.h"
#include "hollywood/scenes/playable/scene7050.h"
#include "hollywood/scenes/playable/scene7060.h"
#include "hollywood/scenes/playable/scene7070.h"
#include "hollywood/scenes/playable/scene7080.h"
#include "hollywood/scenes/playable/scene7090.h"
#include "hollywood/scenes/playable/scene7100.h"
#include "hollywood/resource.h"

#include "audio/mixer.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "engines/advancedDetector.h"
#include "engines/util.h"

namespace Hollywood {

const uint kOptionsMaximumLevel = 200;
const int kMaximumConfigVolume = Audio::Mixer::kMaxMixerVolume;
const int kMaximumTalkSpeed = 255;

bool isImplementedIntroSceneNumber(int sceneNumber) {
	return sceneNumber == 9000 || sceneNumber == 9010 || sceneNumber == 9050 ||
		sceneNumber == 9100 || sceneNumber == 9110 || sceneNumber == 9120;
}

bool isImplementedGameplayState(int stateId) {
	return stateId == 7000 ||
		(stateId >= 0x1b62 && stateId <= 0x1b6b) ||
		stateId == 0x1b6c ||
		(stateId >= 0x1b76 && stateId <= 0x1b7f) ||
		(stateId >= 0x1b80 && stateId <= 0x1b89) ||
		stateId == 0x1b8a ||
		(stateId >= 0x1b94 && stateId <= 0x1b9d) ||
		(stateId >= 0x1b9e && stateId <= 0x1ba0) ||
		(stateId >= 0x1ba8 && stateId <= 0x1bb1) ||
		(stateId >= 0x1bb2 && stateId <= 0x1bbb) ||
		(stateId >= 0x1bbc && stateId <= 0x1bc5);
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
		_gameState() {
}

HollywoodEngine::~HollywoodEngine() {
	delete _font;
	delete _resources;
}

Common::Error HollywoodEngine::run() {
	setDebugger(new Console(this));

	initGraphics(kScreenWidth, kScreenHeight);

	syncSoundSettings();
	_font->load();

	debugC(1, kDebugGeneral, "Hollywood Monsters engine initialized");

	const int bootParam = ConfMan.getInt("boot_param");
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

	if (!bootToGameplayScene) {
		if (shouldRunSceneForBootParam(bootParam, 9000, bootSceneReached)) {
			Scene9000 scene9000(this);
			if (!scene9000.play())
				return Common::kReadingFailed;
		}

		if (shouldRunSceneForBootParam(bootParam, 9010, bootSceneReached)) {
			Scene9010 scene9010(this);
			if (!scene9010.play())
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

	Scene7000 scene7000(this);
	if (!scene7000.play())
		return Common::kReadingFailed;
	if (bootToGameplayScene && bootParam != 7000)
		gameState().mainFlowStateId = (uint16)bootParam;

	bool handledState = true;
	while (!Engine::shouldQuit() && handledState) {
		handledState = false;
		const uint16 stateId = gameState().mainFlowStateId;

		if (stateId >= 0x1b62 && stateId <= 0x1b6b) {
			handledState = true;
			Scene7010 scene7010(this);
			if (!scene7010.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == 0x1b6c) {
			handledState = true;
			Scene7020 scene7020(this);
			if (!scene7020.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b76 && stateId <= 0x1b7f) {
			handledState = true;
			Scene7030 scene7030(this);
			if (!scene7030.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b80 && stateId <= 0x1b89) {
			handledState = true;
			Scene7040 scene7040(this);
			if (!scene7040.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId == 0x1b8a) {
			handledState = true;
			Scene7050 scene7050(this);
			if (!scene7050.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b94 && stateId <= 0x1b9d) {
			handledState = true;
			Scene7060 scene7060(this);
			if (!scene7060.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1b9e && stateId <= 0x1ba0) {
			handledState = true;
			Scene7070 scene7070(this);
			if (!scene7070.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1ba8 && stateId <= 0x1bb1) {
			handledState = true;
			Scene7080 scene7080(this);
			if (!scene7080.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1bb2 && stateId <= 0x1bbb) {
			handledState = true;
			Scene7090 scene7090(this);
			if (!scene7090.play())
				return Common::kReadingFailed;
			continue;
		}

		if (stateId >= 0x1bbc && stateId <= 0x1bc5) {
			handledState = true;
			Scene7100 scene7100(this);
			if (!scene7100.play())
				return Common::kReadingFailed;
			continue;
		}
	}

	debugC(1, kDebugGeneral, "Gameplay preview completed; current main-flow state is 0x%04x", gameState().mainFlowStateId);
	return Common::kNoError;
}

bool HollywoodEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsReturnToLauncher || f == kSupportsSubtitleOptions;
}

void HollywoodEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	const bool globalMute = ConfMan.hasKey("mute") && ConfMan.getBool("mute");
	const bool musicMuted = ConfMan.getBool("music_mute");
	const bool sfxMuted = ConfMan.getBool("sfx_mute");
	const bool speechMuted = ConfMan.getBool("speech_mute");
	bool subtitlesEnabled = ConfMan.getBool("subtitles");
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
