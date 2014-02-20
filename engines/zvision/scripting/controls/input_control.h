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

#ifndef ZVISION_INPUT_CONTROL_H
#define ZVISION_INPUT_CONTROL_H

#include "zvision/scripting/control.h"
#include "zvision/strings/string_manager.h"

#include "common/rect.h"


namespace ZVision {

class InputControl : public Control {
public:
	InputControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);

private:
	Common::Rect _textRectangle;
	Common::Rect _headerRectangle;
	StringManager::TextStyle _textStyle;
	uint32 _nextTabstop;
	Common::String _cursorAnimationFileName;
	bool _focused;

	Common::String _currentInputText;
	bool _textChanged;
	uint _cursorOffset;

public:
	void focus() { _focused = true; }
	void unfocus() { _focused = false; }
	void onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos);
	void onKeyDown(Common::KeyState keyState);
	bool process(uint32 deltaTimeInMillis);
};

} // End of namespace ZVision

#endif
