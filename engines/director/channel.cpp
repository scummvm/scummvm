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
#include "director/movie.h"
#include "director/score.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/castmember.h"

#include "graphics/macgui/mactext.h"

namespace Director {

Channel::Channel(Sprite *sp) {
	_sprite = sp;
	_currentPoint = sp->_startPoint;
	_delta = Common::Point(0, 0);
	_constraint = 0;

	_visible = true;
	_dirty = true;

	_sprite->updateCast();
}

DirectorPlotData Channel::getPlotData() {
	DirectorPlotData pd(g_director->_wm, _sprite->_spriteType, _sprite->_ink, getBackColor(), getForeColor());
	pd.colorWhite = pd._wm->_colorWhite;
	pd.colorBlack = pd._wm->_colorBlack;

	pd.srf = getSurface();
	if (!pd.srf) {
		// Shapes come colourized from macDrawPixel
		pd.ms = getShape();
		pd.applyColor = false;
	} else {
		pd.setApplyColor();
	}

	return pd;
}

Graphics::ManagedSurface *Channel::getSurface() {
	if (_sprite->_cast && _sprite->_cast->_widget) {
		return  _sprite->_cast->_widget->getSurface();
	} else {
		return nullptr;
	}
}

const Graphics::Surface *Channel::getMask(bool forceMatte) {
	if (!_sprite->_cast || _sprite->_spriteType == kTextSprite)
		return nullptr;

	bool needsMatte = _sprite->_ink == kInkTypeMatte ||
		_sprite->_ink == kInkTypeNotCopy ||
		_sprite->_ink == kInkTypeNotTrans ||
		_sprite->_ink == kInkTypeNotReverse ||
		_sprite->_ink == kInkTypeNotGhost;

	if (needsMatte || forceMatte) {
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast->_type == kCastBitmap) {
			return ((BitmapCastMember *)_sprite->_cast)->getMatte();
		} else {
			return nullptr;
		}
	} else if (_sprite->_ink == kInkTypeMask) {
		CastMember *member = g_director->getCurrentMovie()->getCastMember(_sprite->_castId + 1);

		if (member && member->_initialRect == _sprite->_cast->_initialRect) {
			return &member->_widget->getSurface()->rawSurface();
		} else {
			warning("Channel::getMask(): Requested cast mask, but no matching mask was found");
			return nullptr;
		}
	}

	return nullptr;
}

bool Channel::isDirty(Sprite *nextSprite) {
	// When a sprite is puppeted setTheSprite ensures that the dirty flag here is
	// set. Otherwise, we need to rerender when the position, bounding box, or
	// cast of the sprite changes.
	bool isDirty = _dirty ||
		_delta != Common::Point(0, 0) ||
		(_sprite->_cast && _sprite->_cast->isModified());

	if (nextSprite) {
		isDirty |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink ||
			_sprite->getDims() != nextSprite->getDims() ||
			(_currentPoint != nextSprite->_startPoint &&
			 !_sprite->_puppet && !_sprite->_moveable);
	}

	return isDirty;
}

Common::Rect Channel::getBbox() {
	Common::Rect bbox = _sprite->getDims();
	bbox.moveTo(getPosition());

	return bbox;
}

void Channel::setClean(Sprite *nextSprite, int spriteId) {
	if (!nextSprite)
		return;

	bool newSprite = (_sprite->_spriteType == kInactiveSprite && nextSprite->_spriteType != kInactiveSprite);
	_dirty = false;

	if (!_sprite->_puppet) {
		_sprite = nextSprite;
		_sprite->updateCast();

		// Sprites marked moveable are constrained to the same bounding box until
		// the moveable is disabled
		if (!_sprite->_moveable || newSprite)
			_currentPoint = _sprite->_startPoint;
	}

	_currentPoint += _delta;
	_delta = Common::Point(0, 0);

	if (_sprite->_cast && _sprite->_cast->_widget) {
		Common::Point p(getPosition());
		_sprite->_cast->_modified = false;
		_sprite->_cast->_widget->_dims.moveTo(p.x, p.y);

		_sprite->_cast->_widget->_priority = spriteId;
		_sprite->_cast->_widget->draw();
		_sprite->_cast->_widget->_contentIsDirty = false;
	}
}

