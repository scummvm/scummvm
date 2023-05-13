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
#define FORBIDDEN_SYMBOL_EXCEPTION_time

#include <sys/time.h>
#if defined(__CELLOS_LV2__)
#include <sys/sys_time.h>
#elif (defined(GEKKO) && !defined(WIIU))
#include <ogc/lwp_watchdog.h>
#else
#include <time.h>
#endif

#include "common/tokenizer.h"
#include "common/config-manager.h"
#include "base/commandLine.h"
#include "backends/platform/libretro/include/libretro-os.h"
#include "backends/platform/libretro/include/libretro-defs.h"

void OSystem_libretro::getTimeAndDate(TimeDate &t, bool skipRecord) const {
	time_t curTime = time(NULL);

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

Common::String OSystem_libretro::getDefaultConfigFileName() {
	return s_systemDir + "/scummvm.ini";
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

	if (log_cb)
		log_cb(loglevel, "%s\n", message);
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
		Common::FSNode dir(data);
		Common::FSList files;
		dir.getChildren(files, Common::FSNode::kListAll);

		DetectionResults detectionResults = EngineMan.detectGames(files);
		if (!detectionResults.listRecognizedGames().empty()) {
			res = TEST_GAME_OK_ID_AUTODETECTED;
		}

	} else {

		ConfMan.loadDefaultConfigFile(getDefaultConfigFileName().c_str());
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

	PluginManager::instance().unloadDetectionPlugin();
	PluginManager::instance().unloadAllPlugins();
	PluginManager::destroy();
	return res;
}
