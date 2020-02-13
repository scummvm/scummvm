/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef BLADERUNNER_UI_CHECKBOX_H
#define BLADERUNNER_UI_CHECKBOX_H

#include "bladerunner/ui/ui_component.h"

#include "common/rect.h"

namespace BladeRunner {

class UICheckBox : public UIComponent {
	UIComponentCallback *_valueChangedCallback;
	void                *_callbackData;

	int                  _style;
	int                  _isEnabled;
	Common::Rect         _rect;
	uint32               _frame;
	int                  _isPressed;
	uint32               _timeLast;
	int                  _hasFocus;

public:
	bool                 _isChecked;

	UICheckBox(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int style, bool isChecked);

	void draw(Graphics::Surface &surface) override;

	void enable();
	void disable();
	void setChecked(bool isChecked);

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool alternateButton) override;
	void handleMouseUp(bool alternateButton) override;
};

} // End of namespace BladeRunner

#endif
