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

#ifndef DIRECTOR_DEBUGER_DT_INTERNAL_H
#define DIRECTOR_DEBUGER_DT_INTERNAL_H

#define IMGUI_DEFINE_MATH_OPERATORS

#include "backends/imgui/imgui.h"
#include "backends/imgui/imgui_fonts.h"

namespace Director {

namespace DT {

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

public:
	ImGuiLogger();
	~ImGuiLogger();
	void clear();
	void addLog(const char *fmt, ...) IM_FMTARGS(2);
	void draw(const char *title, bool *p_open);
};

bool toggleButton(const char *label, bool *p_value, bool inverse = false);

}

};

#endif
