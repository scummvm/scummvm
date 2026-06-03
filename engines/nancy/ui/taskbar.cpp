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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/ui/taskbar.h"

namespace Nancy {
namespace UI {

Taskbar::Taskbar() :
		RenderObject(7),
		_hoveredButton(-1),
		_clickedButton(-1),
		_currentScene(-1) {
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		_buttonStates[i] = kButtonIdle;
		_enabled[i] = true;
		for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
			_notifications[i][s] = false;
		}
	}
}

void Taskbar::init() {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	g_nancy->_resource->loadImage(taskData->imageName, _backgroundImage);
	g_nancy->_resource->loadImage(taskData->buttons[0].button.primaryImageName, _buttonImage);

	// Draw the taskbar strip itself (background) into the draw surface.
	moveTo(taskData->dstRect);

	_drawSurface.create(_screenPosition.width(), _screenPosition.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.blitFrom(_backgroundImage, taskData->srcRect, Common::Point(0, 0));

	// Draw each button in its idle state on top of the strip
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		drawButton(i, kButtonIdle);
	}

	setTransparent(false);

	RenderObject::init();
}

void Taskbar::registerGraphics() {
	RenderObject::registerGraphics();
	setVisible(true);
}

void Taskbar::drawButton(uint index, ButtonState state) {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	const TASK::ButtonRecord &rec = taskData->buttons[index];
	const UIButtonRecord &btn = rec.button;

	// The notification sprite lives outside the standard sourceRects
	// array, in its own per-button rect.
	Common::Rect src = (state == kButtonNotification)
		? rec.notificationSrcRect
		: btn.sourceRects[state];
	if (src.isEmpty())
		src = btn.sourceRects[kButtonIdle];
	if (src.isEmpty())
		return;

	// Convert the on-screen DEST rect to taskbar-local coordinates.
	Common::Rect dst = btn.destRect;
	dst.translate(-_screenPosition.left, -_screenPosition.top);

	// Restore the background under the button before drawing the new state,
	// so a hover/pressed sprite blends correctly (and idle erases any
	// previous hover/pressed sprite). srcBackgroundRestore is in absolute
	// screen coords, same as destRect — they match in shipped data.
	if (!btn.srcBackgroundRestore.isEmpty())
		_drawSurface.blitFrom(_backgroundImage, btn.srcBackgroundRestore, Common::Point(dst.left, dst.top));

	_drawSurface.blitFrom(_buttonImage, src, Common::Point(dst.left, dst.top));
	_buttonStates[index] = state;
	_needsRedraw = true;
}

Taskbar::ButtonState Taskbar::restingState(uint index) const {
	if (index >= TASK::kNumButtons) {
		return kButtonIdle;
	}
	if (!_enabled[index]) {
		return kButtonDisabled;
	}
	// Disable override (ControlUIItems) takes precedence over the badge —
	// FUN_004d51c3 only draws the notify sprite when state != 3.
	const ButtonOverride &o = _overrides[index];
	if (o.active && _currentScene >= o.startScene && _currentScene <= o.endScene) {
		return kButtonDisabled;
	}
	for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
		if (_notifications[index][s]) {
			return kButtonNotification;
		}
	}
	return kButtonIdle;
}

bool Taskbar::isButtonActive(uint index) const {
	return _enabled[index] && restingState(index) != kButtonDisabled;
}

void Taskbar::toggleButton(uint index, bool enabled) {
	if (index >= TASK::kNumButtons) {
		return;
	}
	_enabled[index] = enabled;
	if ((int)index != _hoveredButton) {
		drawButton(index, restingState(index));
	}
}

void Taskbar::setNotification(uint buttonIndex, uint subCategory) {
	if (buttonIndex >= TASK::kNumButtons || subCategory >= kNumNotificationSubCategories) {
		return;
	}
	_notifications[buttonIndex][subCategory] = true;

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::clearNotification(uint buttonIndex, uint subCategory) {
	if (buttonIndex >= TASK::kNumButtons || subCategory >= kNumNotificationSubCategories) {
		return;
	}
	_notifications[buttonIndex][subCategory] = false;

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::clearAllNotifications(uint buttonIndex) {
	if (buttonIndex >= TASK::kNumButtons) {
		return;
	}
	for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
		_notifications[buttonIndex][s] = false;
	}

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::setDisabledRange(uint buttonIndex, int16 startScene, int16 endScene) {
	if (buttonIndex >= TASK::kNumButtons) {
		return;
	}
	_overrides[buttonIndex].active = true;
	_overrides[buttonIndex].startScene = startScene;
	_overrides[buttonIndex].endScene = endScene;

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::clearButtonOverride(uint buttonIndex) {
	if (buttonIndex >= TASK::kNumButtons) {
		return;
	}
	_overrides[buttonIndex].active = false;

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::updateNotificationStates(int16 currentSceneID) {
	_currentScene = currentSceneID;
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		if ((int)i == _hoveredButton) {
			continue;
		}
		const ButtonState desired = restingState(i);
		if (_buttonStates[i] != desired) {
			drawButton(i, desired);
		}
	}
}

void Taskbar::handleInput(NancyInput &input) {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	_clickedButton = -1;

	int newHovered = -1;
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		if (isButtonActive(i) && taskData->buttons[i].button.destRect.contains(input.mousePos)) {
			newHovered = i;
			break;
		}
	}

	// Update hover graphic on enter/exit. The previously-hovered button
	// returns to its resting sprite (idle or notification) so it doesn't
	// get stuck in hover/pressed after the cursor leaves.
	if (newHovered != _hoveredButton) {
		if (_hoveredButton != -1) {
			drawButton(_hoveredButton, restingState(_hoveredButton));
		}
		if (newHovered != -1) {
			drawButton(newHovered, kButtonHover);
		}
		_hoveredButton = newHovered;
	}

	if (newHovered == -1) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		// Mouse pressed: show the pressed sprite for the duration of the hold.
		if (_buttonStates[newHovered] != kButtonPressed) {
			drawButton(newHovered, kButtonPressed);
		}
	} else if (input.input & NancyInput::kLeftMouseButtonUp) {
		// Mouse released over the button: trigger the click action and
		// snap the sprite back to hover (the cursor is still over it).
		// Acknowledging the click also clears any pending notifications
		// for this button — the popup will read them on entry.
		for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
			_notifications[newHovered][s] = false;
		}
		drawButton(newHovered, kButtonHover);
		_clickedButton = newHovered;

		// Play the first available click sound for this button. The TASK
		// chunk stores up to three alternates per button; if the slot is
		// empty we silently skip.
		const Common::String &snd = taskData->buttons[newHovered].clickSoundName[0];
		if (!snd.empty()) {
			g_nancy->_sound->playSound(snd);
		}

		input.eatMouseInput();
	}
}

} // End of namespace UI
} // End of namespace Nancy
