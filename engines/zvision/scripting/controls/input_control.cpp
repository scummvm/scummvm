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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#include "zvision/scripting/controls/input_control.h"
#include "zvision/cursors/cursor_manager.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/utility/utility.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/rect.h"


namespace ZVision {

InputControl::InputControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_INPUT),
	  _nextTabstop(0),
	  _focused(false),
	  _textChanged(false),
	  _cursorOffset(0),
	  _enterPressed(false),
	  _readOnly(false),
	  _txtWidth(0),
	  _animation(NULL) {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	trimCommentsAndWhiteSpace(&line);

	while (!stream.eos() && !line.contains('}')) {
		if (line.matchString("*rectangle*", true)) {
			int x1;
			int y1;
			int x2;
			int y2;

			sscanf(line.c_str(), "%*[^(](%d %d %d %d)", &x1, &y1, &x2, &y2);

			_textRectangle = Common::Rect(x1, y1, x2, y2);
		} else if (line.matchString("*aux_hotspot*", true)) {
			int x1;
			int y1;
			int x2;
			int y2;

			sscanf(line.c_str(), "%*[^(](%d %d %d %d)", &x1, &y1, &x2, &y2);

			_headerRectangle = Common::Rect(x1, y1, x2, y2);
		} else if (line.matchString("*string_init*", true)) {
			uint fontFormatNumber;

			sscanf(line.c_str(), "%*[^(](%u)", &fontFormatNumber);

			_string_init.readAllStyle(_engine->getStringManager()->getTextLine(fontFormatNumber));
		} else if (line.matchString("*chooser_init_string*", true)) {
			uint fontFormatNumber;

			sscanf(line.c_str(), "%*[^(](%u)", &fontFormatNumber);

			_string_chooser_init.readAllStyle(_engine->getStringManager()->getTextLine(fontFormatNumber));
		} else if (line.matchString("*next_tabstop*", true)) {
			sscanf(line.c_str(), "%*[^(](%u)", &_nextTabstop);
		} else if (line.matchString("*cursor_dimensions*", true)) {
			// Ignore, use the dimensions in the animation file
		} else if (line.matchString("*cursor_animation_frames*", true)) {
			// Ignore, use the frame count in the animation file
		} else if (line.matchString("*cursor_animation*", true)) {
			char fileName[25];

			sscanf(line.c_str(), "%*[^(](%25s %*u)", fileName);

			_animation = new MetaAnimation(fileName, _engine);
			_frame = -1;
			_frameDelay = 0;
		} else if (line.matchString("*focus*", true)) {
			_focused = true;
			_engine->getScriptManager()->setFocusControlKey(_key);
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}
}

bool InputControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_textRectangle.contains(backgroundImageSpacePos)) {
		if (!_readOnly) {
			// Save
			_engine->getScriptManager()->focusControl(_key);
		} else {
			// Restore
			if (_currentInputText.size())
				_enterPressed = true;
		}
	}
	return false;
}

bool InputControl::onMouseMove(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_textRectangle.contains(backgroundImageSpacePos)) {
		if (!_readOnly) {
			// Save
			_engine->getCursorManager()->changeCursor(CursorIndex_Active);
			return true;
		} else {
			// Restore
			if (_currentInputText.size()) {
				_engine->getCursorManager()->changeCursor(CursorIndex_Active);
				_engine->getScriptManager()->focusControl(_key);
				return true;
			}
		}
	}
	return false;
}

bool InputControl::onKeyDown(Common::KeyState keyState) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (!_focused) {
		return false;
	}

	if (keyState.keycode == Common::KEYCODE_BACKSPACE) {
		if (!_readOnly) {
			_currentInputText.deleteLastChar();
			_textChanged = true;
		}
	} else if (keyState.keycode == Common::KEYCODE_RETURN) {
		_enterPressed = true;
	} else if (keyState.keycode == Common::KEYCODE_TAB) {
		unfocus();
		// Focus the next input control
		_engine->getScriptManager()->focusControl(_nextTabstop);
		// Don't process this event for other controls
		return true;
	} else {
		if (!_readOnly) {
			// Otherwise, append the new character to the end of the current text
			uint16 asciiValue = keyState.ascii;
			// We only care about text values
			if (asciiValue >= 32 && asciiValue <= 126) {
				_currentInputText += (char)asciiValue;
				_textChanged = true;
			}
		}
	}
	return false;
}

bool InputControl::process(uint32 deltaTimeInMillis) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	// First see if we need to render the text
	if (_textChanged) {
		// Blit the text using the RenderManager

		Graphics::Surface txt;
		txt.create(_textRectangle.width(), _textRectangle.height(), _engine->_pixelFormat);

		if (!_readOnly || !_focused)
			_txtWidth = _engine->getTextRenderer()->drawTxt(_currentInputText, _string_init, txt);
		else
			_txtWidth = _engine->getTextRenderer()->drawTxt(_currentInputText, _string_chooser_init, txt);

		_engine->getRenderManager()->blitSurfaceToBkg(txt, _textRectangle.left, _textRectangle.top);

		txt.free();
	}

	if (_animation && !_readOnly && _focused) {
		bool need_draw = true;// = _textChanged;
		_frameDelay -= deltaTimeInMillis;
		if (_frameDelay <= 0) {
			_frame = (_frame + 1) % _animation->frameCount();
			_frameDelay = _animation->frameTime();
			need_draw = true;
		}

		if (need_draw) {
			const Graphics::Surface *srf = _animation->getFrameData(_frame);
			uint32 xx = _textRectangle.left + _txtWidth;
			if (xx >= _textRectangle.left + (_textRectangle.width() - _animation->width()))
				xx = _textRectangle.left + _textRectangle.width() - _animation->width();
			_engine->getRenderManager()->blitSurfaceToBkg(*srf, xx, _textRectangle.top);
		}
	}

	_textChanged = false;
	return false;
}

bool InputControl::enterPress() {
	if (_enterPressed) {
		_enterPressed = false;
		return true;
	}
	return false;
}

void InputControl::setText(const Common::String &_str) {
	_currentInputText = _str;
	_textChanged = true;
}

const Common::String InputControl::getText() {
	return _currentInputText;
}

void InputControl::setReadOnly(bool readonly) {
	_readOnly = readonly;
}

} // End of namespace ZVision
