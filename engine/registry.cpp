/* Residual - Virtual machine to run LucasArts' 3D adventure games
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

#include "common/sys.h"
#include "common/debug.h"
#include "common/str.h"

#include "engine/registry.h"
#include "engine/cmd_line.h"

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

// show-fps
// gl-zbuffer
// soft-renderer
// fullscreen

Registry::Registry() : _dirty(true) {
	_develMode = ConfMan.get("game-devel-mode");
	_dataPath = ConfMan.get("path");
	_lastSet = ConfMan.get("last-set");
	_musicVolume = ConfMan.get("music-volume");
	_sfxVolume = ConfMan.get("sfx-volume");
	_voiceVolume = ConfMan.get("voice-volume");
	_lastSavedGame = ConfMan.get("last-saved-game");
	_gamma = ConfMan.get("gamma");
	_voiceEffects = ConfMan.get("voice-effects");
	_textSpeed = ConfMan.get("text-speed");
	_speechMode = ConfMan.get("speech-mode");
	_movement = ConfMan.get("movement");
	_joystick = ConfMan.get("joystick");
	_spewOnError = ConfMan.get("spew-on-error");
	_showFps = ConfMan.get("show-fps");
	_softRenderer = ConfMan.get("soft-renderer");
	_glZbuffer = ConfMan.get("gl-zbuffer");
	_fullscreen = ConfMan.get("fullscreen");
}

const char *Registry::get(const char *key, const char *defval) const {
	if (strcasecmp("good_times", key) == 0 || strcasecmp("GrimDeveloper", key) == 0) {
		return _develMode.c_str();
	} else if (strcasecmp("GrimDataDir", key) == 0) {
		return _dataPath.c_str();
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
	} else if (strcasecmp("SpeechMode", key) == 0) {
		return _speechMode.c_str();
	} else if (strcasecmp("MovementMode", key) == 0) {
		return _movement.c_str();
	} else if (strcasecmp("JoystickEnabled", key) == 0) {
		return _joystick.c_str();
	} else if (strcasecmp("SpewOnError", key) == 0) {
		return _spewOnError.c_str();
	} else if (strcasecmp("show-fps", key) == 0) {
		return _showFps.c_str();
	} else if (strcasecmp("soft-renderer", key) == 0) {
		return _softRenderer.c_str();
	} else if (strcasecmp("gl-zbuffer", key) == 0) {
		return _glZbuffer.c_str();
	} else if (strcasecmp("fullscreen", key) == 0) {
		return _fullscreen.c_str();
	}

	return defval;
}

void Registry::set(const char *key, const char *val) {
	// Hack: Don't save these, so we can run in good_times mode
	// without new games being bogus.
	if (strstr(key, "GrimLastSet") || strstr(key, "GrimMannyState"))
		return;

	_dirty = true;
	assert(val);

	if (strcasecmp("good_times", key) == 0 || strcasecmp("GrimDeveloper", key) == 0) {
		_develMode = val;
		return;
	} else if (strcasecmp("GrimDataDir", key) == 0) {
		_dataPath = val;
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
	} else if (strcasecmp("SpeechMode", key) == 0) {
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
	} else if (strcasecmp("show-fps", key) == 0) {
		_showFps = val;
		return;
	} else if (strcasecmp("soft-renderer", key) == 0) {
		_softRenderer = val;
		return;
	} else if (strcasecmp("gl-zbuffer", key) == 0) {
		_glZbuffer = val;
		return;
	} else if (strcasecmp("fullscreen", key) == 0) {
		_fullscreen = val;
		return;
	}
}

void Registry::save() {
	if (!_dirty)
		return;

	ConfMan.set("game-devel-mode", _develMode);
	ConfMan.set("path", _dataPath);
	ConfMan.set("last-set", _lastSet);
	ConfMan.set("music-volume", _musicVolume);
	ConfMan.set("sfx-volume", _sfxVolume);
	ConfMan.set("voice-volume", _voiceVolume);
	ConfMan.set("last-saved-game", _lastSavedGame);
	ConfMan.set("gamma", _gamma);
	ConfMan.set("voice-effects", _voiceEffects);
	ConfMan.set("text-speed", _textSpeed);
	ConfMan.set("speech-mode", _speechMode);
	ConfMan.set("movement", _movement);
	ConfMan.set("joystick", _joystick);
	ConfMan.set("spew-on-error", _spewOnError);
	ConfMan.set("show-fps", _showFps);
	ConfMan.set("soft-renderer", _softRenderer);
	ConfMan.set("gl-zbuffer", _glZbuffer);
	ConfMan.set("fullscreen", _fullscreen);

	ConfMan.flushToDisk();

	_dirty = false;
}
