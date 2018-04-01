/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/ui/cursor.h"

#include "engines/stark/gfx/driver.h"
#include "engines/stark/gfx/texture.h"

#include "engines/stark/services/gameinterface.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/services.h"
#include "engines/stark/services/staticprovider.h"

#include "engines/stark/resources/item.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

namespace Stark {

const float Cursor::_fadeValueMax = 0.3f;

Cursor::Cursor(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursorImage(nullptr),
		_mouseText(nullptr),
		_currentCursorType(kNone),
		_fading(false),
		_fadeLevelIncreasing(true),
		_fadeLevel(0) {
	setCursorType(kDefault);
}

Cursor::~Cursor() {
	delete _mouseText;
}

void Cursor::setCursorType(CursorType type) {
	if (type == _currentCursorType) {
		return;
	}
	_currentCursorType = type;
	if (type == kNone) {
		_cursorImage = nullptr;
		return;
	}

	_cursorImage = StarkStaticProvider->getCursorImage(_currentCursorType);
}

void Cursor::setCursorImage(VisualImageXMG *image) {
	_currentCursorType = kNone;
	_cursorImage = image;
}


void Cursor::setMousePosition(const Common::Point &pos) {
	_mousePos = pos;
}

void Cursor::setFading(bool fading) {
	_fading = fading;
}

void Cursor::updateFadeLevel() {
	if (_fading) {
		if (_fadeLevelIncreasing) {
			_fadeLevel += 0.001f * StarkGlobal->getMillisecondsPerGameloop();
		} else {
			_fadeLevel -= 0.001f * StarkGlobal->getMillisecondsPerGameloop();
		}
		if (ABS(_fadeLevel) >= _fadeValueMax) {
			_fadeLevelIncreasing = !_fadeLevelIncreasing;
			_fadeLevel = CLIP(_fadeLevel, -_fadeValueMax, _fadeValueMax);
		}
	} else {
		_fadeLevel = 0;
	}
}

void Cursor::render() {
	updateFadeLevel();


	if (!_gfx->isPosInScreenBounds(_mousePos)) {
		setCursorType(Cursor::kPassive);
	}

	if (_mouseText) {
		_gfx->setScreenViewport(false);

		// TODO: Should probably query the image for the width of the cursor
		// TODO: Add delay to the mouse hints like in the game
		const int16 cursorDistance = _gfx->scaleHeightCurrentToOriginal(32);
		Common::Rect mouseRect = _mouseText->getRect();
		Common::Point pos = _gfx->convertCoordinateCurrentToOriginal(_mousePos);
		pos.x = CLIP<int16>(pos.x, 48, Gfx::Driver::kOriginalWidth - 48);
		pos.y = CLIP<int16>(pos.y, Gfx::Driver::kTopBorderHeight, Gfx::Driver::kOriginalHeight - Gfx::Driver::kBottomBorderHeight - cursorDistance - mouseRect.height());
		pos.x -= mouseRect.width() / 2;
		pos.y += cursorDistance;

		_mouseText->render(pos);
	}

	if (_cursorImage) {
		_gfx->setScreenViewport(true); // The cursor is drawn unscaled

		_cursorImage->setFadeLevel(_fadeLevel);
		_cursorImage->render(_mousePos, true);
	}
}

Common::Point Cursor::getMousePosition(bool unscaled) const {
	if (unscaled) {
		return _mousePos;
	} else {
		// Most of the engine expects 640x480 coordinates
		return _gfx->convertCoordinateCurrentToOriginal(_mousePos);
	}
}

void Cursor::setMouseHint(const Common::String &hint) {
	if (hint != _currentHint) {
		delete _mouseText;
		if (!hint.empty()) {
			_mouseText = new VisualText(_gfx);
			_mouseText->setText(hint);
			_mouseText->setColor(0xFFFFFFFF);
			_mouseText->setBackgroundColor(0x80000000);
			_mouseText->setFont(FontProvider::kSmallFont);
			_mouseText->setTargetWidth(96);
		} else {
			_mouseText = nullptr;
		}
		_currentHint = hint;
	}
}

} // End of namespace Stark
