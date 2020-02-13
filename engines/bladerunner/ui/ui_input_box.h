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

#ifndef BLADERUNNER_ui_inputbox_H
#define BLADERUNNER_ui_inputbox_H

#include "bladerunner/ui/ui_component.h"

#include "common/rect.h"
#include "common/str.h"

namespace BladeRunner {

class UIInputBox : public UIComponent {
	UIComponentCallback *_valueChangedCallback;
	void                *_callbackData;

	bool                 _isVisible;
	Common::Rect         _rect;

	uint                 _maxLength;
	Common::String       _text;

	bool                 _cursorIsVisible;

	uint32               _timeLast;

public:
	UIInputBox(BladeRunnerEngine *vm, UIComponentCallback *valueChangedCallback, void *callbackData, Common::Rect rect, int maxLength, const Common::String &text);

	void draw(Graphics::Surface &surface) override;

	void setText(const Common::String &text);
	const Common::String &getText();

	void show();
	void hide();

	void handleKeyDown(const Common::KeyState &kbd) override;

private:
	bool charIsValid(const Common::KeyState &kbd);
};

} // End of namespace BladeRunner

#endif
