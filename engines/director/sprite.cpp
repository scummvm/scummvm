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
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"

namespace Director {

Sprite::Sprite(Frame *frame) {
	_frame = frame;
	_score = _frame->getScore();
	_movie = _score->getMovie();

	_scriptId = CastMemberID(0, 0);
	_colorcode = 0;
	_blendAmount = 0;
	_unk3 = 0;

	_enabled = false;
	_castId = CastMemberID(0, 0);
	_pattern = 0;

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
	_backColor = g_director->_wm->_colorWhite;
	_foreColor = g_director->_wm->_colorBlack;

	_blend = 0;

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

bool Sprite::respondsToMouse() {
	if (_moveable)
		return true;

	ScriptContext *spriteScript = _movie->getScriptContext(kScoreScript, _scriptId);
	if (spriteScript && (spriteScript->_eventHandlers.contains(kEventGeneric)
					  || spriteScript->_eventHandlers.contains(kEventMouseDown)
					  || spriteScript->_eventHandlers.contains(kEventMouseUp)))
		return true;

	ScriptContext *castScript = _movie->getScriptContext(kCastScript, _castId);
	if (castScript && (castScript->_eventHandlers.contains(kEventMouseDown)
					|| castScript->_eventHandlers.contains(kEventMouseUp)))
		return true;

	return false;
}

bool Sprite::isActive() {
	return _movie->getScriptContext(kScoreScript, _scriptId) != nullptr
			|| _movie->getScriptContext(kCastScript, _castId) != nullptr;
}

bool Sprite::shouldHilite() {
	if (!isActive())
		return false;

	if (_moveable)
		return false;

	if (_puppet)
		return false;

	if (_cast) {
		// Restrict to bitmap cast members.
		// Buttons also hilite on click, but they have their own check.
		if (_cast->_type != kCastBitmap)
			return false;

		if (g_director->getVersion() >= 300) {
			// The Auto Hilite flag was introduced in D3.

			CastMemberInfo *castInfo = _cast->getInfo();
			if (castInfo)
				return castInfo->autoHilite;

			// If there's no cast info, fall back to the old matte check.
			// In D4 or above, there should always be a cast info,
			// but in D3, it is not present unless you set a cast member's
			// name, script, etc.
		}
	} else {
		// QuickDraw shapes may also hilite on click.
		if (!isQDShape())
			return false;
	}

	return _ink == kInkTypeMatte;
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

void Sprite::setCast(CastMemberID memberID) {
	CastMember *member = _movie->getCastMember(memberID);
	_castId = memberID;

	if (memberID.member == 0)
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

			_cast->_type = kCastButton;
			((TextCastMember *)_cast)->_buttonType = (ButtonType)(_spriteType - 8);
		}

		// TODO: Respect sprite width/height settings. Need to determine how to read
		// them properly.
		Common::Rect dims = _cast->getInitialRect();
		// strange logic here, need to be fixed
		if (_cast->_type == kCastBitmap) {
			if (!(_inkData & 0x80)) {
				_width = dims.width();
				_height = dims.height();
			}
		} else if (_cast->_type != kCastShape && _cast->_type != kCastText) {
			_width = dims.width();
			_height = dims.height();
		}
	} else {
		warning("Sprite::setCast(): %s has null member", memberID.asString().c_str());
	}
}

} // End of namespace Director
