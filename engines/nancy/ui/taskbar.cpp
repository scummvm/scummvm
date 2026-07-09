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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

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

// "NO_UI_ITEM" marks an absent button slot — e.g. Nancy12's removed cell phone,
// whose on-screen position the coin purse occupies instead. Such slots are neither
// drawn nor hit-tested.
static bool isButtonSlotUsed(const TASK::ButtonRecord &rec) {
	return !rec.button.primaryImageName.empty() &&
		!rec.button.primaryImageName.toString().equalsIgnoreCase("NO_UI_ITEM");
}

void Taskbar::drawButton(uint index, ButtonState state) {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	const TASK::ButtonRecord &rec = taskData->buttons[index];
	if (!isButtonSlotUsed(rec)) {
		return;
	}

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
	// While a popup is open every button is disabled.
	if (_popupLockout) {
		return kButtonDisabled;
	}
	if (!_enabled[index]) {
		return kButtonDisabled;
	}
	// Disable override (ControlUIItems) takes precedence over the badge —
	// the notify sprite is only drawn when the button is not disabled.
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
	auto *taskData = GetEngineData(TASK);
	if (!taskData || !isButtonSlotUsed(taskData->buttons[index])) {
		return false;
	}
	return _enabled[index] && restingState(index) != kButtonDisabled;
}

bool Taskbar::isMoneyDisplay(uint index) const {
	return g_nancy->getGameType() == kGameTypeNancy12 && index == kTaskButtonCoinPurse;
}

void Taskbar::drawMoney() {
	auto *taskData = GetEngineData(TASK);
	const UIRC *uirc = GetEngineData(UIRC);
	if (!taskData || !uirc || uirc->items.empty()) {
		return;
	}

	// The coin purse displays UI resource 0: its current value rendered with a
	// '$' prefix and `unknown2` decimal places. Old Clock tracks cents
	// (decimals 2), so a value of 350 shows as "$3.50". `unknown1` selects the
	// font. The live value lives in the scene state (seeded from UIRC, changed
	// by AR 132); UIRC only supplies the formatting config.
	const UIRC::ItemRecord &res = uirc->items[0];
	if (res.unknown2 < 1) {
		return;
	}
	const int32 value = NancySceneState.getUIResource(0);
	const Common::String text =
		Common::String::format("$%d.%02d", value / 100, value % 100);

	const Font *font = g_nancy->_graphics->getFont(res.unknown1);
	if (!font) {
		return;
	}

	Common::Rect dst = taskData->buttons[kTaskButtonCoinPurse].button.destRect;
	dst.translate(-_screenPosition.left, -_screenPosition.top);

	// Position matches the original: a small inset from the left, and a little
	// below the button's vertical centre.
	const int x = dst.left + 12;
	const int y = dst.top + dst.height() / 2 + 10;
	font->drawString(&_drawSurface, text, x, y, dst.right - x, 0, Graphics::kTextAlignLeft);
	_needsRedraw = true;
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

void Taskbar::setPopupLockout(bool locked) {
	if (_popupLockout == locked) {
		return;
	}
	_popupLockout = locked;

	// Repaint every button in its new resting state (all disabled while locked,
	// back to idle/badge when the popup closes) and drop any lingering hover.
	_hoveredButton = -1;
	auto *taskData = GetEngineData(TASK);
	if (!taskData) {
		return;
	}
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		if (isButtonSlotUsed(taskData->buttons[i])) {
			drawButton(i, restingState(i));
		}
	}
}

