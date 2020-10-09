/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/grim/emi/emi_registry.h"
#include "engines/grim/debug.h"

#include "audio/mixer.h"

#include "common/config-manager.h"

namespace Grim {

EmiRegistry *g_emiregistry = nullptr;

const char *EmiRegistry::_translTable[][2] = {
	{"speech_mode",		""},	//Translated key not needed, see below
	{"vocvolume",		"speech_volume"},
	{"sfxvolume",		"sfx_volume"},
	{"musvolume",		"music_volume"},
	{"textspeed",		"talkspeed"},
	{"gamma",			"gamma"},
	{"joystick_enabled","joystick_enabled"},
	{"analog_mode",		"analog_joystick_mode"},
	{"subtitles",		"movie_subtitles"},
	{"camera_mode",		"camera_relative_mode"},
	{"shadowfx",		"shadow_effects"},
	{"vocfx",			"audio_effects"},
	{"miscfx",			"misc_video_effects"},
	{"moviequality",	"movie_quality"},
	{"musicquality",	"music_quality"},
	{nullptr,nullptr}
};

const char *EmiRegistry::_boolValues[] = {
	"joystick_enabled",
	"analog_mode",
	"subtitles",
	"camera_mode",
	"vocfx",
	"moviequality",
	"musicquality",
	nullptr
};


EmiRegistry::EmiRegistry() {
	int i = 0;
	while (_translTable[i][0] != nullptr) {
		_transMap[_translTable[i][0]] = _translTable[i][1];
		++i;
	}

	i = 0;
	while (_boolValues[i] != nullptr) {
		_boolSet[_boolValues[i]] = true;
		++i;
	}
}

uint EmiRegistry::convertTalkSpeedToGUI(uint talkspeed) const {
	return CLIP<uint>(talkspeed * 255 / 10, 0, 255);
}

uint EmiRegistry::convertTalkSpeedFromGUI(uint talkspeed) const {
	return CLIP<uint>(talkspeed * 10 / 255, 1, 10);
}

const Common::String EmiRegistry::convertGammaToRegistry(float gamma) const {
	return Common::String().format("%.2f", gamma);
}

float EmiRegistry::convertGammaFromRegistry(const Common::String &gamma) const {
	float gamma_f;
	sscanf(gamma.c_str(), "%f", &gamma_f);
	return CLIP<float>(gamma_f, 0.5, 1.5);
}

uint EmiRegistry::convertVolumeToMixer(uint emiVolume) const {
	float vol = float(emiVolume - 25)/(100 - 25)*Audio::Mixer::kMaxMixerVolume;
	return CLIP<uint>(uint(vol), 0, Audio::Mixer::kMaxMixerVolume);
}

uint EmiRegistry::convertVolumeFromMixer(uint volume) const {
	float vol = float(volume)*(100 - 25)/Audio::Mixer::kMaxMixerVolume + 25;
	return CLIP<uint>(uint(vol), 0, Audio::Mixer::kMaxMixerVolume);
}

uint EmiRegistry::convertSpeechModeFromGUI(bool subtitles, bool speechMute) const {
	if (!subtitles && !speechMute) // Speech only
		return 2;
	else if (subtitles && !speechMute) // Speech and subtitles
		return 3;
	else if (subtitles && speechMute) // Subtitles only
		return 1;
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return 1;
}

bool EmiRegistry::Get(const Common::String key, float &res) const {
	Debug::debug(Debug::Engine, "GetResidualVMPreference(%s)", key.c_str());

	if (!_transMap.contains(key))
		return false;

	res = 0.;

	if (key == "speech_mode") {
		if (!(ConfMan.hasKey("subtitles") && ConfMan.hasKey("speech_mute")))
			return false;

		res = convertSpeechModeFromGUI(ConfMan.getBool("subtitles"), ConfMan.getBool("speech_mute"));
	} else {
		if (!(ConfMan.hasKey(_transMap[key])))
			return false;

		if (key == "vocvolume" || key == "sfxvolume" || key == "musvolume")
			res = convertVolumeFromMixer(ConfMan.getInt(_transMap[key]));
		else if (key == "textspeed")
			res = convertTalkSpeedFromGUI(ConfMan.getInt(_transMap[key]));
		else if (key == "gamma")
			res = convertGammaFromRegistry(ConfMan.get(_transMap[key]));
		else if (key == "shadowfx")
			res = ConfMan.getBool(_transMap[key]) + 1;
		else if (_boolSet.contains(key))
			res = ConfMan.getBool(_transMap[key]);
		else
			res = ConfMan.getInt(_transMap[key]);
	}

	Debug::debug(Debug::Engine, "Pushing %f", res);
	return true;
}

void EmiRegistry::Set(const Common::String key, float &value)  {
	Debug::debug(Debug::Engine, "SetResidualVMPreference(%s, %f)", key.c_str(), value);

	if (!_transMap.contains(key))
		return;

	uint value_i = uint(value);

	if (key == "speech_mode") {
		ConfMan.setBool("subtitles", (value_i == 1 || value_i == 3));
		ConfMan.setBool("speech_mute", (value_i == 1));
	} else if (key == "vocvolume" || key == "sfxvolume" || key == "musvolume")
		ConfMan.setInt(_transMap[key], convertVolumeToMixer(value_i));
	else if (key == "textspeed")
		ConfMan.setInt(_transMap[key], convertTalkSpeedToGUI(value_i));
	else if (key == "gamma")
		ConfMan.set(_transMap[key], convertGammaToRegistry(value));
	else if (key == "shadowfx")
		ConfMan.setBool(_transMap[key], (value_i == 2));
	else if (_boolSet.contains(key))
		ConfMan.setBool(_transMap[key], (value_i == 1));
	else
		ConfMan.setInt(_transMap[key], value_i);
}

} // end of namespace Grim
