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
		_clickedButton(-1) {
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		_buttonStates[i] = kButtonIdle;
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

	const UIButtonRecord &btn = taskData->buttons[index].button;

	Common::Rect src = btn.sourceRects[state];
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

void Taskbar::toggleButton(uint index, bool enabled) {
	drawButton(index, enabled ? kButtonIdle : kButtonDisabled);
}

void Taskbar::handleInput(NancyInput &input) {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	_clickedButton = -1;

	int newHovered = -1;
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		if (taskData->buttons[i].button.destRect.contains(input.mousePos)) {
			newHovered = i;
			break;
		}
	}

	// Update hover graphic on enter/exit. Always revert the previously-
	// hovered button (even from kButtonPressed) so it never gets stuck
	// after the cursor leaves.
	if (newHovered != _hoveredButton) {
		if (_hoveredButton != -1) {
			drawButton(_hoveredButton, kButtonIdle);
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
