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
 * $URL$
 * $Id$
 *
 */

#include "asylum/system/config.h"

DECLARE_SINGLETON(Asylum::ConfigurationManager)

namespace Asylum {

static bool g_config_initialized = false;

ConfigurationManager::ConfigurationManager() {
	if (!g_config_initialized) {
		g_config_initialized = true;
	}

	// if the values don't currently exist within the
	// configuration file, load the defaults

	ambientVolume = 0;
	movieVolume   = 0;
	musicVolume   = 0;
	sfxVolume     = 0;
}

ConfigurationManager::~ConfigurationManager() {
}

void ConfigurationManager::read() {
    // TODO: read all necessary configuration settings
    musicVolume = ConfMan.getInt("music_volume");
    sfxVolume   = ConfMan.getInt("sfx_volume");
    voiceVolume = ConfMan.getInt("speech_volume");
    showMovieSubtitles = (VideoSubtitles)ConfMan.getBool("subtitles");
}

void ConfigurationManager::write() {
    // TODO: save all necessary configuration settings
    ConfMan.setInt("music_volume", musicVolume);
    ConfMan.setInt("sfx_volume", sfxVolume);
    ConfMan.setInt("speech_volume", voiceVolume);
    ConfMan.setBool("subtitles", (bool)showMovieSubtitles);
}

} // end of namespace Asylum
