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
	if (member) {
		_cast = member;
		_castId = castId;
		_castType = kCastTypeNull;

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
			_castType = member->_type;
		}
	}
}


} // End of namespace Director
