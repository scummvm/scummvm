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

#ifndef NANCY_UI_TASKBAR_H
#define NANCY_UI_TASKBAR_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

// Always-on bottom-of-screen strip introduced in Nancy 10.
// Holds 5 buttons (MENU / inventory / notebook / cellphone / HELP) that
// open the various popup UIs handled by Scene.
class Taskbar : public RenderObject {
public:
	Taskbar();
	virtual ~Taskbar() = default;

	void init() override;
	void registerGraphics() override;
	void handleInput(NancyInput &input);

	// Enable / disable a taskbar button. A disabled button is rendered
	// in its disabled sprite and ignores clicks.
	void toggleButton(uint index, bool enabled);

	// Disable override: keep the disabled sprite active while the current
	// scene is in [startScene, endScene]. Driven by AR 29 (ControlUIItems,
	// _flagB != 0).
	void setDisabledRange(uint buttonIndex, int16 startScene, int16 endScene);
	void clearButtonOverride(uint buttonIndex);

	// Notification badge: each button has up to 3 independent notification
	// sub-categories. The badge shows when any sub-category is set; it
	// persists across scene changes until cleared (or all sub-cats are
	// cleared individually). Disabled state takes precedence over the badge.
	void setNotification(uint buttonIndex, uint subCategory);
	void clearNotification(uint buttonIndex, uint subCategory);
	void clearAllNotifications(uint buttonIndex);

	// Re-evaluate which buttons should currently show their override
	// sprite. Call after a scene change so the range check kicks in.
	void updateNotificationStates(int16 currentSceneID);

	// Returns the index of the button that was clicked this frame, or -1
	// if none. Cleared on the next call to handleInput().
	int getClickedButton() const { return _clickedButton; }

private:
	enum ButtonState {
		kButtonIdle         = 0,
		kButtonHover        = 1,
		kButtonPressed      = 2,
		kButtonDisabled     = 3,   // not clickable
		kButtonNotification = 4    // popup has new content (badge sprite)
	};

	// A scene-ranged disable override for one button. While active and the
	// current scene is within [startScene, endScene] the button renders in
	// the disabled state.
	struct ButtonOverride {
		bool active = false;
		int16 startScene = -1;
		int16 endScene = -1;
	};

	static const uint kNumNotificationSubCategories = 3;

	void drawButton(uint index, ButtonState state);
	ButtonState restingState(uint index) const;
	// True when the button currently accepts hover/click (not disabled).
	bool isButtonActive(uint index) const;

	Graphics::ManagedSurface _backgroundImage; // TASK::imageName (e.g. "Frame")
	Graphics::ManagedSurface _buttonImage;     // buttons' primaryImageName (e.g. "UIShared_OVL")
	int _hoveredButton;
	int _clickedButton;
	int16 _currentScene;
	bool _enabled[5];
	ButtonState _buttonStates[5];
	ButtonOverride _overrides[5];
	bool _notifications[5][kNumNotificationSubCategories];
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TASKBAR_H
