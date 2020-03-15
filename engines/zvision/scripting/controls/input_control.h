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
#include "zvision/text/text.h"
#include "zvision/text/string_manager.h"

#include "common/rect.h"

namespace Video {
	class VideoDecoder;
}

namespace ZVision {

class InputControl : public Control {
public:
	InputControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream);
	~InputControl() override;

private:
	Graphics::Surface *_background;
	Common::Rect _textRectangle;
	Common::Rect _headerRectangle;
	TextStyleState _stringInit;
	TextStyleState _stringChooserInit;
	uint32 _nextTabstop;
	bool _focused;

	Common::String _currentInputText;
	bool _textChanged;
	bool _enterPressed;
	bool _readOnly;

	int16 _txtWidth;
	int16 _maxTxtWidth;
	Video::VideoDecoder *_animation;

public:
	void focus() override;
	void unfocus() override;
	bool onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) override;
	bool onKeyDown(Common::KeyState keyState) override;
	bool process(uint32 deltaTimeInMillis) override;
	void setText(const Common::String &_str);
	const Common::String getText();
	bool enterPress();
	void setReadOnly(bool);
};

} // End of namespace ZVision

#endif
