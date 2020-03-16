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
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/system.h"
#include "common/translation.h"

namespace Common {

DECLARE_SINGLETON(AchievementsManager);


AchievementsManager::AchievementsManager() {
	unsetActiveDomain();
}


AchievementsManager::~AchievementsManager() {
}

bool AchievementsManager::setActiveDomain(AchievementsPlatform platform, const String &appId) {
	String suffix = platform == STEAM_ACHIEVEMENTS ? "steam-" + appId :
					platform == GALAXY_ACHIEVEMENTS ? "galaxy-" + appId :
					appId;

	String achDomainId = "achievements-" + suffix;
	String statDomainId = "statistics-" + suffix;

	if (_achDomainId == achDomainId && _statDomainId == statDomainId) {
		return true;
	}

	if (isReady()) {
		unsetActiveDomain();
	}

	ConfMan.addMiscDomain(achDomainId);
	ConfMan.addMiscDomain(statDomainId);

	_achDomainId = achDomainId;
	_statDomainId = statDomainId;
	return true;
}


bool AchievementsManager::unsetActiveDomain() {
	if (!isReady()) {
		return true;
	}

	ConfMan.flushToDisk();

	_achDomainId = "";
	_statDomainId = "";
	return true;
}


bool AchievementsManager::setAchievement(const String &id, const String &displayedMessage) {
	if (!isReady()) {
		return false;
	}
	if (isAchieved(id)) {
		return true;
	}

	debug("AchievementsManager::setAchievement('%s'): Achievement unlocked!", id.c_str());

	ConfMan.setBool(id, true, _achDomainId);
	ConfMan.flushToDisk();

	if (!displayedMessage.empty() && g_system) {
		String msg;
		msg = Common::String::format("%s\n%s", _("Achievement unlocked!"), displayedMessage.c_str());
		g_system->displayMessageOnOSD(msg.c_str());
	}

	return true;
}


bool AchievementsManager::isAchieved(const String &id) {
	if (!isReady()) {
		return false;
	}

	return ConfMan.hasKey(id, _achDomainId) && ConfMan.getBool(id, _achDomainId);
}


bool AchievementsManager::clearAchievement(const String &id) {
	if (!isReady()) {
		return false;
	}

	ConfMan.removeKey(id, _achDomainId);
	ConfMan.flushToDisk();
	return true;
}


bool AchievementsManager::setStatFloat(const String &id, float value) {
	if (!isReady()) {
		return false;
	}

	String tmp = Common::String::format("%8.8f", value);
	ConfMan.set(id, tmp, _statDomainId);
	ConfMan.flushToDisk();
	return 0;
}


float AchievementsManager::getStatFloat(const String &id) {
	if (!isReady()) {
		return 0.0;
	}

	String tmp = ConfMan.get(id, _statDomainId);
	return atof(tmp.c_str());
}


bool AchievementsManager::setStatInt(String const &id, int value) {
	if (!isReady()) {
		return false;
	}

	ConfMan.setInt(id, value, _statDomainId);
	ConfMan.flushToDisk();
	return 0;
}


int AchievementsManager::getStatInt(String const &id) {
	if (!isReady()) {
		return 0;
	}

	return ConfMan.getInt(id, _statDomainId);
}


bool AchievementsManager::resetAllAchievements() {
	if (!isReady()) {
		return false;
	}

	ConfMan.removeMiscDomain(_achDomainId);
	ConfMan.flushToDisk();
	return 0;
}


bool AchievementsManager::resetAllStats() {
	if (!isReady()) {
		return false;
	}

	ConfMan.removeMiscDomain(_statDomainId);
	ConfMan.flushToDisk();
	return 0;
}


} // End of namespace Common
