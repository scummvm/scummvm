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

#ifndef BACKENDS_IMGUI_COMPONENTS_IMGUI_LOGGER_H
#define BACKENDS_IMGUI_COMPONENTS_IMGUI_LOGGER_H

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "backends/imgui/imgui.h"

namespace ImGuiEx {

class ImGuiLogger {
	char _inputBuf[256];
	ImVector<char *> _items;
	ImVector<char *> _history;
	int _historyPos; // -1: new line, 0.._history.Size-1 browsing history.
	ImGuiTextFilter _filter;
	bool _autoScroll;
	bool _scrollToBottom;
	bool _showError = true;
	bool _showWarn = true;
	bool _showInfo = true;
	bool _showdebug = true;

	struct {
		ImVec4 _logger_error_b = ImVec4(1.f, 0.f, 0.f, 1.f);
		ImVec4 _logger_warning_b = ImVec4(1.f, 1.f, 0.f, 1.f);
		ImVec4 _logger_info_b = ImVec4(1.f, 1.f, 1.f, 1.f);
		ImVec4 _logger_debug_b = ImVec4(0.8f, 0.8f, 0.8f, 1.f);

		ImVec4 _logger_error = ImVec4(1.0f, 0.4f, 0.4f, 1.0f);
		ImVec4 _logger_warning = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
		ImVec4 _logger_info = ImVec4(1.0f, 0.8f, 0.6f, 1.0f);
		ImVec4 _logger_debug = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
	} _colors;

public:
	ImGuiLogger();
	~ImGuiLogger();
	void clear();
	void addLog(const char *fmt, ...) IM_FMTARGS(2);
	void drawColorOptions();
	void draw(const char *title, bool *p_open);
};

} // namespace ImGuiEx

#endif
