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

#ifndef BLADERUNNER_UI_SLIDER_H
#define BLADERUNNER_UI_SLIDER_H

#include "bladerunner/color.h"
#include "bladerunner/ui/ui_component.h"

#include "common/rect.h"

namespace BladeRunner {

class UISlider : public UIComponent {
	static const Color256 kColors[];

	UIComponentCallback *_valueChangedCallback;
	void                *_callbackData;
	bool                 _isEnabled;
	int                  _maxValue;
	Common::Rect         _rect;
	int                  _currentFrameColor;
	bool                 _hasFocus;
	int                  _pressedStatus;
	int                  _mouseX;

public:
	int                  _value;

	UISlider(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int maxValue, int value);

	void draw(Graphics::Surface &surface) override;

	void handleMouseMove(int mouseX, int mouseY) override;
	void handleMouseDown(bool alternateButton) override;
	void handleMouseUp(bool alternateButton) override;

	void setValue(int value);
};

} // End of namespace BladeRunner

#endif
