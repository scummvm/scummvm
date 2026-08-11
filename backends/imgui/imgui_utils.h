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

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "backends/imgui/imgui.h"
#include "graphics/palette.h"

namespace ImGuiEx {

template<typename INT>
bool InputInt(const char *label, INT *v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0) {
	int tmp = (int)*v;
	if (ImGui::InputInt(label, &tmp, step, step_fast, flags)) {
		*v = (INT)tmp;
		return true;
	}
	return false;
}

void Boolean(bool val);
void Palette(const Graphics::Palette &palette);
bool toggleButton(const char *label, bool *p_value, bool inverse = false);

} // namespace ImGuiEx