void Channel::addRegistrationOffset(Common::Point &pos) {
	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap) {
		BitmapCastMember *bc = (BitmapCastMember *)(_sprite->_cast);

		pos += Common::Point(bc->_initialRect.left - bc->_regX,
												 bc->_initialRect.top - bc->_regY);
	}
}

void Channel::addDelta(Common::Point pos) {
	// TODO: Channel should have a pointer to its score
	if (_sprite->_moveable &&
			_constraint > 0 &&
			_constraint < g_director->getCurrentMovie()->getScore()->_channels.size()) {
		Common::Rect constraintBbox = g_director->getCurrentMovie()->getScore()->_channels[_constraint]->getBbox();

		Common::Rect currentBbox = getBbox();
		currentBbox.translate(_delta.x + pos.x, _delta.y + pos.y);

		Common::Point regPoint;
		addRegistrationOffset(regPoint);

		constraintBbox.top += regPoint.y;
		constraintBbox.bottom -= regPoint.y;

		constraintBbox.left += regPoint.x;
		constraintBbox.right -= regPoint.x;

		if (!constraintBbox.contains(currentBbox)) {
			if (currentBbox.top < constraintBbox.top) {
				pos.y += constraintBbox.top - currentBbox.top;
			} else if (currentBbox.bottom > constraintBbox.bottom) {
				pos.y += constraintBbox.bottom - currentBbox.bottom;
			}

			if (currentBbox.left < constraintBbox.left) {
				pos.x += constraintBbox.left - currentBbox.left;
			} else if (currentBbox.right > constraintBbox.right) {
				pos.x += constraintBbox.right - currentBbox.right;
			}
		}
	}

	_delta += pos;
}

Common::Point Channel::getPosition() {
	Common::Point res = _currentPoint;
	addRegistrationOffset(res);
	return res;
}

MacShape *Channel::getShape() {
	if (!_sprite->isQDShape() && (_sprite->_cast && _sprite->_cast->_type != kCastShape))
		return nullptr;

	MacShape *shape = new MacShape();

	shape->ink = _sprite->_ink;
	shape->spriteType = _sprite->_spriteType;
	shape->foreColor = _sprite->_foreColor;
	shape->backColor = _sprite->_backColor;
	shape->lineSize = _sprite->_thickness & 0x3;
	shape->pattern = _sprite->getPattern();

	if (g_director->getVersion() >= 3 && shape->spriteType == kCastMemberSprite) {
		if (!_sprite->_cast) {
			warning("Channel::getShape(): kCastMemberSprite has no cast defined");
			delete shape;
			return nullptr;
		}

		ShapeCastMember *sc = (ShapeCastMember *)_sprite->_cast;
		switch (sc->_shapeType) {
		case kShapeRectangle:
			shape->spriteType = sc->_fillType ? kRectangleSprite : kOutlinedRectangleSprite;
			break;
		case kShapeRoundRect:
			shape->spriteType = sc->_fillType ? kRoundedRectangleSprite : kOutlinedRoundedRectangleSprite;
			break;
		case kShapeOval:
			shape->spriteType = sc->_fillType ? kOvalSprite : kOutlinedOvalSprite;
			break;
		case kShapeLine:
			shape->spriteType = sc->_lineDirection == 6 ? kLineBottomTopSprite : kLineTopBottomSprite;
			break;
		default:
			break;
		}

		if (g_director->getVersion() > 3) {
			shape->foreColor = sc->getForeColor();
			shape->backColor = sc->getBackColor();
			shape->lineSize = sc->_lineThickness;
			shape->ink = sc->_ink;
		}
	}

	// for outlined shapes, line thickness of 1 means invisible.
	shape->lineSize -= 1;

	return shape;
}

uint Channel::getBackColor() {
	if (!_sprite->_cast)
		return _sprite->_backColor;

	switch (_sprite->_cast->_type) {
	case kCastText:
	case kCastButton:
	case kCastShape: {
		return _sprite->_cast->getBackColor();
	}
	default:
		return _sprite->_backColor;
	}
}

uint Channel::getForeColor() {
	if (!_sprite->_cast)
		return _sprite->_foreColor;

	switch (_sprite->_cast->_type) {
	case kCastText:
	case kCastButton:
	case kCastShape: {
		return _sprite->_cast->getForeColor();
	}
	default:
		return _sprite->_foreColor;
	}
}

} // End of namespace Director
