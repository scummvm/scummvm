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

#include "zvision/zvision.h"
#include "zvision/scripting/script_manager.h"
#include "zvision/strings/string_manager.h"
#include "zvision/graphics/render_manager.h"
#include "zvision/utility/utility.h"

#include "common/str.h"
#include "common/stream.h"
#include "common/rect.h"


namespace ZVision {

InputControl::InputControl(ZVision *engine, uint32 key, Common::SeekableReadStream &stream)
		: Control(engine, key),
		  _nextTabstop(0),
		  _focused(false),
		  _textChanged(false),
		  _cursorOffset(0) {
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

			_textStyle = _engine->getStringManager()->getTextStyle(fontFormatNumber);
		} else if (line.matchString("*next_tabstop*", true)) {
			sscanf(line.c_str(), "%*[^(](%u)", &_nextTabstop);
		} else if (line.matchString("*cursor_animation*", true)) {
			char fileName[25];

			sscanf(line.c_str(), "%*[^(](%25s %*u)", fileName);

			_cursorAnimationFileName = Common::String(fileName);
		} else if (line.matchString("*cursor_dimensions*", true)) {
			// Ignore, use the dimensions in the animation file
		} else if (line.matchString("*cursor_animation_frames*", true)) {
			// Ignore, use the frame count in the animation file
		} else if (line.matchString("*focus*", true)) {
			_focused = true;
		}

		line = stream.readLine();
		trimCommentsAndWhiteSpace(&line);
	}
}

void InputControl::onMouseUp(const Common::Point &screenSpacePos, const Common::Point &backgroundImageSpacePos) {
	_engine->getScriptManager()->focusControl(_key);
}

void InputControl::onKeyDown(Common::KeyState keyState) {
	if (!_focused) {
		return;
	}

	if (keyState.keycode == Common::KEYCODE_BACKSPACE) {
		_currentInputText.deleteLastChar();
	} else if (keyState.keycode == Common::KEYCODE_TAB) {
		_focused = false;
		// Focus the next input control
		_engine->getScriptManager()->focusControl(_nextTabstop);
	} else {
		// Otherwise, append the new character to the end of the current text

		uint16 asciiValue = keyState.ascii;
		// We only care about text values
		if (asciiValue >= 32 && asciiValue <= 126) {
			_currentInputText += (char)asciiValue;
			_textChanged = true;
		}
	}
}

bool InputControl::process(uint32 deltaTimeInMillis) {
	if (!_focused) {
		return false;
	}

	// First see if we need to render the text
	if (_textChanged) {
		// Blit the text using the RenderManager
		Common::Rect destRect = _engine->getRenderManager()->renderTextToWorkingWindow(_key, _currentInputText, _textStyle.font, _textRectangle.left, _textRectangle.top, _textStyle.color, _textRectangle.width());

		_cursorOffset = destRect.left - _textRectangle.left;
	}

	// Render the next frame of the animation
	// TODO: Implement animation handling

	return false;
}

} // End of namespace ZVision
