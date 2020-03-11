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

#include "common/scummsys.h"

#include "zvision/scripting/controls/input_control.h"
#include "zvision/graphics/cursors/cursor_manager.h"

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/text/string_manager.h"
#include "zvision/graphics/render_manager.h"

#include "backends/keymapper/keymap.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/rect.h"
#include "video/video_decoder.h"

namespace ZVision {

InputControl::InputControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
	: Control(engine, key, CONTROL_INPUT),
	  _background(0),
	  _nextTabstop(0),
	  _focused(false),
	  _textChanged(false),
	  _enterPressed(false),
	  _readOnly(false),
	  _txtWidth(0),
	  _animation(NULL) {
	// Loop until we find the closing brace
	Common::String line = stream.readLine();
	_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
	Common::String param;
	Common::String values;
	getParams(line, param, values);

	while (!stream.eos() && !line.contains('}')) {
		if (param.matchString("rectangle", true)) {
			int x1;
			int y1;
			int x2;
			int y2;

			sscanf(values.c_str(), "%d %d %d %d", &x1, &y1, &x2, &y2);

			_textRectangle = Common::Rect(x1, y1, x2, y2);
		} else if (param.matchString("aux_hotspot", true)) {
			int x1;
			int y1;
			int x2;
			int y2;

			sscanf(values.c_str(), "%d %d %d %d", &x1, &y1, &x2, &y2);

			_headerRectangle = Common::Rect(x1, y1, x2, y2);
		} else if (param.matchString("string_init", true)) {
			uint fontFormatNumber;

			sscanf(values.c_str(), "%u", &fontFormatNumber);

			_stringInit.readAllStyles(_engine->getStringManager()->getTextLine(fontFormatNumber));
		} else if (param.matchString("chooser_init_string", true)) {
			uint fontFormatNumber;

			sscanf(values.c_str(), "%u", &fontFormatNumber);

			_stringChooserInit.readAllStyles(_engine->getStringManager()->getTextLine(fontFormatNumber));
		} else if (param.matchString("next_tabstop", true)) {
			sscanf(values.c_str(), "%u", &_nextTabstop);
		} else if (param.matchString("cursor_dimensions", true)) {
			// Ignore, use the dimensions in the animation file
		} else if (param.matchString("cursor_animation_frames", true)) {
			// Ignore, use the frame count in the animation file
		} else if (param.matchString("cursor_animation", true)) {
			char fileName[25];

			sscanf(values.c_str(), "%24s %*u", fileName);

			_animation = _engine->loadAnimation(fileName);
			_animation->start();
		} else if (param.matchString("focus", true)) {
			_focused = true;
			_engine->getScriptManager()->setFocusControlKey(_key);
		} else if (param.matchString("venus_id", true)) {
			_venusId = atoi(values.c_str());
		}

		line = stream.readLine();
		_engine->getScriptManager()->trimCommentsAndWhiteSpace(&line);
		getParams(line, param, values);
	}

	_maxTxtWidth = _textRectangle.width();
	if (_animation)
		_maxTxtWidth -= _animation->getWidth();
}

InputControl::~InputControl() {
	_background->free();
	delete _background;
	unfocus();
}

void InputControl::focus() {
	if (!_readOnly) {
		_engine->getGameKeymap()->setEnabled(false);
	}
	_focused = true;
	_textChanged = true;
}

void InputControl::unfocus() {
	if (!_readOnly) {
		_engine->getGameKeymap()->setEnabled(true);
	}
	_focused = false;
	_textChanged = true;
}

bool InputControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	if (_engine->getScriptManager()->getStateFlag(_key) & Puzzle::DISABLED)
		return false;

	if (_textRectangle.contains(backgroundImageSpacePos)) {
		if (!_readOnly) {
			// Save
			_engine->getScriptManager()->focusControl(_key);
			setVenus();
		} else {
			// Restore
			if (_currentInputText.size()) {
				setVenus();
				_enterPressed = true;
			}
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

	if (!_background) {
		_background = _engine->getRenderManager()->getBkgRect(_textRectangle);
	}

	// First see if we need to render the text
	if (_textChanged) {
		// Blit the text using the RenderManager

		Graphics::Surface txt;
		txt.copyFrom(*_background);

		int32 oldTxtWidth = _txtWidth;

		if (!_readOnly || !_focused)
			_txtWidth = _engine->getTextRenderer()->drawText(_currentInputText, _stringInit, txt);
		else
			_txtWidth = _engine->getTextRenderer()->drawText(_currentInputText, _stringChooserInit, txt);

		if (_readOnly || _txtWidth <= _maxTxtWidth)
			_engine->getRenderManager()->blitSurfaceToBkg(txt, _textRectangle.left, _textRectangle.top);
		else {
			// Assume the last character caused the overflow.
			_currentInputText.deleteLastChar();
			_txtWidth = oldTxtWidth;
		}

		txt.free();
	}

	if (_animation && !_readOnly && _focused) {
		if (_animation->endOfVideo())
			_animation->rewind();

		if (_animation->needsUpdate()) {
			const Graphics::Surface *srf = _animation->decodeNextFrame();
			int16 xx = _textRectangle.left + _txtWidth;
			if (xx >= _textRectangle.left + (_textRectangle.width() - (int16)_animation->getWidth()))
				xx = _textRectangle.left + _textRectangle.width() - (int16)_animation->getWidth();
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
