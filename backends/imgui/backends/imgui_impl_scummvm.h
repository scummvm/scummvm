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

#pragma once
#include "backends/imgui/imgui.h"      // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE
#include "common/events.h"

struct ImGui_ImplScummVM : public Common::EventObserver {
private:
	Common::EventDispatcher *_eventDispatcher;
	uint64 _time;

public:
	ImGui_ImplScummVM(Common::EventDispatcher *eventDispatcher);
	~ImGui_ImplScummVM();

	/* Common::EventObserver API */
	bool notifyEvent(const Common::Event &event);

	void newFrame();
};

#endif // #ifndef IMGUI_DISABLE
