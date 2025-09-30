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
#define IMGUI_DEFINE_MATH_OPERATORS

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"

#include "common/debug.h"
#include "common/system.h"

#include "graphics/managed_surface.h"

#include "wage/wage.h"
#include "wage/dt-internal.h"
#include "wage/script.h"
#include "wage/world.h"

namespace Wage {

ImGuiState *_state = nullptr;

ImGuiImage getImageID(Common::Path filename, int frameNum) {
	Common::String key = Common::String::format("%s:%d", filename.toString().c_str(), frameNum);

	if (_state->_frames.contains(key))
		return _state->_frames[key];

	int sx = 10, sy = 10;
	Graphics::ManagedSurface *surface = nullptr;

	if (surface)
		_state->_frames[key] = { (ImTextureID)g_system->getImGuiTexture(*surface->surfacePtr()), sx, sy };

	delete surface;

	return _state->_frames[key];
}

#if 0
void showImage(const ImGuiImage &image, const char *name, float scale) {
	ImVec2 size = { (float)image.width * scale, (float)image.height * scale };

	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(size.x, size.y), 0xFFFFFFFF);

	ImGui::Image(image.id, size);
	ImGui::EndGroup();
	//setToolTipImage(image, name);
}

static void displayTGA() {
	ImGuiImage imgID;

	imgID = getImageID(_state->_fileToDisplay, 0);

	ImGui::Text("TGA %s: [%d x %d]", transCyrillic(_state->_fileToDisplay.toString()), imgID.width, imgID.height);

	ImGui::Separator();

	showImage(imgID, (char *)transCyrillic(_state->_fileToDisplay.toString()), 1.0);
}

void showArchives() {
	if (!_state->_showArchives)
		return;

	// Calculate the viewport size
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	// Calculate the window size
	ImVec2 windowSize = ImVec2(
		viewportSize.x * 0.9f,
		viewportSize.y * 0.9f
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
		ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

		ImGui::Button(ICON_MS_FILTER_ALT);
		ImGui::SameLine();

		_state->_nameFilter.Draw();
		ImGui::Separator();

		if (_state->_files.children.empty())
			populateFileList();

		displayTree(&_state->_files);

		ImGui::EndChild();

		ImGui::SameLine();

		{ // Right pane
			ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

			if (_state->_displayMode == kDisplayQDA) {
				displayQDA();
			} else if (_state->_displayMode == kDisplayTGA) {
				displayTGA();
			}

			ImGui::EndChild();
		}

	}
	ImGui::End();
}

void showSceneObjects() {
	if (!_state->_showSceneObjects)
		return;

	ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scene Objects", &_state->_showSceneObjects)) {
		qdGameScene *scene;
		qdGameDispatcher *dp = qdGameDispatcher::get_dispatcher();
		if (dp && ((scene = dp->get_active_scene()))) {
			if (!scene->object_list().empty()) {
				for (auto &it : g_engine->_visible_objects) {
					if (ImGui::Selectable((char *)transCyrillic(it->name()), _state->_objectToDisplay == it->name())) {
						_state->_objectToDisplay = it->name();
					}
				}
			}
		}
	}
	ImGui::End();
}
#endif

static void showWorld() {
	if (!_state->_showWorld)
		return;

	// Calculate the viewport size
	ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

	// Calculate the window size
	ImVec2 windowSize = ImVec2(
		viewportSize.x * 0.9f,
		viewportSize.y * 0.9f
	);

	// Calculate the centered position
	ImVec2 centeredPosition = ImVec2(
		(viewportSize.x - windowSize.x) * 0.5f,
		(viewportSize.y - windowSize.y) * 0.5f
	);

	// Set the next window position and size
	ImGui::SetNextWindowPos(centeredPosition, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);

	if (ImGui::Begin("World", &_state->_showWorld)) {
		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;

		if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
			if (ImGui::BeginTabItem("Scenes")) {

				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox scenes", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < g_wage->_world->_orderedScenes.size(); n++) {
							const bool is_selected = (_state->_selectedScene == n);
							if (ImGui::Selectable(g_wage->_world->_orderedScenes[n]->_name.c_str(), is_selected))
								_state->_selectedScene = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					ImGui::EndChild();
				}

				{ // Right pane
					ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

					ImGui::Text("Resource");

					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Objects")) {
				ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Characters")) {
				ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Sounds")) {
				ImGui::Text("This is the Sounds tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Global Script")) {
				for (auto &t : g_wage->_world->_globalScript->_scriptText) {
					ImGui::Text("[%4d]", t->offset);
					ImGui::SameLine();
					ImGui::Text("%s", t->line.c_str());
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("World")) {
				ImGui::Text("This is the Global Script tab!\nblah blah blah blah blah");
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	ImFontConfig icons_config;
	icons_config.MergeMode = true;
	icons_config.PixelSnapH = false;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;
	icons_config.GlyphOffset = {0, 4};

	static const ImWchar icons_ranges[] = {ICON_MIN_MS, ICON_MAX_MS, 0};
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &icons_config, icons_ranges);

	_state = new ImGuiState();
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
			ImGui::MenuItem("World", NULL, &_state->_showWorld);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	showWorld();
}

void onImGuiCleanup() {
	delete _state;
	_state = nullptr;
}

} // namespace Wage