void Taskbar::setNotification(uint buttonIndex, uint subCategory) {
	if (buttonIndex >= TASK::kNumButtons || subCategory >= kNumNotificationSubCategories) {
		return;
	}
	_notifications[buttonIndex][subCategory] = true;
	persistNotifications(buttonIndex);

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::clearNotification(uint buttonIndex, uint subCategory) {
	if (buttonIndex >= TASK::kNumButtons || subCategory >= kNumNotificationSubCategories) {
		return;
	}
	_notifications[buttonIndex][subCategory] = false;
	persistNotifications(buttonIndex);

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
	persistNotifications(buttonIndex);

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
	persistOverride(buttonIndex);

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::clearButtonOverride(uint buttonIndex) {
	if (buttonIndex >= TASK::kNumButtons) {
		return;
	}
	_overrides[buttonIndex].active = false;
	_overrides[buttonIndex].clickSoundMode = kClickSoundDefault;
	persistOverride(buttonIndex);

	if ((int)buttonIndex != _hoveredButton) {
		drawButton(buttonIndex, restingState(buttonIndex));
	}
}

void Taskbar::setClickSoundMode(uint buttonIndex, uint mode) {
	if (buttonIndex >= TASK::kNumButtons) {
		return;
	}
	_overrides[buttonIndex].clickSoundMode = mode;
	persistOverride(buttonIndex);
}

void Taskbar::persistOverride(uint index) {
	if (index >= TASK::kNumButtons || index >= TaskbarData::kNumButtons) {
		return;
	}
	TaskbarData *data = (TaskbarData *)NancySceneState.getPuzzleData(TaskbarData::getTag());
	if (!data) {
		return;
	}
	data->overrides[index].active = _overrides[index].active;
	data->overrides[index].startScene = _overrides[index].startScene;
	data->overrides[index].endScene = _overrides[index].endScene;
	data->overrides[index].clickSoundMode = (uint16)_overrides[index].clickSoundMode;
}

void Taskbar::persistNotifications(uint index) {
	if (index >= TASK::kNumButtons || index >= TaskbarData::kNumButtons) {
		return;
	}
	TaskbarData *data = (TaskbarData *)NancySceneState.getPuzzleData(TaskbarData::getTag());
	if (!data) {
		return;
	}
	for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
		data->notifications[index][s] = _notifications[index][s];
	}
}

void Taskbar::syncFromPuzzleData() {
	TaskbarData *data = (TaskbarData *)NancySceneState.getPuzzleData(TaskbarData::getTag());
	if (!data) {
		return;
	}
	for (uint i = 0; i < TASK::kNumButtons && i < TaskbarData::kNumButtons; ++i) {
		_overrides[i].active = data->overrides[i].active;
		_overrides[i].startScene = data->overrides[i].startScene;
		_overrides[i].endScene = data->overrides[i].endScene;
		_overrides[i].clickSoundMode = data->overrides[i].clickSoundMode;

		for (uint s = 0; s < kNumNotificationSubCategories; ++s) {
			_notifications[i][s] = data->notifications[i][s];
		}
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

void Taskbar::playClickSound(uint index) {
	auto *taskData = GetEngineData(TASK);
	if (!taskData) {
		return;
	}
	// A normal click plays the button's own click sound ("Click01").
	const SoundDescription &clickSound = taskData->buttons[index].button.clickSound;
	g_nancy->_sound->loadSound(clickSound);
	g_nancy->_sound->playSound(clickSound);
}

void Taskbar::playRejectionSound(uint index) {
	auto *taskData = GetEngineData(TASK);
	if (!taskData) {
		return;
	}

	const uint mode = _overrides[index].clickSoundMode;
	if (mode == kClickSoundSilent) {
		return;
	}

	// Clicking a disabled button plays a "popup unavailable" line from
	// clickSoundName, over the clickSound's channel/volume. The mode forces a
	// specific line (kClickSound0..2) or picks a random valid one; an empty /
	// "NO SOUND" slot leaves the default click sound.
	const TASK::ButtonRecord &rec = taskData->buttons[index];
	SoundDescription sound = rec.button.clickSound;
	Common::String chosen;

	if (mode >= kClickSound0 && mode <= kClickSound2) {
		const Common::String &n = rec.clickSoundName[mode - kClickSound0];
		if (!n.empty() && !n.equalsIgnoreCase("NO SOUND")) {
			chosen = n;
		}
	} else {
		Common::String validAlts[TASK::kNumAltSounds];
		uint numValid = 0;
		for (uint s = 0; s < TASK::kNumAltSounds; ++s) {
			const Common::String &n = rec.clickSoundName[s];
			if (!n.empty() && !n.equalsIgnoreCase("NO SOUND")) {
				validAlts[numValid++] = n;
			}
		}
		if (numValid > 0) {
			chosen = validAlts[g_nancy->_randomSource->getRandomNumber(numValid - 1)];
		}
	}

	if (!chosen.empty()) {
		sound.name = chosen;
	}
	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

void Taskbar::handleInput(NancyInput &input) {
	auto *taskData = GetEngineData(TASK);
	assert(taskData);

	_clickedButton = -1;

	// Hit-test present slots. Enabled buttons get full hover/press/click;
	// disabled buttons are still clickable, but only to play their "popup
	// unavailable" rejection sound.
	int newHovered = -1;
	bool hoveredActive = false;
	for (uint i = 0; i < TASK::kNumButtons; ++i) {
		if (!isButtonSlotUsed(taskData->buttons[i])) {
			continue;
		}
		if (taskData->buttons[i].button.destRect.contains(input.mousePos)) {
			newHovered = i;
			hoveredActive = isButtonActive(i);
			break;
		}
	}

	// Update hover graphic on enter/exit. The previously-hovered button returns
	// to its resting sprite; only enabled buttons swap to the hover sprite.
	if (newHovered != _hoveredButton) {
		if (_hoveredButton != -1) {
			drawButton(_hoveredButton, restingState(_hoveredButton));
		}
		if (newHovered != -1 && hoveredActive) {
			// A pending notification badge takes priority over the hover sprite,
			// so a badged button keeps its badge while hovered.
			const ButtonState hoverState =
				restingState(newHovered) == kButtonNotification ? kButtonNotification : kButtonHover;
			drawButton(newHovered, hoverState);
			if (isMoneyDisplay(newHovered)) {
				drawMoney();
			}
		}
		_hoveredButton = newHovered;
	}

	if (newHovered == -1) {
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

	// The Nancy12 coin purse shows Nancy's money on hover but isn't clickable, so
	// it skips the press/click handling below.
	if (isMoneyDisplay(newHovered)) {
		return;
	}

	// Disabled button: a click just plays the rejection sound, no popup.
	if (!hoveredActive) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			playRejectionSound(newHovered);
			input.eatMouseInput();
		}
		return;
	}

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

		playClickSound(newHovered);

		input.eatMouseInput();
	}
}

} // End of namespace UI
} // End of namespace Nancy
