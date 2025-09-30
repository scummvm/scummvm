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
#include "wage/design.h"
#include "wage/gui.h"
#include "wage/script.h"
#include "wage/sound.h"
#include "wage/world.h"

namespace Wage {

ImGuiState *_state = nullptr;

ImGuiImage getImageID(Designed *d, const char *type) {
	Common::String key = Common::String::format("%s:%s", d->_name.c_str(), type);

	if (_state->_images.contains(key))
		return _state->_images[key];

	if (!d->_design)
		return { 0, 0, 0 };

	// We need to precompute dimensions
	Graphics::ManagedSurface tmpsurf(1024, 768, Graphics::PixelFormat::createFormatCLUT8());
	d->_design->paint(&tmpsurf, *g_wage->_world->_patterns, 0, 0);

	int sx = d->_design->getBounds()->width(), sy = d->_design->getBounds()->height();

	if (!sx || !sy)
		return { 0, 0, 0 };

	Graphics::ManagedSurface *surface = new Graphics::ManagedSurface();
	surface->create(sx, sy, Graphics::PixelFormat::createFormatCLUT8());
	surface->fillRect(Common::Rect(0, 0, sx, sy), 4); // white background

	d->_design->paint(surface, *g_wage->_world->_patterns, 0, 0);

	if (surface->surfacePtr()) {
		_state->_images[key] = { (ImTextureID)g_system->getImGuiTexture(*surface->surfacePtr(), g_wage->_gui->_wm->getPalette(), g_wage->_gui->_wm->getPaletteSize()), sx, sy };
	}

	delete surface;

	return _state->_images[key];
}

void showImage(const ImGuiImage &image, float scale) {
	ImVec2 size = { (float)image.width * scale, (float)image.height * scale };

	ImGui::BeginGroup();
	ImVec2 screenPos = ImGui::GetCursorScreenPos();
	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(size.x, size.y), 0xFFFFFFFF);

	ImGui::Image(image.id, size);
	ImGui::EndGroup();
}

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

		if (ImGui::BeginTabBar("MainTabBar", tab_bar_flags)) {
			if (ImGui::BeginTabItem("Scenes")) {

				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox scenes", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < g_wage->_world->_orderedScenes.size(); n++) {
							const bool is_selected = (_state->_selectedScene == n);
							Common::String label = Common::String::format("%s##%d", g_wage->_world->_orderedScenes[n]->_name.c_str(), n);
							if (ImGui::Selectable(label.c_str(), is_selected))
								_state->_selectedScene = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					ImGui::EndChild();
				}

				ImGui::SameLine();

				{ // Right pane
					ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

					if (ImGui::BeginTabBar("SceneTabBar", tab_bar_flags)) {
						if (ImGui::BeginTabItem("Script")) {
							if (g_wage->_world->_orderedScenes[_state->_selectedScene]->_script && g_wage->_world->_orderedScenes[_state->_selectedScene]->_script->_scriptText.size()) {
								for (auto &t : g_wage->_world->_orderedScenes[_state->_selectedScene]->_script->_scriptText) {
									ImGui::Text("[%4d]", t->offset);
									ImGui::SameLine();
									ImGui::Text("%s", t->line.c_str());
								}
							} else {
								ImGui::Text("No script");
							}
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Text")) {
							ImGui::TextWrapped("%s", g_wage->_world->_orderedScenes[_state->_selectedScene]->_text.c_str());
							ImGui::EndTabItem();
						}

						if (ImGui::BeginTabItem("Design")) {
							ImGuiImage imgID = getImageID(g_wage->_world->_orderedScenes[_state->_selectedScene], "obj");

							showImage(imgID, 1.0);
							ImGui::EndTabItem();
						}

						ImGui::EndTabBar();
					}

					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Objects")) {
				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox objects", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < g_wage->_world->_orderedObjs.size(); n++) {
							const bool is_selected = (_state->_selectedObj == n);
							Common::String label = Common::String::format("%s##%d", g_wage->_world->_orderedObjs[n]->_name.c_str(), n);
							if (ImGui::Selectable(label.c_str(), is_selected))
								_state->_selectedObj = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					ImGui::EndChild();
				}

				ImGui::SameLine();

				{ // Right pane
					ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

					ImGuiImage imgID = getImageID(g_wage->_world->_orderedObjs[_state->_selectedObj], "obj");

					showImage(imgID, 1.0);

					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Characters")) {
				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox characters", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < g_wage->_world->_orderedChrs.size(); n++) {
							const bool is_selected = (_state->_selectedChr == n);
							Common::String label = Common::String::format("%s##%d", g_wage->_world->_orderedChrs[n]->_name.c_str(), n);
							if (ImGui::Selectable(label.c_str(), is_selected))
								_state->_selectedChr = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					ImGui::EndChild();
				}

				ImGui::SameLine();

				{ // Right pane
					ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

					ImGuiImage imgID = getImageID(g_wage->_world->_orderedChrs[_state->_selectedChr], "chr");

					showImage(imgID, 1.0);

					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Sounds")) {
				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox sounds", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < g_wage->_world->_orderedSounds.size(); n++) {
							const bool is_selected = (_state->_selectedSound == n);
							Common::String label = Common::String::format("%s##%d", g_wage->_world->_orderedSounds[n]->_name.c_str(), n);
							if (ImGui::Selectable(label.c_str(), is_selected))
								_state->_selectedSound = n;

							// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
							if (is_selected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndListBox();
					}

					ImGui::EndChild();
				}

				ImGui::SameLine();

				{ // Right pane
					ImGui::BeginChild("ChildR", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_Borders);

					ImGui::Text("Sound playback");

					ImGui::EndChild();
				}
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
