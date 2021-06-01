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


#include "common/achievements.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/translation.h"

namespace Common {

DECLARE_SINGLETON(AchievementsManager);


AchievementsManager::AchievementsManager() {
	_iniFile = nullptr;
	unsetActiveDomain();
}


AchievementsManager::~AchievementsManager() {
}

bool AchievementsManager::setActiveDomain(const AchievementsInfo &info) {
	if (info.appId.empty()) {
		unsetActiveDomain();
		return false;
	}

	const char* platform = info.platform == STEAM_ACHIEVEMENTS ? "steam" :
					info.platform == GALAXY_ACHIEVEMENTS ? "galaxy" :
					"achman";

	String iniFileName = String::format("%s-%s.dat", platform, info.appId.c_str());

	if (_iniFileName == iniFileName) {
		return true;
	}

	if (isReady()) {
		unsetActiveDomain();
	}

	debug("AchievementsManager::setActiveDomain(): '%s'", iniFileName.c_str());

	_iniFileName = iniFileName;

	_iniFile = new Common::INIFile();
	_iniFile->loadFromSaveFile(_iniFileName); // missing file is OK

	_descriptions = info.descriptions;

	for (uint32 i = 0; i < info.stats.size(); i++) {
		if (!(_iniFile->hasKey(info.stats[i].id, "statistics"))) {
			_iniFile->setKey(info.stats[i].id, "statistics", info.stats[i].start);
		}
	}

	setSpecialString("platform", platform);
	setSpecialString("gameId", info.appId);

	return true;
}


bool AchievementsManager::unsetActiveDomain() {
	debug("AchievementsManager::unsetActiveDomain()");

	_iniFileName = "";

	delete _iniFile;
	_iniFile = nullptr;

	_descriptions.clear();

	return true;
}


bool AchievementsManager::setAchievement(const String &id) {
	if (!isReady()) {
		warning("AchievementsManager::setAchievement('%s'): AchMan not ready, did you forget to call setActiveDomain()?", id.c_str());
		return false;
	}
	if (isAchieved(id)) {
		return true;
	}

	String displayedMessage = id;
	for (uint32 i = 0; i < _descriptions.size(); i++) {
		if (strcmp(_descriptions[i].id, id.c_str()) == 0) {
			displayedMessage = _descriptions[i].title;
			break;
		}
	}

	debug("AchievementsManager::setAchievement('%s'): '%s'", id.c_str(), displayedMessage.c_str());

	_iniFile->setKey(id, "achievements", "true");
	_iniFile->saveToSaveFile(_iniFileName);

	if (!displayedMessage.empty() && g_system) {
		U32String msg;
		msg = Common::U32String::format("%S\n%S",
			_("Achievement unlocked!").c_str(),
			Common::U32String(displayedMessage).c_str()
		);
		g_system->displayMessageOnOSD(msg);
	}

	return true;
}


bool AchievementsManager::isAchieved(const String &id) const {
	if (!isReady()) {
		return false;
	}

	return _iniFile->hasKey(id, "achievements");
}


bool AchievementsManager::clearAchievement(const String &id) {
	if (!isReady()) {
		return false;
	}

	_iniFile->removeKey(id, "achievements");
	_iniFile->saveToSaveFile(_iniFileName);
	return true;
}


bool AchievementsManager::setStatFloatEx(const String &id, float value, const String &section) const {
	if (!isReady()) {
		return false;
	}

	String tmp = Common::String::format("%8.8f", value);
	_iniFile->setKey(id, section, tmp);
	_iniFile->saveToSaveFile(_iniFileName);
	return 0;
}


float AchievementsManager::getStatFloatEx(const String &id, const String &section) const {
	if (!isReady()) {
		return 0.0;
	}

	String tmp;
	_iniFile->getKey(id, section, tmp);
	return (float)atof(tmp.c_str());
}


bool AchievementsManager::setStatFloat(const String &id, float value) {
	return setStatFloatEx(id, value, "statistics");
}


float AchievementsManager::getStatFloat(const String &id) const {
	return getStatFloatEx(id, "statistics");
}


bool AchievementsManager::updateAverageRateStatFloat(const String &id, float count, float times) {
	if (!isReady()) {
		return false;
	}

	float old_count = getStatFloatEx(id + "_count", "rates");
	float old_times = getStatFloatEx(id + "_times", "rates");

	setStatFloatEx(id + "_count", old_count + count, "rates");
	setStatFloatEx(id + "_times", old_times + times, "rates");

	return 0;
}


float AchievementsManager::getAverageRateStatFloat(const String &id) const {
	if (!isReady()) {
		return 0.0;
	}

	float count = getStatFloatEx(id + "_count", "rates");
	float times = getStatFloatEx(id + "_times", "rates");

	return (times != 0) ? (count / times) : 0.0;
}


bool AchievementsManager::setStatInt(String const &id, int value) {
	if (!isReady()) {
		return false;
	}

	String tmp = Common::String::format("%d", value);
	_iniFile->setKey(id, "statistics", tmp);
	_iniFile->saveToSaveFile(_iniFileName);
	return 0;
}


int AchievementsManager::getStatInt(String const &id) const {
	if (!isReady()) {
		return 0;
	}

	String tmp;
	_iniFile->getKey(id, "statistics", tmp);
	return (int)atol(tmp.c_str());
}


const String AchievementsManager::getStatRaw(String const &id) const {
	if (!isReady()) {
		return "";
	}

	String tmp;
	_iniFile->getKey(id, "statistics", tmp);
	return tmp;
}


bool AchievementsManager::setSpecialString(String const &id, String const &value) {
	if (!isReady()) {
		return false;
	}

	_iniFile->setKey(id, "special", value);
	_iniFile->saveToSaveFile(_iniFileName);
	return 0;
}


bool AchievementsManager::resetAllAchievements() {
	if (!isReady()) {
		return false;
	}

	_iniFile->removeSection("achievements");
	_iniFile->saveToSaveFile(_iniFileName);
	return 0;
}


bool AchievementsManager::resetAllStats() {
	if (!isReady()) {
		return false;
	}

	_iniFile->removeSection("statistics");
	_iniFile->removeSection("rates");
	_iniFile->saveToSaveFile(_iniFileName);
	return 0;
}


} // End of namespace Common
