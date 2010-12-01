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

DECLARE_SINGLETON(Asylum::ConfigurationManager);

namespace Asylum {

static bool g_config_initialized = false;

ConfigurationManager::ConfigurationManager() {
	if (!g_config_initialized) {
		g_config_initialized = true;
	}

	// Register engine-specific options
	ConfMan.registerDefault("show_encounter_subtitles", true);
	ConfMan.registerDefault("gamma_level", 0);
	ConfMan.registerDefault("ambient_volume", 0);
	ConfMan.registerDefault("movie_volume", 0);
	ConfMan.registerDefault("music_status", true);
	ConfMan.registerDefault("reverse_stereo", false);
	ConfMan.registerDefault("performance", 4);
	ConfMan.registerDefault("key_showVersion", 'v');
	ConfMan.registerDefault("key_quickLoad", 'L');
	ConfMan.registerDefault("key_quickSave", 'S');
	ConfMan.registerDefault("key_switchToSara", 's');
	ConfMan.registerDefault("key_switchToGrimwall", 'g');
	ConfMan.registerDefault("key_switchToOlmec", 'o');

	ConfMan.registerDefault("show_scene_loading", false);
	ConfMan.registerDefault("show_intro", true);

	// Init values
	musicVolume   = 0;
	sfxVolume     = 0;
	voiceVolume   = 0;
	showMovieSubtitles = false;

	showEncounterSubtitles = true;
	gammaLevel = 0;
	ambientVolume = 0;
	movieVolume   = 0;
	musicStatus = true;
	reverseStereo = false;
	performance = 4;

	keyShowVersion = 'v';
	keyQuickLoad = 'L';
	keyQuickSave = 'S';
	keySwitchToSara = 's';
	keySwitchToGrimwall = 'g';
	keySwitchToOlmec = 'o';
}

ConfigurationManager::~ConfigurationManager() {
}

void ConfigurationManager::read() {
    // Default options
    musicVolume = ConfMan.getInt("music_volume");
    sfxVolume   = ConfMan.getInt("sfx_volume");
    voiceVolume = ConfMan.getInt("speech_volume");
    showMovieSubtitles = ConfMan.getBool("subtitles");

	// Engine options
	showEncounterSubtitles = ConfMan.getBool("show_encounter_subtitles");
	gammaLevel    = ConfMan.getInt("gamma_level");
	ambientVolume = ConfMan.getInt("ambient_volume");;
	movieVolume   = ConfMan.getInt("movie_volume");
	musicStatus   = ConfMan.getBool("music_status");
	reverseStereo = ConfMan.getBool("reverse_stereo");
	performance   = ConfMan.getInt("performance");

	// Misc options
	showSceneLoading = ConfMan.getBool("show_scene_loading");
	showIntro        = ConfMan.getBool("show_intro");

	// Keyboard shortcuts
	keyShowVersion      = (char)ConfMan.getInt("key_showVersion");
	keyQuickLoad        = (char)ConfMan.getInt("key_quickLoad");
	keyQuickSave        = (char)ConfMan.getInt("key_quickSave");
	keySwitchToSara     = (char)ConfMan.getInt("key_switchToSara");
	keySwitchToGrimwall = (char)ConfMan.getInt("key_switchToGrimwall");
	keySwitchToOlmec    = (char)ConfMan.getInt("key_switchToOlmec");
}

void ConfigurationManager::write() {
    // Default options
    ConfMan.setInt("music_volume", musicVolume);
    ConfMan.setInt("sfx_volume", sfxVolume);
    ConfMan.setInt("speech_volume", voiceVolume);
    ConfMan.setBool("subtitles", showMovieSubtitles);

	// Engine options
	ConfMan.setBool("show_encounter_subtitles", showEncounterSubtitles);
	ConfMan.setInt("gamma_level", gammaLevel);
	ConfMan.setInt("ambient_volume", ambientVolume);
	ConfMan.setInt("movie_volume", movieVolume);
	ConfMan.setBool("music_status", musicStatus);
	ConfMan.setBool("reverse_stereo", reverseStereo);
	ConfMan.setInt("performance", performance);

	// Misc Options
	ConfMan.setBool("show_scene_loading", showSceneLoading);
	ConfMan.setBool("show_intro", showIntro);

	// Keyboard shortcuts
	ConfMan.setInt("key_showVersion",      (int)keyShowVersion);
	ConfMan.setInt("key_quickLoad",        (int)keyQuickLoad);
	ConfMan.setInt("key_quickSave",        (int)keyQuickSave);
	ConfMan.setInt("key_switchToSara",     (int)keySwitchToSara);
	ConfMan.setInt("key_switchToGrimwall", (int)keySwitchToGrimwall);
	ConfMan.setInt("key_switchToOlmec",    (int)keySwitchToOlmec);
}

} // end of namespace Asylum
