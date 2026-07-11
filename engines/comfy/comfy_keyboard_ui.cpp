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

typedef struct ImGuiState {
	ComfyEngine *_engine = nullptr;
	uint32 _activeMask = 0;
	uint32 _latchedMask = 0;
	uint32 _holdMask = 0;
	bool _visible = true;
} ImGuiState;

ImGuiState *_state = nullptr;

static ImU32 keyboardColor(byte red, byte green, byte blue) {
	return IM_COL32(red, green, blue, 255);
}

static ImU32 keyboardHoverColor(ImU32 color) {
	ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color);
	colorVec.x += ((1 - colorVec.x) * 0.15F);
	colorVec.y += ((1 - colorVec.y) * 0.15F);
	colorVec.z += ((1 - colorVec.z) * 0.15F);

	return ImGui::ColorConvertFloat4ToU32(colorVec);
}

static ImU32 keyboardActiveColor(ImU32 color) {
	ImVec4 colorVec = ImGui::ColorConvertU32ToFloat4(color);
	colorVec.x *= 0.82F;
	colorVec.y *= 0.82F;
	colorVec.z *= 0.82F;

	return ImGui::ColorConvertFloat4ToU32(colorVec);
}

static void keyboardPushButtonColor(ImU32 color) {
	ImGui::PushStyleColor(ImGuiCol_Button, color);
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, keyboardHoverColor(color));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, keyboardActiveColor(color));
}

static void keyboardDrawMomentary(const char *label, uint bit, ImVec2 size, ImU32 color) {
	uint32 mask = 1 << bit;
	bool held = (_state->_holdMask & mask) != 0;
	bool wasActive = (_state->_activeMask & mask) != 0;

	keyboardPushButtonColor(held ? keyboardHoverColor(color) : color);
	ImGui::Button(label, size);
	bool active = ImGui::IsItemActive();
	bool rightClicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
	ImGui::PopStyleColor(3);

	if (active) {
		_state->_activeMask |= mask;
		_state->_latchedMask &= ~mask;

		if (_state->_holdMask & mask) {
			_state->_holdMask &= ~mask;
		} else if (_state->_holdMask) {
			_state->_latchedMask |= _state->_holdMask;
			_state->_holdMask = 0;
		}
	} else {
		_state->_activeMask &= ~mask;

		if (wasActive)
			_state->_latchedMask |= mask;
	}

	if (rightClicked) {
		if (held) {
			_state->_holdMask &= ~mask;
			_state->_latchedMask |= mask;
		} else {
			_state->_holdMask = mask;
		}
	}

	if (held) {
		ImDrawList *draw = ImGui::GetWindowDrawList();
		draw->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), IM_COL32(255, 255, 255, 230), 4, 0, 3);
	}
}

static void keyboardDrawToggle(const char *label, uint bit, ImVec2 size, ImU32 offColor, ImU32 onColor) {
	uint32 mask = 1 << bit;
	bool active = (_state->_activeMask & mask) != 0;

	keyboardPushButtonColor(active ? onColor : offColor);
	bool clicked = ImGui::Button(label, size);
	ImGui::PopStyleColor(3);

	if (clicked) {
		if (active) {
			_state->_activeMask &= ~mask;
			_state->_latchedMask |= mask;
		} else {
			_state->_activeMask |= mask;
			_state->_latchedMask &= ~mask;
		}
	}
}

void onImGuiInit() {
	ImGuiIO &io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	_state = new ImGuiState();
	_state->_engine = (ComfyEngine *)g_engine;
}

