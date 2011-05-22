/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sword25/util/lua/scummvm_file.h"
#include "common/config-manager.h"
#include "common/util.h"

namespace Sword25 {

Sword25FileProxy::Sword25FileProxy(const Common::String &filename, const Common::String &mode) {
	assert(filename.contains("config.lua"));
	if (mode == "r")
		setupConfigFile();
}

void Sword25FileProxy::setupConfigFile() {
	float sfxVolume = ConfMan.hasKey("sfx_volume") ? 1.0 : 1.0 * ConfMan.getInt("sfx_volume") / 255.0;
	float musicVolume = ConfMan.hasKey("music_volume") ? 0.5 : 1.0 * ConfMan.getInt("music_volume") / 255.0;
	float speechVolume = ConfMan.hasKey("speech_volume") ? 1.0 : 1.0 * ConfMan.getInt("speech_volume") / 255.0;
	bool subtitles = ConfMan.hasKey("subtitles") ? true : ConfMan.getBool("subtitles");

	_readData = Common::String::format(
"GAME_LANGUAGE = \"%s\"\r\n\
GAME_SUBTITLES = %s\r\n\
MAX_MEMORY_USAGE = 256000000\r\n\
GFX_VSYNC_ACTIVE = true\r\n\
SFX_SAMPLING_RATE = 44100\r\n\
SFX_CHANNEL_COUNT = 32\r\n\
SFX_SOUND_VOLUME = %f\r\n\
SFX_MUSIC_VOLUME = %f\r\n\
SFX_SPEECH_VOLUME = %f\r\n",
		getLanguage().c_str(), subtitles ? "true" : "false", sfxVolume, musicVolume, speechVolume);

	_readPos = 0;
}

/**
 * Get the language code used by the game for each language it supports
 */
Common::String Sword25FileProxy::getLanguage() {
	Common::Language lang = Common::parseLanguage(ConfMan.get("language"));
	switch (lang) {
	case Common::EN_ANY:
		return "en";
	case Common::DE_DEU:
		return "de";
	case Common::ES_ESP:
		return "es";
	case Common::FR_FRA:
		return "fr";
	case Common::HU_HUN:
		return "hr";
	case Common::IT_ITA:
		return "it";
	case Common::PL_POL:
		return "pl";
	case Common::PT_BRA:
		return "pt";
	case Common::RU_RUS:
		return "ru";
	default:
		error("Unknown language '%s' encountered", ConfMan.get("language"));
		break;
	}
}

size_t Sword25FileProxy::read(void *ptr, size_t size, size_t count) {
	size_t bytesRead = MIN(_readData.size() - _readPos, size * count);
	memmove(ptr, &_readData.c_str()[_readPos], bytesRead);
	_readPos += bytesRead;
	return bytesRead / size;
}

} // End of namespace Sword25
