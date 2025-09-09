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

#include "backends/imgui/imgui_utils.h"
#include "common/formats/json.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/file.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/debugger/dt-internal.h"

namespace Director {
namespace DT {

const char *savedStateFileName = "ImGuiSaveState.json";

Common::Array<WindowFlag> getWindowFlags() {
	return {
		{ "Archive",			&_state->_w.archive			 },
		{ "Breakpoints",		&_state->_w.bpList			 },
		{ "Cast",				&_state->_w.cast			 },
		{ "Channels",			&_state->_w.channels		 },
		{ "Control Panel",		&_state->_w.controlPanel	 },
		{ "Execution Context",	&_state->_w.executionContext },
		{ "Functions",			&_state->_w.funcList		 },
		{ "Log",				&_state->_w.logger			 },
		{ "Score",				&_state->_w.score			 },
		{ "Settings",			&_state->_w.settings		 },
		{ "Vars",				&_state->_w.vars			 },
		{ "Watched Vars",		&_state->_w.watchedVars		 },
	};
}

// What are the things that need saving?
// 1) Window Positions
// 2) Score frame number
// 3) Windows that are open/closed
// 4) Watched Variables
void saveCurrentState() {
	Common::JSONObject json = Common::JSONObject();

	// Whether windows are open or not
	Common::Array<WindowFlag> windows = getWindowFlags();

	uint index = 0;
	int64 openFlags = 0;
	for (const WindowFlag &it : windows) {
		openFlags += (*it.flag) ? 1 << index : 0;
		index += 1;
	}
	if (debugChannelSet(7, kDebugImGui)) {
		debugC(7, kDebugImGui, "Window flags: ");
		for (auto it : windows) {
			debug("%s: %s", it.name, *it.flag ? "open" : "closed");
		}
	}

	json["Windows"] = new Common::JSONValue((long long int)openFlags);

	// Window Settings
	const char *windowSettings = ImGui::SaveIniSettingsToMemory();
	json["Window Settings"] = new Common::JSONValue(windowSettings);

	// Current Log
	ImVector<char *> currentLog = _state->_logger->getItems();
	Common::JSONArray log;
	for (auto iter : currentLog) {
		log.push_back(new Common::JSONValue(iter));
	}
	json["Log"] = new Common::JSONValue(log);

	// Other settings
	json["ScoreWindow"] = new Common::JSONValue(_state->_scoreWindow);
	json["ChannelsWindow"] = new Common::JSONValue(_state->_channelsWindow);
	json["CastWindow"] = new Common::JSONValue(_state->_castWindow);

	// Save the JSON
	Common::JSONValue save(json);
	debugC(7, kDebugImGui, "ImGui::Saved state: %s", save.stringify().c_str());

	Common::OutSaveFile *stream = g_engine->getSaveFileManager()->openForSaving(savedStateFileName);

	if (stream) {
		stream->writeString(save.stringify());
		stream->finalize();
		debug("ImGui::SaveCurrentState: Saved the current ImGui State @%s", savedStateFileName);
	} else {
		debug("ImGui::SaveCurrentState: Failed to open the file %s for saving", savedStateFileName);
	}

	// Clean up everything
	delete stream;
}

void loadSavedState() {
	Common::InSaveFile *savedState = g_engine->getSaveFileManager()->openForLoading(savedStateFileName);

	if (!savedState || savedState->size() == 0) {
		debug("ImGui::loadSavedState(): Failed to open saved state file: %s", savedStateFileName);
		return;
	}

	// ASAN throws an error if the data is exactly equal to savedState->size() in JSON::parse()
	// Probably because JSON::parse() expects a NULL terminated data
	char *data = (char *)malloc(savedState->size() + 1);
	data[savedState->size()] = '\0';
	savedState->read(data, savedState->size());

	Common::JSONValue *saved = Common::JSON::parse(data);

	if (!saved) {
		debug("ImGui:: Bad JSON: Failed to parse the Saved state");
		free(data);
		return;
	}

	debugC(7, kDebugImGui, "ImGui::loaded state: %s", saved->stringify(true).c_str());

	// Load open/closed window flags
	int64 openFlags = saved->asObject()["Windows"]->asIntegerNumber();
	Common::Array<WindowFlag> windows = getWindowFlags();

	uint index = 0;
	for (WindowFlag it : windows) {
		*it.flag = (openFlags & 1 << index) ? true : false;
		index += 1;
	}
	_state->_w.archive = (openFlags & 1) ? true : false;
	if (debugChannelSet(7, kDebugImGui)) {
		debugC(7, kDebugImGui, "Window flags: ");
		for (auto it : windows) {
			debug("%s: %s", it.name, *it.flag ? "open" : "closed");
		}
	}

	// Load window settings
	const char *windowSettings = saved->asObject()["Window Settings"]->asString().c_str();
	ImGui::LoadIniSettingsFromMemory(windowSettings);

	// Load the log
	Common::JSONArray log = saved->asObject()["Log"]->asArray();

	if (debugChannelSet(7, kDebugImGui)) {
		debugC(7, kDebugImGui, "Loading log: \n");
		for (auto iter : log) {
			debugC(7, kDebugImGui, "%s", iter->asString().c_str());
		}
	}

	_state->_logger->clear();
	for (auto iter : log) {
		_state->_logger->addLog(iter->asString().c_str());
	}

	// Load other settings
	_state->_scoreWindow = saved->asObject()["ScoreWindow"]->asString();
	_state->_channelsWindow = saved->asObject()["ChannelsWindow"]->asString();
	_state->_castWindow = saved->asObject()["CastWindow"]->asString();

	free(data);
	delete saved;
	delete savedState;
}

}	// End of namespace DT
}	// End of namespace Director
