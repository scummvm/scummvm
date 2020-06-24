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
#include "director/lingo/lingo.h"
#include "director/sprite.h"

#include "director/score.h"

#include "graphics/macgui/macwidget.h"

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
	_moveable = false;
	_editable = false;
	_puppet = false;
	_immediate = false;
	_backColor = 255;
	_foreColor = 0;

	_blend = 0;
	_movieRate = 0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;
	_volume = 0;
	_stretch = 0;
}

Sprite::~Sprite() {
}

void Sprite::updateCast() {
	if (!_cast)
		return;

	if (!_cast->_widget) {
		if (_cast->_type == kCastText && (_spriteType == kButtonSprite || _spriteType == kCheckboxSprite || _spriteType == kRadioButtonSprite)) {
			// WORKAROUND: In D2/D3 there can be text casts that have button
			// information set in the sprite.
			warning("Sprite::updateCast: Working around D2/3 button glitch");
			_cast->_type = kCastButton;
			((TextCast *)_cast)->_buttonType = (ButtonType)(_spriteType - 8);
		}

		_cast->createWidget();
	}

	if (_cast->isEditable() != _editable && !_puppet)
		_cast->setEditable(_editable);
}

bool Sprite::isDirty() {
	return _castType != kCastTypeNull && (_dirty || (_cast && _cast->isModified()));
}

bool Sprite::isActive() {
	if (_moveable || _puppet || _scriptId)
		return true;
	
	if (g_lingo->getScriptContext(kArchMain, kCastScript, _castId)
			|| g_lingo->getScriptContext(kArchShared, kCastScript, _castId))
		return true;

	return false;
}

void Sprite::setClean() {
	_dirty = false;
	if (_cast)
		_cast->_modified = false;
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
	else {
		warning("Sprite::setCast: Cast id %d has null member", castId);
	}

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
			if (_cast) {
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

				g_director->getCurrentScore()->_loadedCast->setVal(_castId, new ShapeCast());
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

Common::Rect Sprite::getDims() {
	Common::Rect result;
	if (_castId == 0) {
		return result;
	}

	if (_castType == kCastShape) {
		// WORKAROUND: Shape widgets not fully implemented.
		result = Common::Rect(_width, _height);
	} else {
		if (_cast && _cast->_widget) {
			result = Common::Rect(_cast->_widget->_dims.width(), _cast->_widget->_dims.height());
		}
	}

	if (_puppet && _stretch) {
		// TODO: Properly align the bounding box

		result.setHeight(_height);
		result.setWidth(_width);
	}

	return result;
}


} // End of namespace Director
