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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "asylum/system/config.h"

#include "asylum/system/sound.h"

namespace Common {
DECLARE_SINGLETON(Asylum::ConfigurationManager);
}

namespace Asylum {

static bool g_config_initialized = false;

ConfigurationManager::ConfigurationManager() {
	if (!g_config_initialized) {
		g_config_initialized = true;
	}

	// Register engine-specific options
	ConfMan.registerDefault("show_encounter_subtitles", true);
	ConfMan.registerDefault("gamma_level", 0);
	ConfMan.registerDefault("ambient_volume",  -750);
	ConfMan.registerDefault("movie_volume",    -500);
	ConfMan.registerDefault("music_volume",   -1500);
	ConfMan.registerDefault("sfx_volume",     -1000);
	ConfMan.registerDefault("speech_volume",   -750);

	ConfMan.registerDefault("music_status", true);
	ConfMan.registerDefault("reverse_stereo", false);
	ConfMan.registerDefault("performance", 4);
	ConfMan.registerDefault("key_showVersion", 'v');
	ConfMan.registerDefault("key_quickLoad", 'L');
	ConfMan.registerDefault("key_quickSave", 'S');
	ConfMan.registerDefault("key_switchToSara", 's');
	ConfMan.registerDefault("key_switchToGrimwall", 'g');
	ConfMan.registerDefault("key_switchToOlmec", 'o');

	// Special debug options
	ConfMan.registerDefault("show_scene_loading", true);
	ConfMan.registerDefault("show_intro", true);

	// Init values
	musicVolume   = 0;
	ambientVolume = 0;
	sfxVolume     = 0;
	voiceVolume   = 0;
	movieVolume   = 0;

	musicStatus = true;
	reverseStereo = false;

	showMovieSubtitles = false;
	showEncounterSubtitles = true;
	showSceneLoading = true;
	showIntro = true;

	gammaLevel = 0;
	performance = 0;

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

	// Convert volumes to engine-values
	Sound::convertVolumeTo(musicVolume);
	Sound::convertVolumeTo(sfxVolume);
	Sound::convertVolumeTo(voiceVolume);

	// Engine options
	showEncounterSubtitles = ConfMan.getBool("show_encounter_subtitles");
	gammaLevel    = ConfMan.getInt("gamma_level");
	ambientVolume = ConfMan.getInt("ambient_volume");
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
	int32 mixerMusicVolume = musicVolume;
	int32 mixerSfxVolume   = sfxVolume;
	int32 mixerVoiceVolume = voiceVolume;

	// Convert volumes to mixer-values
	Sound::convertVolumeFrom(mixerMusicVolume);
	Sound::convertVolumeFrom(mixerSfxVolume);
	Sound::convertVolumeFrom(mixerVoiceVolume);

	// Default options
	ConfMan.setInt("music_volume", mixerMusicVolume);
	ConfMan.setInt("sfx_volume", mixerSfxVolume);
	ConfMan.setInt("speech_volume", mixerVoiceVolume);
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

	ConfMan.flushToDisk();
}

bool ConfigurationManager::isKeyAssigned(char key) const {
	return (keyShowVersion == key || keyQuickLoad == key || keyQuickSave == key || keySwitchToSara == key || keySwitchToGrimwall == key || keySwitchToOlmec == key);
}

} // end of namespace Asylum
