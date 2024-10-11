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

namespace ImGuiEx {

void Boolean(bool val) {
	if (val) {
		ImGui::Text(ICON_MS_CHECK_BOX);
	} else {
		ImGui::Text(ICON_MS_CHECK_BOX_OUTLINE_BLANK);
	}
}

void Palette(const Graphics::Palette &palette) {
	ImDrawList *drawList = ImGui::GetWindowDrawList();
	const ImDrawListFlags backupFlags = drawList->Flags;
	drawList->Flags &= ~ImDrawListFlags_AntiAliasedLines;
	const ImVec2 &pos = ImGui::GetCursorScreenPos();
	for (uint palettePanelIdx = 0; palettePanelIdx < palette.size(); ++palettePanelIdx) {
		uint8 r, g, b;
		palette.get(palettePanelIdx, r, g, b);
		const float borderWidth = 1.0f;
		const ImVec2 availableInner = ImGui::GetContentRegionAvail();
		const float contentRegionWidth = availableInner.x + ImGui::GetCursorPosX();
		const ImVec2 colorButtonSize(ImGui::GetFrameHeight(), ImGui::GetFrameHeight());
		ImVec2 globalCursorPos = ImGui::GetCursorScreenPos();
		const ImVec2 &windowPos = ImGui::GetWindowPos();
		const ImVec2 v1(globalCursorPos.x + borderWidth, globalCursorPos.y + borderWidth);
		const ImVec2 v2(globalCursorPos.x + colorButtonSize.x, globalCursorPos.y + colorButtonSize.y);
		drawList->AddRectFilled(v1, v2, IM_COL32(r, g, b, 255));

		ImGui::PushID((int)palettePanelIdx);
		if (ImGui::InvisibleButton("", colorButtonSize)) {
		}
		ImGui::SetItemTooltip("Index: %i, R(%d), G(%d), B(%d)", (int)palettePanelIdx, (int)r, (int)g, (int)b);
		ImGui::PopID();

		globalCursorPos.x += colorButtonSize.x;
		if (globalCursorPos.x > windowPos.x + contentRegionWidth - colorButtonSize.x) {
			globalCursorPos.x = pos.x;
			globalCursorPos.y += colorButtonSize.y;
		}
		ImGui::SetCursorScreenPos(globalCursorPos);
	}
	// restore the draw list flags from above
	drawList->Flags = backupFlags;
}

bool toggleButton(const char *label, bool *p_value, bool inverse) {
	int pop = 0;
	if (*p_value != inverse) {
		ImVec4 hovered = ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered];
		ImGui::PushStyleColor(ImGuiCol_Button, hovered);
		pop = 1;
	}
	bool result = ImGui::Button(label);
	if (result) {
		*p_value = !*p_value;
	}
	ImGui::PopStyleColor(pop);
	return result;
}

} // namespace ImGuiEx
