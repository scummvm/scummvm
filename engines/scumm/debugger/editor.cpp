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

#include "common/config-manager.h"
#include "common/formats/json.h"
#include "common/savefile.h"

#include "scumm/scumm.h"

#include "scumm/debugger/editor.h"

#define ICON_EDITOR   ICON_MS_CONSTRUCTION
#define ICON_SETTINGS ICON_MS_SETTINGS

namespace Scumm {

static const char *saveStateFileName = "ImGuiSaveState.json";

static const char *colorNames[] = {"Label", "Property", "Warning", "Error"};

ScummEditor::ScummEditor(ScummEngine *engine)
	: _engine(engine),
	  _gameName(ConfMan.get("gameid")),
	  _showSettings(false) {
	// Specify default colors
	_colors.resize(Editor::kColorCount);
	_colors[Editor::kColorLabel] = ImVec4(0.149f, 0.545f, 0.824f, 1.0f);    // SOL_BLUE
	_colors[Editor::kColorProperty] = ImVec4(0.514f, 0.580f, 0.588f, 1.0f); // SOL_BASE0
	_colors[Editor::kColorWarning] = ImVec4(0.710f, 0.537f, 0.000f, 1.0f);  // SOL_YELLOW
	_colors[Editor::kColorError] = ImVec4(0.863f, 0.196f, 0.184f, 1.0f);    // SOL_RED

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

	// Load colors
	if (state->asObject().contains("Colors")) {
		const Common::JSONObject &colors = state->asObject()["Colors"]->asObject();
		for (uint i = 0; i < _colors.size(); ++i) {
			if (colors.contains(colorNames[i])) {
				const Common::JSONArray &arr = colors[colorNames[i]]->asArray();
				_colors[i].x = (float)arr[0]->asNumber();
				_colors[i].y = (float)arr[1]->asNumber();
				_colors[i].z = (float)arr[2]->asNumber();
				_colors[i].w = (float)arr[3]->asNumber();
			}
		}
	}

	// Load ImGui layout
	if (state->asObject().contains("IniSettings")) {
		const char *iniSettings = state->asObject()["IniSettings"]->asString().c_str();
		ImGui::LoadIniSettingsFromMemory(iniSettings);
	}

	// Load ImGui colors
	if (state->asObject().contains("ImGuiColors")) {
		const Common::JSONObject &obj = state->asObject()["ImGuiColors"]->asObject();
		ImGuiStyle &style = ImGui::GetStyle();
		for (int i = 0; i < ImGuiCol_COUNT; ++i) {
			const char *name = ImGui::GetStyleColorName(i);
			if (obj.contains(name)) {
				const Common::JSONArray &arr = obj[name]->asArray();
				style.Colors[i].x = (float)arr[0]->asNumber();
				style.Colors[i].y = (float)arr[1]->asNumber();
				style.Colors[i].z = (float)arr[2]->asNumber();
				style.Colors[i].w = (float)arr[3]->asNumber();
			}
		}
	}

	delete state;
}

void ScummEditor::saveState() {
	Common::JSONObject json;

	// Save colors
	Common::JSONObject colors;
	for (uint i = 0; i < _colors.size(); ++i) {
		Common::JSONArray arr;
		arr.push_back(new Common::JSONValue((double)_colors[i].x));
		arr.push_back(new Common::JSONValue((double)_colors[i].y));
		arr.push_back(new Common::JSONValue((double)_colors[i].z));
		arr.push_back(new Common::JSONValue((double)_colors[i].w));
		colors[colorNames[i]] = new Common::JSONValue(arr);
	}
	json["Colors"] = new Common::JSONValue(colors);

	// Save layout
	const char *iniSettings = ImGui::SaveIniSettingsToMemory();
	json["IniSettings"] = new Common::JSONValue(iniSettings);

	// Save ImGui colors
	ImGuiStyle &style = ImGui::GetStyle();
	Common::JSONObject imguiColors;
	for (int i = 0; i < ImGuiCol_COUNT; ++i) {
		Common::JSONArray arr;
		arr.push_back(new Common::JSONValue((double)style.Colors[i].x));
		arr.push_back(new Common::JSONValue((double)style.Colors[i].y));
		arr.push_back(new Common::JSONValue((double)style.Colors[i].z));
		arr.push_back(new Common::JSONValue((double)style.Colors[i].w));
		imguiColors[ImGui::GetStyleColorName(i)] = new Common::JSONValue(arr);
	}
	json["ImGuiColors"] = new Common::JSONValue(imguiColors);

	// Write to save file
	Common::JSONValue state(json);
	Common::OutSaveFile *stream = g_engine->getSaveFileManager()->openForSaving(saveStateFileName);
	if (stream) {
		stream->writeString(state.stringify());
		stream->finalize();
	}
	delete stream;
}

void ScummEditor::showSettings() {
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
	if (ImGui::Begin(ICON_SETTINGS " Settings", &_showSettings, ImGuiWindowFlags_NoDocking)) {
		// General settings
		if (ImGui::CollapsingHeader("General", ImGuiTreeNodeFlags_DefaultOpen))
			for (uint i = 0; i < _colors.size(); ++i)
				ImGui::ColorEdit4(colorNames[i], &_colors[i].x);

		// ImGui settings
		if (ImGui::CollapsingHeader("ImGui", ImGuiTreeNodeFlags_DefaultOpen)) {
			ImGuiStyle &style = ImGui::GetStyle();
			for (int i = 0; i < ImGuiCol_COUNT; ++i)
				ImGui::ColorEdit4(ImGui::GetStyleColorName(i), &style.Colors[i].x);
		}
	}
	ImGui::End();
}

void ScummEditor::render() {
	ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

	// Menu bar
	if (ImGui::BeginMainMenuBar()) {
		ImGui::Text(ICON_EDITOR);
		ImGui::TextDisabled(_gameName.c_str());
		ImGui::Separator();
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("Quit"))
				_engine->quitGame();
			ImGui::EndMenu();
		}
		if (ImGui::BeginMenu("View")) {
			ImGui::MenuItem(ICON_SETTINGS " Settings", nullptr, &_showSettings);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	// Settings window
	if (_showSettings)
		showSettings();

	saveState();
}

} // End of namespace Scumm
