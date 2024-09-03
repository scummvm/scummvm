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

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"

#include "common/archive.h"
#include "common/compression/unzip.h"
#include "common/debug.h"
#include "common/path.h"

#include "qdengine/qdengine.h"
#include "qdengine/debugger/debugtools.h"
#include "qdengine/qdcore/qd_file_manager.h"

namespace QDEngine {

typedef struct ImGuiState {
	bool _showCallStack = false;
	bool _showVars = false;
	bool _showScore = false;
	bool _showArchives = false;
} ImGuiState;

ImGuiState *_state = nullptr;

static void showCallStack() {
}

static void showVars() {
}

void showArchives() {
	if (!_state->_showArchives)
		return;

	// Calculate the viewport size
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	// Calculate the window size
	ImVec2 windowSize = ImVec2(
		viewportSize.x * 0.7f,
		viewportSize.y * 0.7f
	);

	// Calculate the centered position
	ImVec2 centeredPosition = ImVec2(
		(viewportSize.x - windowSize.x) * 0.5f,
		(viewportSize.y - windowSize.y) * 0.5f
	);

	// Set the next window position and size
	ImGui::SetNextWindowPos(centeredPosition, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Archives", &_state->_showArchives)) {
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.3f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		// Iterate through the 3 resource pak files
		for (int i = 0; i < 3; i++) {
			Common::Archive *archive = qdFileManager::instance().get_package(i);
			Common::ArchiveMemberList members;

			if (archive)
				archive->listMembers(members);

			if (archive && ImGui::TreeNode(Common::String::format("Resource/resource%d.pak", i).c_str())) {

				for (auto &it : members) {
					if (ImGui::TreeNode(Common::String::format("%s", transCyrillic(it->getFileName().c_str())).c_str())) {

						if (ImGui::Selectable(Common::String::format("%d", i).c_str())) {
						}

						ImGui::TreePop();
					}
				}

				ImGui::TreePop();
			}
		}

		ImGui::EndChild();
	}
	ImGui::End();
}

static void showScore() {
	if (!_state->_showScore)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Score", &_state->_showScore)) {
		ImGui::Text("WIP");
	}
	ImGui::End();
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();

	static const ImWchar cyrillic_ranges[] = {
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0400, 0x04FF, // Cyrillic
		0
	};

	io.FontDefault = ImGui::addTTFFontFromArchive("FreeSans.ttf", 16.0f, nullptr, cyrillic_ranges);;

	_state = new ImGuiState();
	memset(_state, 0, sizeof(ImGuiState));
}

void onImGuiRender() {
	if (!debugChannelSet(-1, kDebugImGui)) {
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse;
		return;
	}

	if (!_state)
		return;

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("View")) {
			ImGui::SeparatorText("Windows");

			ImGui::MenuItem("CallStack", NULL, &_state->_showCallStack);
			ImGui::MenuItem("Vars", NULL, &_state->_showVars);
			ImGui::MenuItem("Score", NULL, &_state->_showScore);
			ImGui::MenuItem("Archives", NULL, &_state->_showArchives);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showVars();
	showCallStack();
	showScore();
	showArchives();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}

} // namespace QDEngine
