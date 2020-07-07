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

#include "graphics/macgui/macwidget.h"

#include "director/director.h"
#include "director/castmember.h"
#include "director/movie.h"
#include "director/sprite.h"
#include "director/lingo/lingo.h"

namespace Director {

Sprite::Sprite() {
	_scriptId = 0;
	_scriptCastIndex = 0;
	_colorcode = 0;
	_blendAmount = 0;
	_unk3 = 0;

	_enabled = false;
	_castId = 0;
	_pattern = 0;

	_castIndex = 0;
	_spriteType = kInactiveSprite;
	_inkData = 0;
	_ink = kInkTypeCopy;
	_trails = 0;

	_cast = nullptr;

	_thickness = 0;
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

bool Sprite::isQDShape() {
	return _spriteType == kRectangleSprite ||
		_spriteType == kRoundedRectangleSprite ||
		_spriteType == kOvalSprite ||
		_spriteType == kLineTopBottomSprite ||
		_spriteType == kLineBottomTopSprite ||
		_spriteType == kOutlinedRectangleSprite ||
		_spriteType == kOutlinedRoundedRectangleSprite ||
		_spriteType == kOutlinedOvalSprite ||
		_spriteType == kThickLineSprite;
}

void Sprite::updateCast() {
	if (!_cast)
		return;

	if (_cast->isEditable() != _editable && !_puppet)
		_cast->setEditable(_editable);
}

bool Sprite::isFocusable() {
	if (_moveable || _puppet || _scriptId)
		return true;

	return false;
}

bool Sprite::shouldHilite() {
	if ((_cast && _cast->_autoHilite) || (isQDShape() && _ink == kInkTypeMatte))
		if (g_director->getVersion() < 4 && !_moveable)
			if (g_director->getCurrentMovie()->getScriptContext(kScoreScript, _scriptId) ||
					g_director->getCurrentMovie()->getScriptContext(kCastScript, _castId))
				return true;

	return false;
}

uint16 Sprite::getPattern() {
	if (!_cast) {
		if (isQDShape())
			return _pattern;
	} else if (_cast->_type == kCastShape) {
		return ((ShapeCastMember *)_cast)->_pattern;
	}

	return 0;
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
		_pattern = pattern;
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
	CastMember *member = g_director->getCurrentMovie()->getCastMember(castId);
	_castId = castId;

	if (castId == 0)
		return;

	if (member) {
		_cast = member;

		if (_cast->_type == kCastText &&
				(_spriteType == kButtonSprite ||
				 _spriteType == kCheckboxSprite ||
				 _spriteType == kRadioButtonSprite)) {
			// WORKAROUND: In D2/D3 there can be text casts that have button
			// information set in the sprite.
			warning("Sprite::setCast(): Working around D2/3 button glitch");

			delete _cast->_widget;
			_cast->_type = kCastButton;
			((TextCastMember *)_cast)->_buttonType = (ButtonType)(_spriteType - 8);
			((TextCastMember *)_cast)->createWidget();
		}
	} else {
		warning("Sprite::setCast(): CastMember id %d has null member", castId);
	}
}

Common::Rect Sprite::getDims() {
	Common::Rect result;

	if (!_cast || _cast->_type == kCastShape) {
		result = Common::Rect(_width, _height);
	} else if (_cast->_widget) {
		result = Common::Rect(_cast->_widget->_dims.width(), _cast->_widget->_dims.height());
	} else {
		warning("Sprite::getDims(): Unable to find sprite dimensions");
	}

	if (_puppet && _stretch) {
		// TODO: Properly align the bounding box

		result.setHeight(_height);
		result.setWidth(_width);
	}

	return result;
}


} // End of namespace Director
