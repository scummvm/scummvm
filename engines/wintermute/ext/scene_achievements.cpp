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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/ext/scene_achievements_tables.h"
#include "engines/wintermute/ext/wme_steam.h"

namespace Wintermute {

void SceneAchievements(const char *sceneFilename) {
	for (const AchievementsList *i = achievementsList; i->gameId; i++) {
		if (BaseEngine::instance().getGameId() == i->gameId) {
			for (const Achievement *it = i->mapping; it->sceneFilename; it++) {
				if (strcmp(sceneFilename, it->sceneFilename) == 0) {
					AchMan.setActiveDomain(getAchievementsInfo());
					AchMan.setAchievement(it->id);
					return;
				}
			}
		}
	}
}

} // End of namespace Wintermute
