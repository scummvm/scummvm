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
#include "hollywood/scenes/playable/scene7030.h"
#include "hollywood/scenes/playable/scene7040.h"
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
	Scene9000 scene9000(this);
	if (!scene9000.play())
		return Common::kReadingFailed;

	Scene9010 scene9010(this);
	if (!scene9010.play())
		return Common::kReadingFailed;

	Scene9100 scene9100(this);
	if (!scene9100.play())
		return Common::kReadingFailed;

	Scene9050 scene9050(this);
	if (!scene9050.play())
		return Common::kReadingFailed;

	Scene9120 scene9120(this);
	if (!scene9120.play())
		return Common::kReadingFailed;

	Scene9110 scene9110(this);
	if (!scene9110.play())
		return Common::kReadingFailed;

	Scene7000 scene7000(this);
	if (!scene7000.play())
		return Common::kReadingFailed;

	if (gameState().mainFlowStateId == 0x1b62) {
		Scene7010 scene7010(this);
		if (!scene7010.play())
			return Common::kReadingFailed;
	}

	if (gameState().mainFlowStateId == 0x1b76 || gameState().mainFlowStateId == 0x1b77) {
		Scene7030 scene7030(this);
		if (!scene7030.play())
			return Common::kReadingFailed;
	}

	if (gameState().mainFlowStateId >= 0x1b80 && gameState().mainFlowStateId <= 0x1b82) {
		Scene7040 scene7040(this);
		if (!scene7040.play())
			return Common::kReadingFailed;
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
