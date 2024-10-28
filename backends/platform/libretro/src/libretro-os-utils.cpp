/* Copyright (C) 2023 Giovanni Cascione <ing.cascione@gmail.com>
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
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <features/features_cpu.h>

#include "common/tokenizer.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "base/commandLine.h"
#include "base/plugins.h"
#include "engines/game.h"
#include "engines/metaengine.h"

#include "backends/platform/libretro/include/libretro-defs.h"
#include "backends/platform/libretro/include/libretro-core.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-options-widget.h"
#include "backends/platform/libretro/include/libretro-fs.h"

void OSystem_libretro::getTimeAndDate(TimeDate &t, bool skipRecord) const {
	uint32 curTime = (uint32)(cpu_features_get_time_usec() / 1000000);

#define YEAR0 1900
#define EPOCH_YR 1970
#define SECS_DAY (24L * 60L * 60L)
#define LEAPYEAR(year) (!((year) % 4) && (((year) % 100) || !((year) % 400)))
#define YEARSIZE(year) (LEAPYEAR(year) ? 366 : 365)
	const int _ytab[2][12] = {{31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}, {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};
	int year = EPOCH_YR;
	unsigned long dayclock = (unsigned long)curTime % SECS_DAY;
	unsigned long dayno = (unsigned long)curTime / SECS_DAY;
	t.tm_sec = dayclock % 60;
	t.tm_min = (dayclock % 3600) / 60;
	t.tm_hour = dayclock / 3600;
	t.tm_wday = (dayno + 4) % 7; /* day 0 was a thursday */
	while (dayno >= YEARSIZE(year)) {
		dayno -= YEARSIZE(year);
		year++;
	}
	t.tm_year = year - YEAR0;
	t.tm_mon = 0;
	while (dayno >= _ytab[LEAPYEAR(year)][t.tm_mon]) {
		dayno -= _ytab[LEAPYEAR(year)][t.tm_mon];
		t.tm_mon++;
	}
	t.tm_mday = dayno + 1;
}

Common::Path OSystem_libretro::getDefaultConfigFileName() {
	if (s_systemDir.empty())
		return Common::Path("scummvm.ini");
	else
		return Common::Path(s_systemDir).appendComponent("scummvm.ini");
}

void OSystem_libretro::logMessage(LogMessageType::Type type, const char *message) {
	retro_log_level loglevel = RETRO_LOG_INFO;
	switch (type) {
	case LogMessageType::kDebug:
		loglevel = RETRO_LOG_DEBUG;
		break;
	case LogMessageType::kWarning:
		loglevel = RETRO_LOG_WARN;
		break;
	case LogMessageType::kError:
		loglevel = RETRO_LOG_ERROR;
		break;
	}

	if (retro_log_cb)
		retro_log_cb(loglevel, "%s\n", message);
}


bool OSystem_libretro::parseGameName(const Common::String &gameName, Common::String &engineId,
                                     Common::String &gameId) {
	Common::StringTokenizer tokenizer(gameName, ":");
	Common::String token1, token2;

	if (!tokenizer.empty()) {
		token1 = tokenizer.nextToken();
	}

	if (!tokenizer.empty()) {
		token2 = tokenizer.nextToken();
	}

	if (!tokenizer.empty()) {
		return false; // Stray colon
	}

	if (!token1.empty() && !token2.empty()) {
		engineId = token1;
		gameId = token2;
		return true;
	} else if (!token1.empty()) {
		engineId.clear();
		gameId = token1;
		return true;
	}

	return false;
}

