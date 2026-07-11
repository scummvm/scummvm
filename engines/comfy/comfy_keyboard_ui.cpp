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

#include "comfy/comfy.h"

#ifdef USE_IMGUI
#include "backends/imgui/imgui.h"
#endif

namespace Comfy {

#ifdef USE_IMGUI
static ImVec4 keyboardColor(byte red, byte green, byte blue) {
	return ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f);
}

static ImVec4 keyboardHoverColor(ImVec4 color) {
	color.x += (1.0f - color.x) * 0.15f;
	color.y += (1.0f - color.y) * 0.15f;
	color.z += (1.0f - color.z) * 0.15f;
	return color;
}

static ImVec4 keyboardActiveColor(ImVec4 color) {
	color.x *= 0.82f;
	color.y *= 0.82f;
	color.z *= 0.82f;
	return color;
}

static void keyboardPushButtonColor(ImVec4 color) {
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, keyboardHoverColor(color));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, keyboardActiveColor(color));
}

static void keyboardDrawMomentary(const char *label, uint bit, ImVec2 size, ImVec4 color,
		uint32 &activeMask, uint32 &latchedMask, uint32 &holdMask) {
	uint32 mask = 1 << bit;
	bool held = (holdMask & mask) != 0;
	bool wasActive = (activeMask & mask) != 0;
	keyboardPushButtonColor(held ? keyboardHoverColor(color) : color);
	ImGui::Button(label, size);
	bool active = ImGui::IsItemActive();
	bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
	ImGui::PopStyleColor(3);

	if (active) {
		activeMask |= mask;
		latchedMask &= ~mask;
		if (holdMask & mask)
			holdMask &= ~mask;
		else if (holdMask) {
			latchedMask |= holdMask;
			holdMask = 0;
		}
	} else {
		activeMask &= ~mask;
		if (wasActive)
			latchedMask |= mask;
	}

	if (rightClicked) {
		if (held) {
			holdMask &= ~mask;
			latchedMask |= mask;
		} else {
			holdMask = mask;
		}
	}

	if (held) {
		ImDrawList *draw = ImGui::GetWindowDrawList();
		draw->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 230),
			4.0f, 0, 3.0f);
	}
}

static void keyboardDrawToggle(const char *label, uint bit, ImVec2 size, ImVec4 offColor, ImVec4 onColor,
		uint32 &activeMask, uint32 &latchedMask) {
	uint32 mask = 1 << bit;
	bool active = (activeMask & mask) != 0;
	keyboardPushButtonColor(active ? onColor : offColor);
	bool clicked = ImGui::Button(label, size);
	ImGui::PopStyleColor(3);

	if (clicked) {
		if (active) {
			activeMask &= ~mask;
			latchedMask |= mask;
		} else {
			activeMask |= mask;
			latchedMask &= ~mask;
		}
	}
}
#endif

void ComfyEngine::keyboardUiInitCallback() {
#ifdef USE_IMGUI
	ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
#endif
}

void ComfyEngine::keyboardUiRenderCallback() {
	if (g_engine)
		g_engine->keyboardUiRender();
}

void ComfyEngine::keyboardUiCleanupCallback() {
#ifdef USE_IMGUI
	ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
#endif
}

