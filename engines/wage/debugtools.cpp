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

ImGuiImage getPatternImage(int fillType) {
	int index = fillType - 1;
	if (index >= 0 && index < (int)_state->_patternTextures.size()) {
		return _state->_patternTextures[index];
	}
	return {0, 0, 0};
}

void drawThicknessIcon(int thickness) {
	ImVec2 pos = ImGui::GetCursorScreenPos();
	ImDrawList *dl = ImGui::GetWindowDrawList();

	// draw Border
	dl->AddRect(pos, pos + ImVec2(16, 16), IM_COL32(255, 255, 255, 255));

	float half = thickness * 0.5f;
	// draw Bar
	dl->AddRectFilled(
		ImVec2(pos.x, pos.y + 8 - half),
		ImVec2(pos.x + 16, pos.y + 8 + half),
		IM_COL32(255, 255, 255, 255));

	ImGui::Dummy(ImVec2(16, 16));
	ImGui::SetItemTooltip("Line thickness: %d", thickness);
}

ImGuiImage getImageID(Designed *d, const char *type, int steps = -1) {
	Common::String key = Common::String::format("%s:%s:%d", d->_name.c_str(), type, steps);

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

	d->_design->paint(surface, *g_wage->_world->_patterns, 0, 0, steps);

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
    ImGui::Image(image.id, size);

	ImGui::GetWindowDrawList()->AddRect(screenPos, screenPos + ImVec2(size.x, size.y), 0xFFFFFFFF);
	ImGui::EndGroup();
}

void showDesignScriptWindow(Design *d) {
	if (!_state->_showScriptWindow)
		return;

	ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Design Script", &_state->_showScriptWindow)) {

		for (uint i = 0; i < d->_drawOps.size(); i++) {
			const DrawOp &op = d->_drawOps[i];
			// check if this is the current step
			bool isCurrent = (_state->_currentStep == i + 1);

			if (isCurrent) {
				ImVec2 pos = ImGui::GetCursorScreenPos();
				float width = ImGui::GetContentRegionAvail().x;
				ImDrawList *dl = ImGui::GetWindowDrawList();

				ImU32 green = IM_COL32(0, 255, 0, 255);
				ImU32 greenTransparent = IM_COL32(0, 255, 0, 50);

				// draw Arrow 
				dl->AddText(pos, green, ICON_MS_ARROW_RIGHT_ALT);

				// draw highlight
				dl->AddRectFilled(
					ImVec2(pos.x + 16.f, pos.y),
					ImVec2(pos.x + width, pos.y + 16.f),
					greenTransparent,
					0.4f);
			}

			// offset text to the right so it doesn't overlap the arrow
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 16.0f);

			// opcode
			ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "[%5d]", op.offset);
			ImGui::SameLine();

			// draw current foreground fill pattern
			if (op.fillType > 0) {
				ImGuiImage patImg = getPatternImage(op.fillType);
				if (patImg.id) {
					showImage(patImg, 1.0f);
					ImGui::SetItemTooltip("Foreground pattern: %d", op.fillType);
					ImGui::SameLine();
				}
			}

			// draw current border fill pattern
			if (op.borderFillType > 0) {
				ImGuiImage patImg = getPatternImage(op.borderFillType);
				if (patImg.id) {
					showImage(patImg, 1.0f);
					ImGui::SetItemTooltip("Border pattern: %d", op.borderFillType);
					ImGui::SameLine();
				}
			}

			// draw line thickness icon
			if (op.borderThickness > 0) {
				drawThicknessIcon(op.borderThickness);
				ImGui::SameLine();
			}

			Common::String label = Common::String::format("%s##%d", op.opcode.c_str(), i);
			if (ImGui::Selectable(label.c_str(), isCurrent)) {
				_state->_currentStep = i + 1;
			}
			if (isCurrent) {
				ImGui::SetItemDefaultFocus();
			}
		}
	}
	ImGui::End();
}

void showDesignViewer(Designed *item, const char *typeStr) {
	if (!item)
		return;

	Design *d = item->_design;
	if (d) {
		if (d->_drawOps.empty()) {
			// force render to populate ops
			Graphics::ManagedSurface tmp;
			tmp.create(1, 1, Graphics::PixelFormat::createFormatCLUT8());
			d->paint(&tmp, *g_wage->_world->_patterns, 0, 0, -1);
			tmp.free();
		}

		uint totalSteps = d->_drawOps.size();

		if (ImGui::Button(ICON_MS_SKIP_PREVIOUS)) {
			if (_state->_currentStep > 1)
				_state->_currentStep--;
			else
				_state->_currentStep = totalSteps;
		}
		ImGui::SameLine();

		if (ImGui::Button(ICON_MS_SKIP_NEXT)) {
			if (_state->_currentStep < totalSteps)
				_state->_currentStep++;
			else
				_state->_currentStep = 1;
		}
		ImGui::SameLine();

		if (ImGui::Button("Script")) {
			_state->_showScriptWindow = !_state->_showScriptWindow;
		}
		ImGui::SameLine();
		ImGui::Text("Step: %d / %d", (_state->_currentStep), totalSteps);

		if (d->getBounds()) {
			ImGuiImage imgID = getImageID(item, typeStr, _state->_currentStep);
			showImage(imgID, 1.0);
		}
		showDesignScriptWindow(d);
	} else {
		ImGui::Text("No Design Data");
	}
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
						for (int n = 0; n < (int)g_wage->_world->_orderedScenes.size(); n++) {
							const bool is_selected = (_state->_selectedScene == n);
							Common::String label = Common::String::format("%s##%d", g_wage->_world->_orderedScenes[n]->_name.c_str(), n);
							if (ImGui::Selectable(label.c_str(), is_selected)) {
								_state->_selectedScene = n;
								_state->_currentStep = 1;
							}

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
							Designed *d = g_wage->_world->_orderedScenes[_state->_selectedScene];
							showDesignViewer(d, "scene");
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
						for (int n = 0; n < (int)g_wage->_world->_orderedObjs.size(); n++) {
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

					showDesignViewer(g_wage->_world->_orderedObjs[_state->_selectedObj], "obj");

					ImGui::EndChild();
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Characters")) {
				{ // Left pane
					ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), ImGuiChildFlags_None);

					if (ImGui::BeginListBox("##listbox characters", ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y))) {
						for (int n = 0; n < (int)g_wage->_world->_orderedChrs.size(); n++) {
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
						for (int n = 0; n < (int)g_wage->_world->_orderedSounds.size(); n++) {
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

	// pre-load patterns into array
	Graphics::MacPatterns &patterns = *g_wage->_world->_patterns;
	for (uint i = 0; i < patterns.size(); ++i) {
		Graphics::ManagedSurface surface(16, 16, Graphics::PixelFormat::createFormatCLUT8());
		Common::Rect rect(16, 16);
		Design::drawFilledRect(&surface, rect, kColorBlack, patterns, i + 1);

		if (surface.surfacePtr()) {
			ImTextureID tex = (ImTextureID)g_system->getImGuiTexture(
				*surface.surfacePtr(),
				g_wage->_gui->_wm->getPalette(),
				g_wage->_gui->_wm->getPaletteSize());

			_state->_patternTextures.push_back({tex, 16, 16});
		} else {
			_state->_patternTextures.push_back({0, 0, 0});
		}
	}
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
