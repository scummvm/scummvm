/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
 */

#include "engines/grim/registry.h"

#include "audio/mixer.h"

#include "common/config-manager.h"

namespace Grim {

Registry *g_registry = NULL;

// SpewOnError
// GrimLastSet
// JoystickEnabled
// MovementMode
// SpeechMode
// GammaCorrection, Gamma
// GrimDeveloper
// SfxVolume
// MusicVolume
// VoiceVolume
// TextSpeed
// VoiceEffects
// LastSavedGame
// good_times

// show_fps
// soft_renderer
// fullscreen
// engine_speed

Registry::Registry() : _dirty(true) {
	// Default settings for GRIM
	ConfMan.registerDefault("subtitles", true);
	ConfMan.registerDefault("talkspeed", 255);

	// Read settings
	_develMode = ConfMan.get("game_devel_mode");
	_dataPath = ConfMan.get("path");
	_savePath = ConfMan.get("savepath");
	_lastSet = ConfMan.get("last_set");
	_musicVolume = convertVolumeFromMixer(ConfMan.getInt("music_volume"));
	_sfxVolume = convertVolumeFromMixer(ConfMan.getInt("sfx_volume"));
	_voiceVolume = convertVolumeFromMixer(ConfMan.getInt("speech_volume"));
	_lastSavedGame = ConfMan.get("last_saved_game");
	_gamma = ConfMan.get("gamma");
	_voiceEffects = ConfMan.get("voice_effects");
	_textSpeed = convertTalkSpeedFromGUI(ConfMan.getInt("talkspeed"));
	_speechMode = convertSpeechModeFromGUI(ConfMan.getBool("subtitles"), ConfMan.getBool("speech_mute"));
	_movement = ConfMan.get("movement");
	_joystick = ConfMan.get("joystick");
	_spewOnError = ConfMan.get("spew_on_error");
	_showFps = ConfMan.get("show_fps");
	_softRenderer = ConfMan.get("soft_renderer");
	_fullscreen = ConfMan.get("fullscreen");
	_engineSpeed = ConfMan.get("engine_speed");
}

const char *Registry::get(const char *key, const char *defval) const {
	if (scumm_stricmp("good_times", key) == 0 || scumm_stricmp("GrimDeveloper", key) == 0) {
		return _develMode.c_str();
	} else if (scumm_stricmp("GrimDataDir", key) == 0) {
		return _dataPath.c_str();
	} else if (scumm_stricmp("savepath", key) == 0) {
		return _savePath.c_str();
	} else if (scumm_stricmp("GrimLastSet", key) == 0) {
		return _lastSet.c_str();
	} else if (scumm_stricmp("MusicVolume", key) == 0) {
		return _musicVolume.c_str();
	} else if (scumm_stricmp("SfxVolume", key) == 0) {
		return _sfxVolume.c_str();
	} else if (scumm_stricmp("VoiceVolume", key) == 0) {
		return _voiceVolume.c_str();
	} else if (scumm_stricmp("LastSavedGame", key) == 0) {
		return _lastSavedGame.c_str();
	} else if (scumm_stricmp("Gamma", key) == 0 || scumm_stricmp("GammaCorrection", key) == 0) {
		return "";//_gamma.c_str();
	} else if (scumm_stricmp("VoiceEffects", key) == 0) {
		return _voiceEffects.c_str();
	} else if (scumm_stricmp("TextSpeed", key) == 0) {
		return _textSpeed.c_str();
	} else if (scumm_stricmp("TextMode", key) == 0) {
		return _speechMode.c_str();
	} else if (scumm_stricmp("MovementMode", key) == 0) {
		return _movement.c_str();
	} else if (scumm_stricmp("JoystickEnabled", key) == 0) {
		return _joystick.c_str();
	} else if (scumm_stricmp("SpewOnError", key) == 0) {
		return _spewOnError.c_str();
	} else if (scumm_stricmp("show_fps", key) == 0) {
		return _showFps.c_str();
	} else if (scumm_stricmp("soft_renderer", key) == 0) {
		return _softRenderer.c_str();
	} else if (scumm_stricmp("fullscreen", key) == 0) {
		return _fullscreen.c_str();
	} else if (scumm_stricmp("engine_speed", key) == 0) {
		return _engineSpeed.c_str();
	}

	return defval;
}

void Registry::set(const char *key, const char *val) {
	_dirty = true;
	assert(val);

	if (scumm_stricmp("good_times", key) == 0 || scumm_stricmp("GrimDeveloper", key) == 0) {
		_develMode = val;
		return;
	} else if (scumm_stricmp("GrimDataDir", key) == 0) {
		_dataPath = val;
		return;
	} else if (scumm_stricmp("savepath", key) == 0) {
		_savePath = val;
		return;
	} else if (scumm_stricmp("GrimLastSet", key) == 0) {
		_lastSet = val;
		return;
	} else if (scumm_stricmp("MusicVolume", key) == 0) {
		_musicVolume = val;
		return;
	} else if (scumm_stricmp("SfxVolume", key) == 0) {
		_sfxVolume = val;
		return;
	} else if (scumm_stricmp("VoiceVolume", key) == 0) {
		_voiceVolume = val;
		return;
	} else if (scumm_stricmp("LastSavedGame", key) == 0) {
		_lastSavedGame = val;
		return;
	} else if (scumm_stricmp("Gamma", key) == 0 || scumm_stricmp("GammaCorrection", key) == 0) {
		_gamma = "";//val;
		return;
	} else if (scumm_stricmp("VoiceEffects", key) == 0) {
		_voiceEffects = val;
		return;
	} else if (scumm_stricmp("TextSpeed", key) == 0) {
		_textSpeed = val;
		return;
	} else if (scumm_stricmp("TextMode", key) == 0) {
		_speechMode = val;
		return;
	} else if (scumm_stricmp("MovementMode", key) == 0) {
		_movement = val;
		return;
	} else if (scumm_stricmp("JoystickEnabled", key) == 0) {
		_joystick = val;
		return;
	} else if (scumm_stricmp("SpewOnError", key) == 0) {
		_spewOnError = val;
		return;
	} else if (scumm_stricmp("show_fps", key) == 0) {
		_showFps = val;
		return;
	} else if (scumm_stricmp("soft_renderer", key) == 0) {
		_softRenderer = val;
		return;
	} else if (scumm_stricmp("fullscreen", key) == 0) {
		_fullscreen = val;
		return;
	} else if (scumm_stricmp("engine_speed", key) == 0) {
		_engineSpeed = val;
		return;
	}
}

void Registry::save() {
	if (!_dirty)
		return;

	ConfMan.set("game_devel_mode", _develMode);
	ConfMan.set("path", _dataPath);
	ConfMan.set("savepath", _savePath);
	ConfMan.set("last_set", _lastSet);
	ConfMan.setInt("music_volume", convertVolumeToMixer(_musicVolume));
	ConfMan.setInt("sfx_volume", convertVolumeToMixer(_sfxVolume));
	ConfMan.setInt("speech_volume", convertVolumeToMixer(_voiceVolume));
	ConfMan.set("last_saved_game", _lastSavedGame);
	ConfMan.set("gamma", _gamma);
	ConfMan.set("speech_effects", _voiceEffects);
	ConfMan.setInt("talkspeed", convertTalkSpeedToGUI(_textSpeed));
	ConfMan.setBool("subtitles", convertSubtitlesToGUI(_speechMode));
	ConfMan.setBool("speech_mute", convertSpeechMuteToGUI(_speechMode));
	ConfMan.set("movement", _movement);
	ConfMan.set("joystick", _joystick);
	ConfMan.set("spew_on_error", _spewOnError);
	ConfMan.set("show_fps", _showFps);
	ConfMan.set("soft_renderer", _softRenderer);
	ConfMan.set("fullscreen", _fullscreen);
	ConfMan.set("engine_speed", _engineSpeed);

	ConfMan.flushToDisk();

	_dirty = false;
}

uint Registry::convertVolumeToMixer(const Common::String &grimVolume) {
	return CLIP<uint>(atoi(grimVolume.c_str()) * 2, 0, Audio::Mixer::kMaxMixerVolume);
}

Common::String Registry::convertVolumeFromMixer(uint volume) {
	return Common::String::format("%d", CLIP<uint>(volume / 2, 0, 127));
}

uint Registry::convertTalkSpeedToGUI(const Common::String &talkspeed) {
	return CLIP<uint>(atoi(talkspeed.c_str()) * 255 / 10, 0, 255);
}

Common::String Registry::convertTalkSpeedFromGUI(uint talkspeed) {
	return Common::String::format("%d", CLIP<uint>(talkspeed * 10 / 255, 1, 10));
}

bool Registry::convertSubtitlesToGUI(const Common::String &speechmode) {
	int mode = atoi(speechmode.c_str());
	return mode == 1 || mode == 3;
}

bool Registry::convertSpeechMuteToGUI(const Common::String &speechmode) {
	int mode = atoi(speechmode.c_str());
	return mode == 1;
}

Common::String Registry::convertSpeechModeFromGUI(bool subtitles, bool speechMute) {
	if (!subtitles && !speechMute) // Speech only
		return "2";
	else if (subtitles && !speechMute) // Speech and subtitles
		return "3";
	else if (subtitles && speechMute) // Subtitles only
		return "1";
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return "1";
}

} // end of namespace Grim
