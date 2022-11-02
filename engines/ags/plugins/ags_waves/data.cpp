/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "common/util.h"
#include "ags/plugins/ags_waves/ags_waves.h"

namespace AGS3 {
namespace Plugins {
namespace AGSWaves {

void AGSWaves::SaveVariable(ScriptMethodParams &params) {
	PARAMS2(const char *, value, int, id);

	if (GameDatavalue[id] != nullptr) {
		free(GameDatavalue[id]);
	}
	if (value != nullptr) {
		GameDatavalue[id] = scumm_strdup(value);
	} else {
		GameDatavalue[id] = nullptr;
	}
}

void AGSWaves::ReadVariable(ScriptMethodParams &params) {
	PARAMS1(int, id);

	if (GameDatavalue[id] == nullptr) {
		params._result = _engine->CreateScriptString("");
	} else {
		params._result = _engine->CreateScriptString(GameDatavalue[id]);
	}
}

void AGSWaves::GameDoOnceOnly(ScriptMethodParams &params) {
	//	PARAMS1(const char *, value);

	GetGDState(params);
	if (params._result) {
		// Set state to false
		params.push_back(false);
		SetGDState(params);

		params._result = true;
	} else {
		params._result = false;
	}
}

void AGSWaves::SetGDState(ScriptMethodParams &params) {
	PARAMS2(const char *, value, bool, setValue);

	int id = -1;
	for (int i = 0; i <= usedTokens; i++) {
		if (Token[i] != nullptr && strcmp(Token[i], value) == 0) {
			id = i;
			TokenUnUsed[i] = setValue;
			i = usedTokens + 1;
		}
	}

	if (id == -1) {
		// It doesn't find it while trying to set its state
		// create the thing with said state
		id = usedTokens;
		TokenUnUsed[id] = setValue;
		if (Token[id] != nullptr)
			free(Token[id]);

		Token[id] = scumm_strdup(value);
		usedTokens++;
	}
}

void AGSWaves::GetGDState(ScriptMethodParams &params) {
	PARAMS1(const char *, value);

	int id = -1;

	for (int i = 0; i <= usedTokens; i++) {
		if (Token[i] != nullptr && strcmp(Token[i], value) == 0) {
			id = i;
			i = usedTokens + 1;
		}
	}

	if (id == -1) {
		params._result = true;
	} else {
		params._result = TokenUnUsed[id];
	}
}

void AGSWaves::ResetAllGD(ScriptMethodParams &params) {
	for (int i = 0; i <= usedTokens; i++) {
		if (Token[i] != nullptr)
			free(Token[i]);
		Token[i] = nullptr;
		TokenUnUsed[i] = true;
	}
	usedTokens = 0;
}

void AGSWaves::GetWalkbehindBaserine(ScriptMethodParams &params) {
	PARAMS1(int, id);
	params._result = Walkbehind[id];
}

void AGSWaves::SetWalkbehindBaserine(ScriptMethodParams &params) {
	PARAMS2(int, id, int, base);
	Walkbehind[id] = base;
}

} // namespace AGSWaves
} // namespace Plugins
} // namespace AGS3
