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
#include "director/debugger/dt-internal.h"

namespace Director {
namespace DT {

Common::String savedStateFileName = Common::String("ImGuiSaveState.json");
const char *WINDOW_LIST[] = {
	"Archive", "Breakpoints", "Vars", "Watched Vars", "Score", "Control Panel",
	"Execution Context", "Functions", "Channels", "Cast", "Settings"
};

// What are the things that need saving?
// 1) Window Positions
// 2) Score frame number
// 3) Windows that are open/closed
// 4) Watched Variables
void saveCurrentState() {
	Common::JSONObject json = Common::JSONObject();

	// Whether windows are open or not
	long long int openFlags = getWindowFlags();
	json["Windows"] = new Common::JSONValue(openFlags);

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

	// Save the JSON
	Common::JSONValue save(json);
	debug("ImGui::Saved state: %s", save.stringify().c_str());

	Common::OutSaveFile *stream = g_engine->getSaveFileManager()->openForSaving(savedStateFileName);

	if (stream) {
		stream->writeString(save.stringify());
		stream->finalize();
		debug("ImGui::SaveCurrentState: Saved the current ImGui State @%s", savedStateFileName.c_str());
	} else {
		debug("ImGui::SaveCurrentState: Failed to open the file %s for saving", savedStateFileName.c_str());
	}

	// Clean up everything
	delete stream;
}

long long int getWindowFlags() {
	long long int openFlags = 0;
	openFlags += (_state->_w.archive) ? 1 : 0;
	openFlags += (_state->_w.bpList) ? 1 << 1 : 0;
	openFlags += (_state->_w.cast) ? 1 << 2 : 0;
	openFlags += (_state->_w.channels) ? 1 << 3 : 0;
	openFlags += (_state->_w.controlPanel) ? 1 << 4 : 0;
	openFlags += (_state->_w.executionContext) ? 1 << 5 : 0;
	openFlags += (_state->_w.funcList) ? 1 << 6: 0;
	openFlags += (_state->_w.logger) ? 1 << 7 : 0;
	openFlags += (_state->_w.score) ? 1 << 8 : 0;
	openFlags += (_state->_w.settings) ? 1 << 9 : 0;
	openFlags += (_state->_w.vars) ? 1 << 10 : 0;
	openFlags += (_state->_w.watchedVars) ? 1 << 11 : 0;

	return openFlags;
}

void loadSavedState() {
	Common::InSaveFile *savedState = g_engine->getSaveFileManager()->openForLoading(savedStateFileName);

	if (!savedState) {
		debug("ImGui::loadSavedState(): Failed to open saved state file: %s", savedStateFileName.c_str());
		return;
	}

	char *data = (char *)malloc(savedState->size());
	savedState->read(data, savedState->size());

	Common::JSONValue *saved = Common::JSON::parse(data);

	if (!saved) {
		debug("ImGui:: Bad JSON: Failed to parse the Saved state");
		return;
	}

	debug("ImGui::loaded state: %s", saved->stringify(true).c_str());

	// Load open/closed window flags
	long long int openFlags = saved->asObject()["Windows"]->asIntegerNumber();
	setWindowFlags(openFlags);

	// Load window settings
	const char *windowSettings = saved->asObject()["Window Settings"]->asString().c_str();
	ImGui::LoadIniSettingsFromMemory(windowSettings);

	// Load the log
	Common::JSONArray log = saved->asObject()["Log"]->asArray();

	if (debugChannelSet(7, kDebugImGui)) {
		debugC(7, kDebugImGui, "Loading log: \n");
		for (auto iter: log) {
			debugC(7, kDebugImGui, "%s", iter->asString().c_str());
		}
	}

	_state->_logger->clear();
	for (auto iter : log) {
		_state->_logger->addLog(iter->asString().c_str());
	}

	free(data);
}

void setWindowFlags(long long int openFlags) {
	_state->_w.archive = (openFlags & 1) ? true : false;
	_state->_w.bpList = (openFlags & 1 << 1) ? true : false;
	_state->_w.cast = (openFlags & 1 << 2) ? true : false;
	_state->_w.channels = (openFlags & 1 << 3) ? true : false;
	_state->_w.controlPanel = (openFlags & 1 << 4) ? true : false;
	_state->_w.executionContext = (openFlags & 1 << 5) ? true : false;
	_state->_w.funcList = (openFlags & 1 << 6) ? true : false;
	_state->_w.logger = (openFlags & 1 << 7) ? true : false;
	_state->_w.score = (openFlags & 1 << 8) ? true : false;
	_state->_w.settings = (openFlags & 1 << 9) ? true : false;
	_state->_w.vars = (openFlags & 1 << 10) ? true : false;
	_state->_w.watchedVars = (openFlags & 1 << 11) ? true : false;
}

}	// End of namespace DT
}	// End of namespace Director