void onImGuiRender() {
	if (!_state || !_state->_engine || _state->_engine->shouldQuit())
		return;

	if (!_state->_visible) {
		_state->_engine->setToyKeyboardState(0, _state->_latchedMask | _state->_activeMask, 0);
		_state->_activeMask = 0;
		_state->_latchedMask = 0;
		_state->_holdMask = 0;
		return;
	}

	ImGui::GetIO().ConfigFlags &= ~(ImGuiConfigFlags_NoMouseCursorChange | ImGuiConfigFlags_NoMouse);

	ImGui::SetNextWindowPos(ImVec2(8, 8), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(740, 440), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Comfy Keyboard", &_state->_visible, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize)) {
		ImU32 shell = keyboardColor(232, 228, 208);
		ImU32 red = keyboardColor(199, 31, 27);
		ImU32 handset = keyboardColor(211, 48, 38);
		ImU32 blue = keyboardColor(31, 59, 139);
		ImU32 blueDark = keyboardColor(23, 43, 104);
		ImU32 green = keyboardColor(41, 123, 58);
		ImU32 purple = keyboardColor(157, 58, 137);
		ImU32 orange = keyboardColor(234, 112, 29);
		ImU32 yellow = keyboardColor(230, 190, 34);

		ImGui::BeginChild("toy_keyboard_body", ImVec2(730, 430), ImGuiChildFlags_AlwaysUseWindowPadding, ImGuiWindowFlags_NoScrollbar);

		ImDrawList *draw = ImGui::GetWindowDrawList();
		ImVec2 origin = ImGui::GetCursorScreenPos();

		draw->AddRectFilled(origin, ImVec2(origin.x + 700, origin.y + 390), shell, 20);
		draw->AddRect(origin, ImVec2(origin.x + 700, origin.y + 390), keyboardColor(110, 110, 105), 20, 0, 2);

		ImGui::SetCursorPos(ImVec2(24, 46));
		keyboardDrawToggle("HANDSET", 13, ImVec2(92, 258), handset, keyboardColor(248, 70, 54));

		static const char *phoneLabels[] = {"SNAILY", "BUDDY", "JUMPY", "FEELY", "COMFY"};
		static const uint phoneBits[] = {4, 16, 10, 22, 20};

		for (uint i = 0; i < ARRAYSIZE(phoneBits); i++) {
			ImGui::SetCursorPos(ImVec2(136, 44 + i * 58));
			keyboardDrawMomentary(phoneLabels[i], phoneBits[i], ImVec2(74, 42), red);
		}

		ImGui::SetCursorPos(ImVec2(302, 22));
		keyboardDrawMomentary("ROLL", 1, ImVec2(214, 44), green);

		ImGui::SetCursorPos(ImVec2(628, 24));
		keyboardDrawMomentary("STOP", 12, ImVec2(58, 58), blueDark);

		ImGui::SetCursorPos(ImVec2(702, 82));
		keyboardDrawToggle("ON\nOFF", 7, ImVec2(22, 78), keyboardColor(86, 91, 95), keyboardColor(40, 160, 72));

		ImGui::SetCursorPos(ImVec2(604, 132));
		keyboardDrawMomentary("SUN", 0, ImVec2(80, 42), blue);

		ImGui::SetCursorPos(ImVec2(604, 188));
		keyboardDrawMomentary("MOON", 9, ImVec2(80, 42), blue);

		ImGui::SetCursorPos(ImVec2(604, 244));
		keyboardDrawMomentary("CLOUD", 21, ImVec2(80, 42), blue);

		ImGui::SetCursorPos(ImVec2(286, 124));
		keyboardDrawMomentary("TRUMPET", 15, ImVec2(86, 66), blue);

		ImGui::SetCursorPos(ImVec2(402, 112));
		keyboardDrawMomentary("DRUMS", 3, ImVec2(86, 66), blue);

		ImGui::SetCursorPos(ImVec2(518, 124));
		keyboardDrawMomentary("FLUTE", 18, ImVec2(74, 66), blue);

		ImGui::SetCursorPos(ImVec2(384, 192));
		keyboardDrawMomentary("PIANO", 6, ImVec2(126, 56), blue);

		ImGui::SetCursorPos(ImVec2(354, 276));
		keyboardDrawMomentary("MUSIC", 23, ImVec2(180, 58), blue);

		static const char *colorLabels[] = {"PURPLE", "ORANGE", "GREEN", "BLUE", "YELLOW", "RED"};
		static const uint colorBits[] = {5, 8, 17, 14, 11, 2};
		ImU32 colorValues[] = {purple, orange, green, blue, yellow, red};

		for (uint i = 0; i < ARRAYSIZE(colorBits); i++) {
			ImGui::SetCursorPos(ImVec2(226 + i * 72, 346));
			keyboardDrawMomentary(colorLabels[i], colorBits[i], ImVec2(62, 32), colorValues[i]);
		}

		ImGui::SetCursorPos(ImVec2(230, 88));
		ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(green), "Comfy Keyboard");

		ImGui::SetCursorPos(ImVec2(20, 346));
		ImGui::Text("scan 0x%06X", (_state->_activeMask | _state->_holdMask) & 0x00FFFFFF);

		ImGui::EndChild();
	}

	ImGui::End();
	_state->_engine->setToyKeyboardState(_state->_activeMask, _state->_latchedMask, _state->_holdMask);
	_state->_latchedMask = 0;
}

void onImGuiCleanup() {
	if (_state && _state->_engine)
		_state->_engine->setToyKeyboardState(0, 0, 0);

	delete _state;
	_state = nullptr;
}

#endif // USE_IMGUI

} // End of namespace Comfy