int OSystem_libretro::testGame(const char *filedata, bool autodetect) {
	Common::String game_id;
	Common::String engine_id;
	Common::String data = filedata;
	int res = TEST_GAME_KO_NOT_FOUND;

	PluginManager::instance().init();
	PluginManager::instance().loadAllPlugins();
	PluginManager::instance().loadDetectionPlugin();

	if (autodetect) {
		Common::FSNode dir = Common::FSNode(Common::Path(data));
		Common::FSList files;
		dir.getChildren(files, Common::FSNode::kListAll);

		DetectionResults detectionResults = EngineMan.detectGames(files);
		if (!detectionResults.listRecognizedGames().empty()) {
			res = TEST_GAME_OK_ID_AUTODETECTED;
		}

	} else {

		ConfMan.loadDefaultConfigFile(getDefaultConfigFileName().toString().c_str());
		if (ConfMan.hasGameDomain(data)) {
			res = TEST_GAME_OK_TARGET_FOUND;
		} else {
			parseGameName(data, engine_id, game_id);

			QualifiedGameList games = EngineMan.findGamesMatching(engine_id, game_id);
			if (games.size() == 1) {
				res = TEST_GAME_OK_ID_FOUND;
			} else if (games.size() > 1) {
				res = TEST_GAME_KO_MULTIPLE_RESULTS;
			}
		}
	}

	PluginManager::destroy();
	return res;
}

GUI::OptionsContainerWidget *OSystem_libretro::buildBackendOptionsWidget(GUI::GuiObject *boss, const Common::String &name, const Common::String &target) const {
	if (target.equalsIgnoreCase(Common::ConfigManager::kApplicationDomain))
		return new LibretroOptionsWidget(boss, name, target);
	else
		return nullptr;

}

bool OSystem_libretro::checkPathSetting(const char *setting, Common::String const &defaultPath, bool isDirectory) {
	Common::String setPath;
	if (ConfMan.hasKey(setting))
		setPath = Common::Path::fromConfig(ConfMan.get(setting)).toString();
	if (setPath.empty() || ! (isDirectory ? LibRetroFilesystemNode(setPath).isDirectory() : LibRetroFilesystemNode(setPath).exists()))
		ConfMan.removeKey(setting, Common::ConfigManager::kApplicationDomain);
	if (! ConfMan.hasKey(setting))
		if (defaultPath.empty())
			return false;
		else
			ConfMan.set(setting, defaultPath);
	return true;
}

void OSystem_libretro::setLibretroDir(const char * path, Common::String &var) {
	var = Common::String(path ? path : "");
	if (! var.empty())
		if (! LibRetroFilesystemNode(var).isDirectory())
			var.clear();
	return;
}

void OSystem_libretro::applyBackendSettings() {
	/* ScummVM paths checks at startup and on settings applied */
	Common::String s_homeDir(LibRetroFilesystemNode::getHomeDir());
	Common::String s_themeDir(s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_THEME_SUBDIR);
	Common::String s_extraDir(s_systemDir + "/" + SCUMMVM_SYSTEM_SUBDIR + "/" + SCUMMVM_EXTRA_SUBDIR);
	Common::String s_soundfontPath(s_extraDir + "/" + DEFAULT_SOUNDFONT_FILENAME);

	if (! LibRetroFilesystemNode(s_themeDir).isDirectory())
		s_themeDir.clear();
	if (! LibRetroFilesystemNode(s_extraDir).isDirectory())
		s_extraDir.clear();
	if (! LibRetroFilesystemNode(s_soundfontPath).exists())
		s_soundfontPath.clear();
	if (s_homeDir.empty() || ! LibRetroFilesystemNode(s_homeDir).isDirectory())
		s_homeDir = s_systemDir;

	//Register default paths
	if (! s_homeDir.empty()) {
		ConfMan.registerDefault("browser_lastpath", s_homeDir);
		retro_log_cb(RETRO_LOG_DEBUG, "Default browser last path set to: %s\n", s_homeDir.c_str());
	}
	if (! s_saveDir.empty()) {
		ConfMan.registerDefault("savepath", s_saveDir);
		retro_log_cb(RETRO_LOG_DEBUG, "Default save path set to: %s\n", s_saveDir.c_str());
	}

	//Check current path settings
	if (!checkPathSetting("savepath", s_saveDir)) {
		ConfMan.setAndFlush("savepath", s_homeDir);
		retro_osd_notification("ScummVM save folder not found.");
	}
	if (!checkPathSetting("themepath", s_themeDir))
		retro_osd_notification("ScummVM theme folder not found.");
	if (!checkPathSetting("extrapath", s_extraDir))
		retro_osd_notification("ScummVM extra folder not found. Some engines/features (e.g. Virtual Keyboard) will not work without relevant datafiles.");
	checkPathSetting("soundfont", s_soundfontPath, false);
	checkPathSetting("browser_lastpath", s_homeDir);
	checkPathSetting("libretro_playlist_path", s_playlistDir.empty() ? s_homeDir : s_playlistDir);
	checkPathSetting("iconspath", "");
}

