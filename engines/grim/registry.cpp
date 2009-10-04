/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 * $URL$
 * $Id$
 *
 */

#include "engines/grim/registry.h"

#include "base/commandLine.h"

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
	_develMode = ConfMan.get("game_devel_mode");
	_dataPath = ConfMan.get("path");
	_savePath = ConfMan.get("savepath");
	_lastSet = ConfMan.get("last_set");
	_musicVolume = ConfMan.get("music_volume");
	_sfxVolume = ConfMan.get("sfx_volume");
	_voiceVolume = ConfMan.get("speech_volume");
	_lastSavedGame = ConfMan.get("last_saved_game");
	_gamma = ConfMan.get("gamma");
	_voiceEffects = ConfMan.get("voice_effects");
	_textSpeed = ConfMan.get("text_speed");
	_speechMode = ConfMan.get("speech_mode");
	_movement = ConfMan.get("movement");
	_joystick = ConfMan.get("joystick");
	_spewOnError = ConfMan.get("spew_on_error");
	_showFps = ConfMan.get("show_fps");
	_softRenderer = ConfMan.get("soft_renderer");
	_fullscreen = ConfMan.get("fullscreen");
	_engineSpeed = ConfMan.get("engine_speed");
}

const char *Registry::get(const char *key, const char *defval) const {
	if (strcasecmp("good_times", key) == 0 || strcasecmp("GrimDeveloper", key) == 0) {
		return _develMode.c_str();
	} else if (strcasecmp("GrimDataDir", key) == 0) {
		return _dataPath.c_str();
	} else if (strcasecmp("savepath", key) == 0) {
		return _savePath.c_str();
	} else if (strcasecmp("GrimLastSet", key) == 0) {
		return _lastSet.c_str();
	} else if (strcasecmp("MusicVolume", key) == 0) {
		return _musicVolume.c_str();
	} else if (strcasecmp("SfxVolume", key) == 0) {
		return _sfxVolume.c_str();
	} else if (strcasecmp("VoiceVolume", key) == 0) {
		return _voiceVolume.c_str();
	} else if (strcasecmp("LastSavedGame", key) == 0) {
		return _lastSavedGame.c_str();
	} else if (strcasecmp("Gamma", key) == 0 || strcasecmp("GammaCorrection", key) == 0) {
		return "";//_gamma.c_str();
	} else if (strcasecmp("VoiceEffects", key) == 0) {
		return _voiceEffects.c_str();
	} else if (strcasecmp("TextSpeed", key) == 0) {
		return _textSpeed.c_str();
	} else if (strcasecmp("TextMode", key) == 0) {
		return _speechMode.c_str();
	} else if (strcasecmp("MovementMode", key) == 0) {
		return _movement.c_str();
	} else if (strcasecmp("JoystickEnabled", key) == 0) {
		return _joystick.c_str();
	} else if (strcasecmp("SpewOnError", key) == 0) {
		return _spewOnError.c_str();
	} else if (strcasecmp("show_fps", key) == 0) {
		return _showFps.c_str();
	} else if (strcasecmp("soft_renderer", key) == 0) {
		return _softRenderer.c_str();
	} else if (strcasecmp("fullscreen", key) == 0) {
		return _fullscreen.c_str();
	} else if (strcasecmp("engine_speed", key) == 0) {
		return _engineSpeed.c_str();
	}

	return defval;
}

void Registry::set(const char *key, const char *val) {
	_dirty = true;
	assert(val);

	if (strcasecmp("good_times", key) == 0 || strcasecmp("GrimDeveloper", key) == 0) {
		_develMode = val;
		return;
	} else if (strcasecmp("GrimDataDir", key) == 0) {
		_dataPath = val;
		return;
	} else if (strcasecmp("savepath", key) == 0) {
		_savePath = val;
		return;
	} else if (strcasecmp("GrimLastSet", key) == 0) {
		_lastSet = val;
		return;
	} else if (strcasecmp("MusicVolume", key) == 0) {
		_musicVolume = val;
		return;
	} else if (strcasecmp("SfxVolume", key) == 0) {
		_sfxVolume = val;
		return;
	} else if (strcasecmp("VoiceVolume", key) == 0) {
		_voiceVolume = val;
		return;
	} else if (strcasecmp("LastSavedGame", key) == 0) {
		_lastSavedGame = val;
		return;
	} else if (strcasecmp("Gamma", key) == 0 || strcasecmp("GammaCorrection", key) == 0) {
		_gamma = "";//val;
		return;
	} else if (strcasecmp("VoiceEffects", key) == 0) {
		_voiceEffects = val;
		return;
	} else if (strcasecmp("TextSpeed", key) == 0) {
		_textSpeed = val;
		return;
	} else if (strcasecmp("TextMode", key) == 0) {
		_speechMode = val;
		return;
	} else if (strcasecmp("MovementMode", key) == 0) {
		_movement = val;
		return;
	} else if (strcasecmp("JoystickEnabled", key) == 0) {
		_joystick = val;
		return;
	} else if (strcasecmp("SpewOnError", key) == 0) {
		_spewOnError = val;
		return;
	} else if (strcasecmp("show_fps", key) == 0) {
		_showFps = val;
		return;
	} else if (strcasecmp("soft_renderer", key) == 0) {
		_softRenderer = val;
		return;
	} else if (strcasecmp("fullscreen", key) == 0) {
		_fullscreen = val;
		return;
	} else if (strcasecmp("engine_speed", key) == 0) {
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
	ConfMan.set("music_volume", _musicVolume);
	ConfMan.set("sfx_volume", _sfxVolume);
	ConfMan.set("speech_volume", _voiceVolume);
	ConfMan.set("last_saved_game", _lastSavedGame);
	ConfMan.set("gamma", _gamma);
	ConfMan.set("speech_effects", _voiceEffects);
	ConfMan.set("text_speed", _textSpeed);
	ConfMan.set("speech_mode", _speechMode);
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

} // end of namespace Grim
