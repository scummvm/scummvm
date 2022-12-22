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

#ifndef WINTERMUTE_SXSTEAMAPI_H
#define WINTERMUTE_SXSTEAMAPI_H

#include "engines/achievements.h"
#include "wintermute/base/base_scriptable.h"

namespace Wintermute {

class SXSteamAPI : public BaseScriptable {
public:
	DECLARE_PERSISTENT(SXSteamAPI, BaseScriptable)
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	SXSteamAPI(BaseGame *inGame, ScStack *stack);
	~SXSteamAPI() override;

private:
	void init();

	Common::AchievementsInfo _achievementsInfo;
};

Common::AchievementsInfo getAchievementsInfo();
Common::String getAchievementMessage(const Common::AchievementsInfo &info, const char *id);

} // End of namespace Wintermute

#endif
