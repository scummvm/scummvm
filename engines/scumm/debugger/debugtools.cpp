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
#include "backends/imgui/imgui_fonts.h"

#include "scumm/scumm.h"

#include "scumm/debugger/editor.h"

namespace Scumm {

namespace Editor {

ScummEditor *g_editor = nullptr;

void onImGuiInit() {
	// Add built-in default font
	ImGuiIO &io = ImGui::GetIO();
	io.Fonts->AddFontDefault();

	// Merge icon font
	ImFontConfig iconConfig;
	iconConfig.MergeMode = true;
	iconConfig.PixelSnapH = false;
	iconConfig.OversampleH = 3;
	iconConfig.OversampleV = 3;
	iconConfig.GlyphOffset = {0, 4};
	static const ImWchar iconRanges[] = {ICON_MIN_MS, ICON_MAX_MS, 0};
	ImGui::addTTFFontFromArchive("MaterialSymbolsSharp.ttf", 16.f, &iconConfig, iconRanges);

	g_editor = new ScummEditor(g_scumm);
}

void onImGuiRender() {
	g_editor->render();
}

void onImGuiCleanup() {
	delete g_editor;
	g_editor = nullptr;
}

} // End of namespace Editor

} // End of namespace Scumm