void ComfyEngine::keyboardUiRender() {
#ifdef USE_IMGUI
	if (!_keyboardUiVisible)
		return;

	ImGui::SetNextWindowSize(ImVec2(780.0f, 520.0f), ImGuiCond_FirstUseEver);
	if (!ImGui::Begin("Comfy Keyboard", &_keyboardUiVisible)) {
		ImGui::End();
		return;
	}

	ImVec4 shell = keyboardColor(232, 228, 208);
	ImVec4 panel = keyboardColor(36, 40, 47);
	ImVec4 red = keyboardColor(199, 31, 27);
	ImVec4 handset = keyboardColor(211, 48, 38);
	ImVec4 blue = keyboardColor(31, 59, 139);
	ImVec4 blueDark = keyboardColor(23, 43, 104);
	ImVec4 green = keyboardColor(41, 123, 58);
	ImVec4 fuchsia = keyboardColor(157, 58, 137);
	ImVec4 orange = keyboardColor(234, 112, 29);
	ImVec4 yellow = keyboardColor(230, 190, 34);

	ImGui::BeginChild("toy_keyboard_body", ImVec2(730.0f, 430.0f), true, ImGuiWindowFlags_NoScrollbar);
	ImDrawList *draw = ImGui::GetWindowDrawList();
	ImVec2 origin = ImGui::GetCursorScreenPos();
	draw->AddRectFilled(origin, ImVec2(origin.x + 700.0f, origin.y + 390.0f),
		ImGui::ColorConvertFloat4ToU32(shell), 20.0f);
	draw->AddRect(origin, ImVec2(origin.x + 700.0f, origin.y + 390.0f), IM_COL32(110, 110, 105, 255),
		20.0f, 0, 2.0f);

	ImGui::SetCursorPos(ImVec2(24.0f, 46.0f));
	keyboardDrawToggle("HANDSET", 13, ImVec2(92.0f, 258.0f), handset, keyboardColor(248, 70, 54),
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask);

	static const char *phoneLabels[] = {"SNAILY", "BUDDY", "JUMPY", "PHILLY", "COMFY"};
	static const uint phoneBits[] = {4, 16, 10, 22, 20};
	for (uint i = 0; i < ARRAYSIZE(phoneBits); i++) {
		ImGui::SetCursorPos(ImVec2(136.0f, 44.0f + i * 58.0f));
		keyboardDrawMomentary(phoneLabels[i], phoneBits[i], ImVec2(74.0f, 42.0f), red,
			_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	}

	ImGui::SetCursorPos(ImVec2(302.0f, 22.0f));
	keyboardDrawMomentary("ROLL", 1, ImVec2(214.0f, 44.0f), green,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);

	ImGui::SetCursorPos(ImVec2(628.0f, 24.0f));
	keyboardDrawMomentary("STOP", 12, ImVec2(58.0f, 58.0f), blueDark,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);

	ImGui::SetCursorPos(ImVec2(702.0f, 82.0f));
	keyboardDrawToggle("ON\nOFF", 7, ImVec2(22.0f, 78.0f), keyboardColor(86, 91, 95),
		keyboardColor(40, 160, 72), _toyKeyboardActiveMask, _toyKeyboardLatchedMask);

	ImGui::SetCursorPos(ImVec2(604.0f, 132.0f));
	keyboardDrawMomentary("SUN", 0, ImVec2(80.0f, 42.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(604.0f, 188.0f));
	keyboardDrawMomentary("MOON", 9, ImVec2(80.0f, 42.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(604.0f, 244.0f));
	keyboardDrawMomentary("CLOUD", 21, ImVec2(80.0f, 42.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);

	ImGui::SetCursorPos(ImVec2(286.0f, 124.0f));
	keyboardDrawMomentary("TRUMPET", 15, ImVec2(86.0f, 66.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(402.0f, 112.0f));
	keyboardDrawMomentary("DRUMS", 3, ImVec2(86.0f, 66.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(518.0f, 124.0f));
	keyboardDrawMomentary("FLUTE", 18, ImVec2(74.0f, 66.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(384.0f, 192.0f));
	keyboardDrawMomentary("PIANO", 6, ImVec2(126.0f, 56.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	ImGui::SetCursorPos(ImVec2(354.0f, 276.0f));
	keyboardDrawMomentary("MUSIC", 23, ImVec2(180.0f, 58.0f), blue,
		_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);

	static const char *colorLabels[] = {"FUCHSIA", "ORANGE", "GREEN", "BLUE", "YELLOW", "RED"};
	static const uint colorBits[] = {5, 8, 17, 14, 11, 2};
	ImVec4 colorValues[] = {fuchsia, orange, green, blue, yellow, red};
	for (uint i = 0; i < ARRAYSIZE(colorBits); i++) {
		ImGui::SetCursorPos(ImVec2(226.0f + i * 72.0f, 346.0f));
		keyboardDrawMomentary(colorLabels[i], colorBits[i], ImVec2(62.0f, 32.0f), colorValues[i],
			_toyKeyboardActiveMask, _toyKeyboardLatchedMask, _toyKeyboardHoldMask);
	}

	ImGui::SetCursorPos(ImVec2(230.0f, 88.0f));
	ImGui::TextColored(green, "Comfy Keyboard");
	ImGui::SetCursorPos(ImVec2(20.0f, 346.0f));
	ImGui::TextColored(panel, "scan 0x%06X", (_toyKeyboardActiveMask | _toyKeyboardHoldMask) & 0x00FFFFFF);
	ImGui::EndChild();
	ImGui::End();
#endif
}

} // End of namespace Comfy
