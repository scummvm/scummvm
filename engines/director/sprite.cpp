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

	_matte = nullptr;
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
	delete _matte;
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

void Sprite::createQDMatte() {
	Graphics::ManagedSurface tmp;
	tmp.create(_width, _height, g_director->_pixelformat);
	tmp.clear(g_director->_wm->_colorWhite);

	Common::Rect srcRect(_width, _height);

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	Graphics::MacPlotData plotFill(&tmp, nullptr, &g_director->getPatterns(), getPattern(), 0, 0, 1, g_director->_wm->_colorBlack);

	// it's the same for filled and outlined qd shape when we are using floodfill, so we use filled rect directly since it won't be affected by line size.
	switch (_spriteType) {
	case kOutlinedRectangleSprite:
	case kRectangleSprite:
		Graphics::drawFilledRect(fillAreaRect, g_director->_wm->_colorBlack, g_director->_wm->getDrawPixel(), &plotFill);
		break;
	case kOutlinedRoundedRectangleSprite:
	case kRoundedRectangleSprite:
		Graphics::drawRoundRect(fillAreaRect, 12, g_director->_wm->_colorBlack, true, g_director->_wm->getDrawPixel(), &plotFill);
		break;
	case kOutlinedOvalSprite:
	case kOvalSprite:
		Graphics::drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, g_director->_wm->_colorBlack, true, g_director->_wm->getDrawPixel(), &plotFill);
		break;
	case kLineBottomTopSprite:
	case kLineTopBottomSprite:
		warning("Sprite::createQDMatte doesn't support creating matte for type %d", _spriteType);
		break;
	default:
		warning("Sprite::createQDMatte Expected shape type but got type %d", _spriteType);
	}

	Graphics::Surface surface;
	surface.create(_width, _height, g_director->_pixelformat);
	surface.copyFrom(tmp);

	_matte = new Graphics::FloodFill(&surface, g_director->_wm->_colorWhite, 0, true);

	for (int yy = 0; yy < surface.h; yy++) {
		_matte->addSeed(0, yy);
		_matte->addSeed(surface.w - 1, yy);
	}

	for (int xx = 0; xx < surface.w; xx++) {
		_matte->addSeed(xx, 0);
		_matte->addSeed(xx, surface.h - 1);
	}

	_matte->fillMask();
	tmp.free();
	surface.free();
}

Graphics::Surface *Sprite::getQDMatte() {
	if (!isQDShape() || _ink != kInkTypeMatte)
		return nullptr;
	if (!_matte)
		createQDMatte();
	return _matte ? _matte->getMask() : nullptr;
}

void Sprite::updateEditable() {
	if (!_cast)
		return;

	if (!_puppet)
		_editable = _editable || _cast->isEditable();
}

bool Sprite::respondsToMouse() {
	if (_moveable)
		return true;

	if (_cast && _cast->_type == kCastButton)
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
	if (_cast && _cast->_type == kCastButton)
		return true;

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

bool Sprite::checkSpriteType() {
	// check whether the sprite type match the cast type
	// if it doesn't match, then we treat it as transparent
	// this happens in warlock-mac data/stambul/c up
	if (_spriteType == kBitmapSprite && _cast->_type != kCastBitmap) {
		if (debugChannelSet(2, kDebugImages))
			warning("Sprite::checkSpriteType: Didn't render sprite due to the sprite type mismatch with cast type");
		return false;
	}
	return true;
}

void Sprite::setCast(CastMemberID memberID) {
	/**
	 * There are two things we need to take into account here:
	 *   1. The cast member's type
	 *   2. The sprite's type
	 * If the two types do not align, the sprite should not render.
	 * 
	 * Before D4, you needed to manually set a sprite's type along
	 * with its castNum.
	 * 
	 * Starting in D4, setting a sprite's castNum also set its type
	 * to an appropriate default.
	 */

	_castId = memberID;
	_cast = _movie->getCastMember(_castId);
	if (g_director->getVersion() >= 400)
		_spriteType = kCastMemberSprite;

	if (_cast) {
		if (g_director->getVersion() >= 400) {
			// Set the sprite type to be more specific ONLY for bitmap or text.
			// Others just use the generic kCastMemberSprite in D4.
			switch (_cast->_type) {
			case kCastBitmap:
				_spriteType = kBitmapSprite;
				break;
			case kCastText:
				_spriteType = kTextSprite;
				break;
			default:
				break;
			}
		}

		// TODO: Respect sprite width/height settings. Need to determine how to read
		// them properly.
		Common::Rect dims = _cast->getInitialRect();
		// strange logic here, need to be fixed
		if (_cast->_type == kCastBitmap) {
			// for the stretched sprites, we need the original size to get the correct bbox offset.
			// there are two stretch situation here.
			// 1. stretch happened when creating the widget, there is no lingo participated. we will use the original sprite size to create widget. check copyStretchImg
			// 2. stretch set by lingo. this time we need to store the original dims because we will create the original sprite and stretch it when bliting. check inkBlitStretchSurface
			if (!(_inkData & 0x80) || _stretch) {
				_width = dims.width();
				_height = dims.height();
			}
		} else if (_cast->_type != kCastShape && _cast->_type != kCastText) {
			_width = dims.width();
			_height = dims.height();
		}

	} else {
		if (_castId.member != 0 && debugChannelSet(kDebugImages, 2))
			warning("Sprite::setCast(): %s is null", memberID.asString().c_str());
	}
}

} // End of namespace Director
