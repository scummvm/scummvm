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

#include "director/director.h"
#include "director/cast.h"
#include "director/sprite.h"

namespace Director {

Sprite::Sprite() {
	_scriptId = 0;
	_scriptCastIndex = 0;
	_colorcode = 0;
	_blendAmount = 0;
	_unk3 = 0;

	_enabled = false;
	_castId = 0;
	_castIndex = 0;
	_spriteType = kInactiveSprite;
	_castType = kCastTypeNull;
	_inkData = 0;
	_ink = kInkTypeCopy;
	_trails = 0;

	_cast = nullptr;

	_thickness = 0;
	_dirty = false;
	_width = 0;
	_height = 0;
	_constraint = 0;
	_moveable = false;
	_editable = false;
	_puppet = false;
	_backColor = 255;
	_foreColor = 0;

	_blend = 0;
	_visible = false;
	_movieRate = 0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;
	_volume = 0;
	_stretch = 0;
}

Sprite::~Sprite() {
}

uint16 Sprite::getPattern() {
	switch (_spriteType) {
	case kRectangleSprite:
	case kRoundedRectangleSprite:
	case kOvalSprite:
	case kLineTopBottomSprite:
	case kLineBottomTopSprite:
	case kOutlinedRectangleSprite:
	case kOutlinedRoundedRectangleSprite:
	case kOutlinedOvalSprite:
		return _castId;

	case kCastMemberSprite:
		switch (_cast->_type) {
		case kCastShape:
			return ((ShapeCast *)_cast)->_pattern;
			break;
		default:
			warning("Sprite::getPattern(): Unhandled cast type: %d", _cast->_type);
			break;
		}
		// fallthrough
	default:
		return 0;
	}
}

void Sprite::setPattern(uint16 pattern) {
	switch (_spriteType) {
	case kRectangleSprite:
	case kRoundedRectangleSprite:
	case kOvalSprite:
	case kLineTopBottomSprite:
	case kLineBottomTopSprite:
	case kOutlinedRectangleSprite:
	case kOutlinedRoundedRectangleSprite:
	case kOutlinedOvalSprite:
		_castId = pattern;
		break;

	case kCastMemberSprite:
		// TODO
		warning("Sprite::setPattern(): kCastMemberSprite");
		return;

	default:
		return;
	}
}

void Sprite::setCast(uint16 castId) {
	Cast *member = g_director->getCastMember(castId);
	_castType = kCastTypeNull;
	_castId = castId;

	if (castId == 0)
		return;

	if (member)
		_cast = member;
	else
		warning("Sprite::setCast: Cast id %d has null member", castId);

	if (g_director->getVersion() < 4) {
		switch (_spriteType) {
		case kBitmapSprite:
			_castType = kCastBitmap;
			break;
		case kRectangleSprite:
		case kRoundedRectangleSprite:
		case kOvalSprite:
		case kLineTopBottomSprite:
		case kLineBottomTopSprite:
		case kOutlinedRectangleSprite:
		case kOutlinedRoundedRectangleSprite:
		case kOutlinedOvalSprite:
		case kCastMemberSprite:
			if (_cast != nullptr) {
				switch (_cast->_type) {
				case kCastButton:
					_castType = kCastButton;
					break;
				default:
					_castType = kCastShape;
					break;
				}
			} else {
				_castType = kCastShape;
			}
			break;
		case kTextSprite:
			_castType = kCastText;
			break;
		case kButtonSprite:
		case kCheckboxSprite:
		case kRadioButtonSprite:
			_castType = kCastButton;
			break;
		default:
			warning("Sprite::setCast(): Unhandled sprite type %d", _spriteType);
			break;
		}
	} else {
		if (!member) {
			debugC(1, kDebugImages, "Sprite::setCast(): Cast id %d not found", _castId);
		} else {
			_castType = member->_type;
		}
	}

	_dirty = true;
}

Common::Rect Sprite::getBbox() {
	Common::Rect result;
	if (_castId == 0) {
		return result;
	}

	switch (_castType) {
	case kCastShape:
		result = Common::Rect(_currentPoint.x,
								_currentPoint.y,
								_currentPoint.x + _width,
								_currentPoint.y + _height);
		break;
	case kCastRTE:
	case kCastText: {
		if (!_cast)
			return result;

		TextCast *textCast = (TextCast*)_cast;
		int x = _currentPoint.x; // +rectLeft;
		int y = _currentPoint.y; // +rectTop;
		int height = textCast->_initialRect.height(); //_sprites[spriteId]->_height;
		int width;
		Common::Rect *textRect = NULL;

		if (g_director->getVersion() >= 4) {
			// where does textRect come from?
			if (textRect == NULL) {
				width = textCast->_initialRect.right;
			} else {
				width = textRect->width();
			}
		} else {
			width = textCast->_initialRect.width(); //_sprites[spriteId]->_width;
		}

		result = Common::Rect(x, y, x + width, y + height);
		break;
	}
	case kCastButton: {
		// This may not be a button cast. It could be a textcast with the
		// channel forcing it to be a checkbox or radio button!
		if (!_cast)
			return result;

		ButtonCast *button = (ButtonCast *)_cast;

		// Sometimes, at least in the D3 Workshop Examples, these buttons are
		// just TextCast. If they are, then we just want to use the spriteType
		// as the button type. If they are full-bown Cast members, then use the
		// actual cast member type.
		int buttonType = _spriteType;
		if (buttonType == kCastMemberSprite) {
			switch (button->_buttonType) {
			case kTypeCheckBox:
				buttonType = kCheckboxSprite;
				break;
			case kTypeButton:
				buttonType = kButtonSprite;
				break;
			case kTypeRadio:
				buttonType = kRadioButtonSprite;
				break;
			}
		}

		uint32 rectLeft = button->_initialRect.left;
		uint32 rectTop = button->_initialRect.top;

		int x = _currentPoint.x;
		int y = _currentPoint.y;

		if (g_director->getVersion() > 3) {
			x += rectLeft;
			y += rectTop;
		}

		int height = button->_initialRect.height();
		int width = button->_initialRect.width() + 3;

		switch (buttonType) {
		case kCheckboxSprite:
			// Magic numbers: checkbox square need to move left about 5px from
			// text and 12px side size (D4)
			result = Common::Rect(x, y + 2, x + 12, y + 14);
			break;
		case kButtonSprite:
			result = Common::Rect(x, y, x + width, y + height + 3);
			break;
		case kRadioButtonSprite:
			result = Common::Rect(x, y + 2, x + 12, y + 14);
			break;
		default:
			warning("Score::getBbox(): Unknown buttonType");
		}
		break;
	}
	case kCastBitmap: {
		if (!_cast)
			return result;

		BitmapCast *bc = (BitmapCast *)_cast;

		int32 regX = bc->_regX;
		int32 regY = bc->_regY;
		int32 rectLeft = bc->_initialRect.left;
		int32 rectTop = bc->_initialRect.top;

		int x = _currentPoint.x - regX + rectLeft;
		int y = _currentPoint.y - regY + rectTop;
		int height = _height;
		int width = g_director->getVersion() > 4 ? bc->_initialRect.width() : _width;

		// If one of the dimensions is invalid, invalidate whole thing
		if (width == 0 || height == 0)
			width = height = 0;

		result = Common::Rect(x, y, x + width, y + height);
		break;
	}
	default:
		warning("Sprite::getBbox(): Unhandled cast type: %d : castId: %d", _castType, _castId);
	}
	return result;
}


} // End of namespace Director
