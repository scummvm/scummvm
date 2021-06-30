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

void Sprite::createQDMatte() {
	Graphics::ManagedSurface tmp;
	tmp.create(_width, _height, g_director->_pixelformat);
	tmp.clear(g_director->_wm->_colorWhite);

	MacShape *ms = new MacShape();

	ms->ink = _ink;
	ms->spriteType = _spriteType;
	ms->foreColor = _foreColor;
	ms->backColor = _backColor;
	ms->lineSize = _thickness & 0x3;
	ms->pattern = getPattern();

	Common::Rect srcRect(_width, _height);

	Common::Rect fillAreaRect((int)srcRect.width(), (int)srcRect.height());
	Graphics::MacPlotData plotFill(&tmp, nullptr, &g_director->getPatterns(), ms->pattern, 0, 0, 1, ms->backColor);

	Common::Rect strokeRect(MAX((int)srcRect.width() - ms->lineSize, 0), MAX((int)srcRect.height() - ms->lineSize, 0));
	Graphics::MacPlotData plotStroke(&tmp, nullptr, &g_director->getPatterns(), 1, 0, 0, ms->lineSize, ms->backColor);

	switch (_spriteType) {
	case kRectangleSprite:
		Graphics::drawFilledRect(fillAreaRect, g_director->_wm->_colorBlack, g_director->_wm->getDrawPixel(), &plotFill);
		// fall through
	case kOutlinedRectangleSprite:
		Graphics::drawRect(strokeRect, g_director->_wm->_colorBlack, g_director->_wm->getDrawPixel(), &plotStroke);
		break;
	case kRoundedRectangleSprite:
		Graphics::drawRoundRect(fillAreaRect, 12, g_director->_wm->_colorBlack, true, g_director->_wm->getDrawPixel(), &plotFill);
		// fall through
	case kOutlinedRoundedRectangleSprite:
		Graphics::drawRoundRect(strokeRect, 12, g_director->_wm->_colorBlack, false, g_director->_wm->getDrawPixel(), &plotStroke);
		break;
	case kOvalSprite:
		Graphics::drawEllipse(fillAreaRect.left, fillAreaRect.top, fillAreaRect.right, fillAreaRect.bottom, g_director->_wm->_colorBlack, true, g_director->_wm->getDrawPixel(), &plotFill);
		// fall through
	case kOutlinedOvalSprite:
		Graphics::drawEllipse(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, g_director->_wm->_colorBlack, false, g_director->_wm->getDrawPixel(), &plotStroke);
		break;
	case kLineTopBottomSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.top, strokeRect.right, strokeRect.bottom, g_director->_wm->_colorBlack, g_director->_wm->getDrawPixel(), &plotStroke);
		break;
	case kLineBottomTopSprite:
		Graphics::drawLine(strokeRect.left, strokeRect.bottom, strokeRect.right, strokeRect.top, g_director->_wm->_colorBlack, g_director->_wm->getDrawPixel(), &plotStroke);
		break;
	default:
		warning("Sprite:createQDMatte Expected shape type but got type %d", _spriteType);
	}

	Graphics::Surface managedSurface;
	managedSurface.create(_width, _height, g_director->_pixelformat);
	managedSurface.copyFrom(tmp);

	_matte = new Graphics::FloodFill(&managedSurface, g_director->_wm->_colorWhite, 0, true);

	for (int yy = 0; yy < managedSurface.h; yy++) {
		_matte->addSeed(0, yy);
		_matte->addSeed(managedSurface.w - 1, yy);
	}

	for (int xx = 0; xx < managedSurface.w; xx++) {
		_matte->addSeed(xx, 0);
		_matte->addSeed(xx, managedSurface.h - 1);
	}

	_matte->fillMask();
	tmp.free();
	managedSurface.free();
}

Graphics::Surface *Sprite::getQDMatte() {
	if (!isQDShape() || _ink != kInkTypeMatte)
		return nullptr;
	if (!_matte)
		createQDMatte();
	return _matte ? _matte->getMask() : nullptr;
}

bool Sprite::shouldHilite() {
	if (!isActive())
		return false;

	if (_moveable)
		return false;

	if (_puppet)
		return false;

	if (_ink == kInkTypeMatte) {
		if (g_director->getVersion() < 300)
			return true;
		if (isQDShape())
			return true;
	}

	if (_cast) {
		// we have our own check for button, thus we don't need it here
		if (_cast->_type == kCastButton)
			return false;
		CastMemberInfo *castInfo = _cast->getInfo();
		if (castInfo)
			return castInfo->autoHilite;
	}

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
		if (_cast->_type != kCastShape && _cast->_type != kCastText) {
			Common::Rect dims = _cast->getInitialRect();
			_width = dims.width();
			_height = dims.height();
		}
	} else {
		warning("Sprite::setCast(): %s has null member", memberID.asString().c_str());
	}
}

} // End of namespace Director
