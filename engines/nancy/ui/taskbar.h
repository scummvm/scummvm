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

	void toggleButton(uint index, bool enabled);

	// Returns the index of the button that was clicked this frame, or -1
	// if none. Cleared on the next call to handleInput().
	int getClickedButton() const { return _clickedButton; }

private:
	enum ButtonState {
		kButtonIdle     = 0,
		kButtonHover    = 1,
		kButtonPressed  = 2,
		kButtonDisabled = 3
	};

	void drawButton(uint index, ButtonState state);

	Graphics::ManagedSurface _backgroundImage; // TASK::imageName (e.g. "Frame")
	Graphics::ManagedSurface _buttonImage;     // buttons' primaryImageName (e.g. "UIShared_OVL")
	int _hoveredButton;
	int _clickedButton;
	ButtonState _buttonStates[5];
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TASKBAR_H