static const char * const helpTabs[] = {
_s("Libretro playlist"),
"",
_s(
"## Libretro playlists for ScummVM core\n"
"Playlists used in Libretro frontends (e.g. Retroarch) are plain text lists used to directly launch a game with a specific core from the user interface. Those lists are structured to pass to the core the path of a specific content file to be loaded (e.g. ROM).\n"
"\n"
"ScummVM core can accept as content the path to any of the files inside a valid game folder, the detection system will try to autodetect the game from the content file parent folder and run the game with default ScummVM options.\n"
"\n"
"The core also supports dedicated per game **hook** plain text files with **." CORE_EXTENSIONS "** extension, which can be used as target in the playlist to specify one of the following ScummVM identifiers:\n"
"\n"
"  - **game ID**: this is a unique identifier for any game supported by ScummVM. In this case hook files must be placed inside each game folder, and there is no need to add the game from within ScummVM. Game will be launched with default ScummVM options.\n"
"\n"
"  - **target**: this is the game identifier from ScummVM configuration file (e.g. 'scummvm.ini'). In this case the game must be added from ScummVM GUI first, and the hook files can be placed anywhere, as the path for the game files is already part of the target configuration. The game will be launched with the options set in ScummVM\n"
"\n"
"## Creating ScummVM core playlist\n"
"ScummVM core playlist can be created in the following ways:\n"
"\n"
"  1. Manually (hook files to be created manually - optional)\n"
"\n"
"  2. Automatically from Retroarch scanner (hook files not used)\n"
"\n"
"  3. Automatically from ScummVM GUI (hook files created automatically)\n"
"\n"
"First two methods are not covered here, as outside of ScummVM scope. Detailed info can be found in [Libretro documentation](https://docs.libretro.com/guides/roms-playlists-thumbnails/).\n"
"Note that Retroarch scanner is based on a third party database instead of ScummVM game detection system, hence it is not guaranteed to work properly.\n"
"\n"
"Third method is covered in the following subheading.\n"
"\n"
"## ScummVM Playlist Generator\n"
"ScummVM core includes a tool to generate a Libretro playlist and needed hook files based on current ScummVM games list.\n"
"\n"
" - Load the core from RetroArch and start it to reach the ScummVM GUI (i.e. the Launcher)\n"
"\n"
" - Add games to the list as required using the GUI buttons ('Mass Add' available).\n"
"\n"
" - Select **Global Options** and then the **Backend** tab.\n"
"\n"
" - Check or select the path of frontend playlists. A '" CORE_NAME ".lpl' file will be created or overwritten in there.\n"
"\n"
" - Check the 'Hooks location' setting, to have one '." CORE_EXTENSIONS "' in each game folder or all of them in a '" COMMON_HOOKS_FOLDER "' folder in the 'save' path.\n"
"\n"
" - Check the 'Playlist version' setting. JSON format should be selected, 6-lines format is deprecated and provided for backwards compatibility only.\n"
"\n"
" - Select the 'Clear existing hooks' checkbox to remove any existing '." CORE_EXTENSIONS "' file in the working folders.\n"
"\n"
" - Press the 'Generate playlist' button.\n"
"\n"
"Operation status will be shown in the same dialog, while details will be given in frontend logs."
),

0 // End of list
};

const char * const *OSystem_libretro::buildHelpDialogData() {
	return helpTabs;
}

Common::String OSystem_libretro::getSaveDir(void) {
	return s_saveDir;
}

void OSystem_libretro::addSysArchivesToSearchSet(Common::SearchSet &s, int priority) {
	if (!s_systemDir.empty())
		s.add("systemDir", new Common::FSDirectory(Common::FSNode(Common::Path(s_systemDir))), priority);
}
