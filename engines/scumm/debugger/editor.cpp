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

#include "backends/imgui/IconsMaterialSymbols.h"
#include "backends/imgui/imgui.h"

#include "common/config-manager.h"
#include "common/formats/json.h"
#include "common/savefile.h"

#include "scumm/scumm.h"

#include "scumm/debugger/editor.h"

namespace Scumm {

static const char *saveStateFileName = "ImGuiSaveState.json";

ScummEditor::ScummEditor(ScummEngine *engine)
	: _engine(engine),
	  _gameName(ConfMan.get("gameid")) {
	loadState();
}

void ScummEditor::loadState() {
	// Read from save file
	Common::InSaveFile *stream = g_engine->getSaveFileManager()->openForLoading(saveStateFileName);
	if (!stream || stream->size() == 0) {
		delete stream;
		return;
	}

	// Parse JSON
	char *data = (char *)malloc(stream->size() + 1);
	data[stream->size()] = '\0';
	stream->read(data, stream->size());
	Common::JSONValue *state = Common::JSON::parse(data);
	free(data);
	delete stream;
	if (!state)
		return;

	// Load state
	if (state->asObject().contains("IniSettings")) {
		const char *iniSettings = state->asObject()["IniSettings"]->asString().c_str();
		ImGui::LoadIniSettingsFromMemory(iniSettings);
	}
	delete state;
}

void ScummEditor::saveState() {
	// Save state
	Common::JSONObject json;
	const char *iniSettings = ImGui::SaveIniSettingsToMemory();
	json["IniSettings"] = new Common::JSONValue(iniSettings);

	// Write to save file
	Common::JSONValue state(json);
	Common::OutSaveFile *stream = g_engine->getSaveFileManager()->openForSaving(saveStateFileName);
	if (stream) {
		stream->writeString(state.stringify());
		stream->finalize();
	}
	delete stream;
}

void ScummEditor::render() {
	// Menu bar
	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text(ICON_MS_CONSTRUCTION);
		ImGui::TextDisabled(_gameName.c_str());
		ImGui::Separator();
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Quit"))
				_engine->quitGame();
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	saveState();
}

} // End of namespace Scumm
