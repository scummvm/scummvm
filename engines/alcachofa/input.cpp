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

#include "alcachofa/input.h"
#include "alcachofa/alcachofa.h"
#include "alcachofa/metaengine.h"

using namespace Common;

namespace Alcachofa {

void Input::nextFrame() {
	if (_debugInput != nullptr)
		return _debugInput->nextFrame();

	_wasMouseLeftPressed = false;
	_wasMouseRightPressed = false;
	_wasMouseLeftReleased = false;
	_wasMouseRightReleased = false;
	_wasMenuKeyPressed = false;
	_wasInventoryKeyPressed = false;
	_wasSubtitlesKeyPressed = false;
	updateMousePos3D(); // camera transformation might have changed
}

bool Input::handleEvent(const Common::Event &event) {
	if (_debugInput != nullptr) {
		auto result = _debugInput->handleEvent(event);
		_mousePos2D = _debugInput->mousePos2D(); // even for debug input we want to e.g. draw a cursor
		_mousePos3D = _debugInput->mousePos3D();
		return result;
	}

	switch (event.type) {
	case EVENT_LBUTTONDOWN:
		_wasMouseLeftPressed = true;
		_isMouseLeftDown = true;
		return true;
	case EVENT_LBUTTONUP:
		_wasMouseLeftReleased = true;
		_isMouseLeftDown = false;
		return true;
	case EVENT_RBUTTONDOWN:
		_wasMouseRightPressed = true;
		_isMouseRightDown = true;
		return true;
	case EVENT_RBUTTONUP:
		_wasMouseRightReleased = true;
		_isMouseRightDown = false;
		return true;
	case EVENT_MOUSEMOVE: {
		_mousePos2D = event.mouse;
		updateMousePos3D();
		return true;
	case EVENT_CUSTOM_ENGINE_ACTION_START:
		switch ((EventAction)event.customType) {
		case EventAction::InputMenu:
			_wasMenuKeyPressed = true;
			return true;
		case EventAction::InputInventory:
			_wasInventoryKeyPressed = true;
			return true;
		case EventAction::InputSubtitles:
			_wasSubtitlesKeyPressed = true;
			return true;
		default:
			return false;
		}
	}
	default:
		return false;
	}
}

void Input::toggleDebugInput(bool debugMode) {
	if (!debugMode) {
		_debugInput.reset();
		return;
	}
	nextFrame(); // resets frame-specific flags
	_isMouseLeftDown = _isMouseRightDown = false;
	if (_debugInput == nullptr)
		_debugInput.reset(new Input());
}

void Input::updateMousePos3D() {
	auto pos3D = g_engine->camera().transform2Dto3D({ (float)_mousePos2D.x, (float)_mousePos2D.y, kBaseScale });
	_mousePos3D = { (int16)pos3D.x(), (int16)pos3D.y() };
}

struct WaitForInputTask final : public Task {
	WaitForInputTask(Process &process) : Task(process) {}

	WaitForInputTask(Process &process, Serializer &s) : Task(process) {}

	TaskReturn run() override {
		TASK_BEGIN;

		// originally this would unlock interaction
		do {
			TASK_YIELD(1);
		} while(!g_engine->input().wasAnyMousePressed());
		TASK_END;
	}

	void debugPrint() override {
		g_engine->getDebugger()->debugPrintf("Wait for input");
	}

	const char *taskName() const override;
};
DECLARE_TASK(WaitForInputTask)

Task *Input::waitForInput(Process &process) {
	return new WaitForInputTask(process);
}

}
