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
#include "engines/stark/resources/sound.h"

#include "engines/stark/visual/image.h"
#include "engines/stark/visual/text.h"

namespace Stark {

const float Cursor::_fadeValueMax = 0.3f;

Cursor::Cursor(Gfx::Driver *gfx) :
		_gfx(gfx),
		_cursorImage(nullptr),
		_mouseText(nullptr),
		_currentCursorType(kImage),
		_itemActive(false),
		_fadeLevelIncreasing(true),
		_fadeLevel(0),
		_hintDisplayDelay(150) {
	setCursorType(kDefault);
	_actionHoverSound = StarkStaticProvider->getUISound(StaticProvider::kActionHover);
}

Cursor::~Cursor() {
	delete _mouseText;
}

void Cursor::setCursorType(CursorType type) {
	assert(type != kImage);
	if (type == _currentCursorType) {
		return;
	}
	_currentCursorType = type;
	_cursorImage = nullptr;
}

void Cursor::setCursorImage(VisualImageXMG *image) {
	_currentCursorType = kImage;
	_cursorImage = image;
}

void Cursor::setMousePosition(const Common::Point &pos) {
	_mousePos = pos;
	_hintDisplayDelay = 150;
}

void Cursor::setItemActive(bool itemActive) {
	if (_itemActive == itemActive) {
		return;
	}
	if (itemActive) {
		_actionHoverSound->play();
	} else {
		_actionHoverSound->stop();
	}
	_itemActive = itemActive;
}

void Cursor::onScreenChanged() {
	if (_mouseText) {
		_mouseText->resetTexture();
	}
}

void Cursor::updateFadeLevel() {
	if (_itemActive) {
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

void Cursor::updateHintDelay() {
	if (_hintDisplayDelay >= 0) {
		_hintDisplayDelay -= StarkGlobal->getMillisecondsPerGameloop();

		if (_hintDisplayDelay <= 0) {
			_hintDisplayDelay = -1;
		}
	}
}

void Cursor::render() {
	updateFadeLevel();
	updateHintDelay();

	if (!_gfx->isPosInScreenBounds(_mousePos)) {
		setCursorType(Cursor::kPassive);
	}

	if (_mouseText && _gfx->gameViewport().contains(_mousePos) && _hintDisplayDelay <= 0) {
		_gfx->setScreenViewport(false);

		// TODO: Should probably query the image for the width of the cursor
		// TODO: Add delay to the mouse hints like in the game
		const int16 cursorDistance = 32;
		Common::Rect mouseRect = _mouseText->getRect();
		Common::Point pos = _gfx->convertCoordinateCurrentToOriginal(_mousePos);
		pos.x = CLIP<int16>(pos.x, 48, Gfx::Driver::kOriginalWidth - 48);
		pos.y = CLIP<int16>(pos.y, Gfx::Driver::kTopBorderHeight, Gfx::Driver::kOriginalHeight - Gfx::Driver::kBottomBorderHeight - cursorDistance - mouseRect.height());
		pos.x -= mouseRect.width() / 2;
		pos.y += cursorDistance;

		_mouseText->render(pos);
	}

	if (_currentCursorType != kImage) {
		_cursorImage = StarkStaticProvider->getCursorImage(_currentCursorType);
	}

	if (_cursorImage) {
		_gfx->setScreenViewport(true); // Unscaled viewport so that cursor is drawn on native pixel space, thus no 'skipping', perform scaling below instead

		_cursorImage->setFadeLevel(_fadeLevel);
		_cursorImage->render(_mousePos, true, false); // Draws image (scaled)
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
			_mouseText->setColor(Color(0xFF, 0xFF, 0xFF));
			_mouseText->setBackgroundColor(Color(0x00, 0x00, 0x00, 0x80));
			_mouseText->setFont(FontProvider::kSmallFont);
			_mouseText->setTargetWidth(96);
		} else {
			_mouseText = nullptr;
		}
		_currentHint = hint;
		_hintDisplayDelay = 150;
	}
}

Common::Rect Cursor::getHotRectangle() const {
	if (!_cursorImage) {
		return Common::Rect();
	} else {
		Common::Point hotSpot = _cursorImage->getHotspot();

		Common::Rect hotRectangle;
		hotRectangle.setWidth(_cursorImage->getWidth());
		hotRectangle.setHeight(_cursorImage->getHeight());
		hotRectangle.translate(-hotSpot.x, -hotSpot.y);

		return hotRectangle;
	}
}

} // End of namespace Stark
