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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/debugtools.h"

namespace Director {

typedef struct ImGuiState {
	bool _showCallStack;
	bool _showVars;
	bool _showScore;
} ImGuiState;

ImGuiState *_state = nullptr;

static void showCallStack() {
	if (!_state->_showCallStack)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 160), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("CallStack", &_state->_showCallStack)) {
		ImGui::Text("%s", lingo->formatCallStack(lingo->_state->pc).c_str());
	}
	ImGui::End();
}

static void showVars() {
	if (!_state->_showVars)
		return;

	Director::Lingo *lingo = g_director->getLingo();
	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(120, 120), ImGuiCond_FirstUseEver);
	if (ImGui::Begin("Vars", &_state->_showVars)) {
		ImGui::Text("%s", lingo->formatAllVars().c_str());
		ImGui::Separator();
		ImGuiIO &io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
			ImGui::MenuItem("CallStack", NULL, &_state->_showCallStack);
			ImGui::MenuItem("Vars", NULL, &_state->_showVars);
			ImGui::MenuItem("Score", NULL, &_state->_showScore);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showVars();
	showCallStack();
	showScore();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}
} // namespace Director
