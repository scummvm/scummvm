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
#include "director/cursor.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/castmember.h"

#include "graphics/macgui/mactext.h"

namespace Director {

Channel::Channel(Sprite *sp, int priority) {
	_sprite = sp;
	_widget = nullptr;
	_currentPoint = sp->_startPoint;
	_delta = Common::Point(0, 0);
	_constraint = 0;
	_mask = nullptr;

	_priority = priority;
	_width = _sprite->_width;
	_height = _sprite->_height;

	_movieRate = 0.0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;

	_visible = true;
	_dirty = true;

	_sprite->updateCast();
}

Channel::~Channel() {
	if (_widget)
		delete _widget;
	if (_mask)
		delete _mask;
}

DirectorPlotData Channel::getPlotData() {
	DirectorPlotData pd(g_director->_wm, _sprite->_spriteType, _sprite->_ink, _sprite->_blend, getBackColor(), getForeColor());
	pd.colorWhite = pd._wm->_colorWhite;
	pd.colorBlack = pd._wm->_colorBlack;
	pd.dst = nullptr;

	pd.srf = getSurface();
	if (!pd.srf && _sprite->_spriteType != kBitmapSprite) {
		// Shapes come colourized from macDrawPixel
		pd.ms = getShape();
		pd.applyColor = false;
	} else {
		pd.setApplyColor();
	}

	return pd;
}

Graphics::ManagedSurface *Channel::getSurface() {
	if (_widget) {
		return _widget->getSurface();
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
		_sprite->_ink == kInkTypeNotGhost ||
		_sprite->_blend > 0;

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
			Common::Rect bbox(getBbox());
			Graphics::MacWidget *widget = member->createWidget(bbox, this);
			if (_mask)
				delete _mask;
			_mask = new Graphics::ManagedSurface();
			_mask->copyFrom(*widget->getSurface());
			delete widget;
			return &_mask->rawSurface();
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
	if (!nextSprite)
		return false;

	bool isDirty = _dirty ||
		_delta != Common::Point(0, 0) ||
		(_sprite->_cast && _sprite->_cast->isModified());

	if (!_sprite->_puppet) {
		// When puppet is set, the overall dirty flag should be set when sprite is
		// modified.
		isDirty |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink;
		if (!_sprite->_moveable)
			isDirty |= _currentPoint != nextSprite->_startPoint;
		if (!_sprite->_stretch)
			isDirty |= _width != nextSprite->_width || _height != nextSprite->_height;
	}

	return isDirty;
}

bool Channel::isStretched() {
	return _sprite->_puppet && _sprite->_stretch &&
		(_sprite->_width != _width || _sprite->_height != _height);
}

bool Channel::isEmpty() {
	return (_sprite->_spriteType == kInactiveSprite);
}

bool Channel::isActiveText() {
	if (_sprite->_spriteType != kTextSprite)
		return false;

	if (_widget && _widget->hasAllFocus())
		return true;

	return false;
}

bool Channel::isMouseIn(const Common::Point &pos) {
	Common::Rect bbox = getBbox();

	if (!bbox.contains(pos))
		return false;

	if (_sprite->_ink == kInkTypeMatte) {
		if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap) {
			Graphics::Surface *matte = ((BitmapCastMember *)_sprite->_cast)->getMatte();
			return matte ? !(*(byte *)(matte->getBasePtr(pos.x - bbox.left, pos.y - bbox.top))) : true;
		}
	}

	return true;
}

bool Channel::isMatteIntersect(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (intersectRect.isEmpty() || !_sprite->_cast || _sprite->_cast->_type != kCastBitmap ||
			!channel->_sprite->_cast || channel->_sprite->_cast->_type != kCastBitmap)
		return false;

	Graphics::Surface *myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte();
	Graphics::Surface *yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte();

	if (myMatte && yourMatte) {
		for (int i = intersectRect.top; i < intersectRect.bottom; i++) {
			const byte *my = (const byte *)myMatte->getBasePtr(intersectRect.left - myBbox.left, i - myBbox.top);
			const byte *your = (const byte *)yourMatte->getBasePtr(intersectRect.left - yourBbox.left, i - yourBbox.top);

			for (int j = intersectRect.left; j < intersectRect.right; j++, my++, your++)
				if (!*my && !*your)
					return true;
		}
	}

	return false;
}

bool Channel::isMatteWithin(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (!myBbox.contains(yourBbox) || !_sprite->_cast || _sprite->_cast->_type != kCastBitmap ||
			!channel->_sprite->_cast || channel->_sprite->_cast->_type != kCastBitmap)
		return false;

	Graphics::Surface *myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte();
	Graphics::Surface *yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte();

	if (myMatte && yourMatte) {
		for (int i = intersectRect.top; i < intersectRect.bottom; i++) {
			const byte *my = (const byte *)myMatte->getBasePtr(intersectRect.left - myBbox.left, i - myBbox.top);
			const byte *your = (const byte *)yourMatte->getBasePtr(intersectRect.left - yourBbox.left, i - yourBbox.top);

			for (int j = intersectRect.left; j < intersectRect.right; j++, my++, your++)
				if (*my && !*your)
					return false;
		}

		return true;
	}

	return false;
}

bool Channel::isActiveVideo() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo)
		return false;

	return true;
}

bool Channel::isVideoDirectToStage() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo)
		return false;

	return ((DigitalVideoCastMember *)_sprite->_cast)->_directToStage;
}

Common::Rect Channel::getBbox(bool unstretched) {
	Common::Rect result(unstretched ? _sprite->_width : _width,
											unstretched ? _sprite->_height : _height);
	result.moveTo(getPosition());

	return result;
}

void Channel::setCast(uint16 castId) {
	_sprite->setCast(castId);
	_width = _sprite->_width;
	_height = _sprite->_height;
	replaceWidget();
}

void Channel::setClean(Sprite *nextSprite, int spriteId, bool partial) {
	if (!nextSprite)
		return;

	bool replace = isDirty(nextSprite);

	if (nextSprite) {
		if (nextSprite->_cast && (_dirty || _sprite->_castId != nextSprite->_castId)) {
			if (nextSprite->_cast->_type == kCastDigitalVideo) {
				Common::String path = nextSprite->_cast->getCast()->getVideoPath(nextSprite->_castId);

				if (!path.empty()) {
					((DigitalVideoCastMember *)nextSprite->_cast)->loadVideo(pathMakeRelative(path));
					((DigitalVideoCastMember *)nextSprite->_cast)->startVideo(this);
				}
			}
		}

		if (_sprite->_puppet || partial) {
			// Updating scripts, etc. does not require a full re-render
			_sprite->_scriptId = nextSprite->_scriptId;
		} else {
			replaceSprite(nextSprite);
		}

		_currentPoint += _delta;
		_delta = Common::Point(0, 0);
	}

	if (replace) {
		_sprite->updateCast();
		replaceWidget();
	}

	setEditable(_sprite->_editable);

	_dirty = false;
}

void Channel::setEditable(bool editable) {
	if (_sprite->_cast && _sprite->_cast->_type == kCastText) {
		_sprite->_cast->setEditable(editable);

		if (_widget) {
			_widget->_editable = editable;
			g_director->_wm->setActiveWidget(_widget);
		}
	}
}

void Channel::replaceSprite(Sprite *nextSprite) {
	if (!nextSprite)
		return;

	bool newSprite = (_sprite->_spriteType == kInactiveSprite && nextSprite->_spriteType != kInactiveSprite);
	_sprite = nextSprite;

	// Sprites marked moveable are constrained to the same bounding box until
	// the moveable is disabled
	if (!_sprite->_moveable || newSprite)
		_currentPoint = _sprite->_startPoint;

	if (!_sprite->_stretch) {
		_width = _sprite->_width;
		_height = _sprite->_height;
	}
}

void Channel::setWidth(int w) {
	if (_sprite->_puppet && _sprite->_stretch) {
		_width = w;
	}
}

void Channel::setHeight(int h) {
	if (_sprite->_puppet && _sprite->_stretch) {
		_height = h;
	}
}

void Channel::setBbox(int l, int t, int r, int b) {
	if (_sprite->_puppet && _sprite->_stretch) {
		_width = r - l;
		_height = b - t;

		_currentPoint.x = l;
		_currentPoint.y = t;

		addRegistrationOffset(_currentPoint, true);
	}
}

void Channel::replaceWidget() {
	if (_widget) {
		delete _widget;
		_widget = nullptr;
	}

	if (_sprite && _sprite->_cast) {
		Common::Rect bbox(getBbox());
		_sprite->_cast->_modified = false;

		_widget = _sprite->_cast->createWidget(bbox, this);
		if (_widget) {
			_widget->_priority = _priority;
			_widget->draw();

			// HACK: Account for the added dimensions for borders, etc.
			if (_sprite->_cast->_type == kCastText || _sprite->_cast->_type == kCastButton) {
				_sprite->_width = _widget->_dims.width();
				_sprite->_height = _widget->_dims.height();

				_width = _sprite->_width;
				_height = _sprite->_height;
			}
		}
	}
}

bool Channel::updateWidget() {
	if (_widget && _widget->needsRedraw()) {
		if (_sprite->_cast) {
			_sprite->_cast->updateFromWidget(_widget);
		}
		_widget->draw();
		return true;
	}

	return false;
}

void Channel::addRegistrationOffset(Common::Point &pos, bool subtract) {
	if (!_sprite->_cast)
		return;

	switch (_sprite->_cast->_type) {
	case kCastBitmap:
		{
			BitmapCastMember *bc = (BitmapCastMember *)(_sprite->_cast);

			if (subtract)
				pos -= Common::Point(bc->_initialRect.left - bc->_regX,
															bc->_initialRect.top - bc->_regY);
			else
				pos += Common::Point(bc->_initialRect.left - bc->_regX,
															bc->_initialRect.top - bc->_regY);
		}
		break;
	case kCastDigitalVideo:
		pos -= Common::Point(_sprite->_cast->_initialRect.width() >> 1, _sprite->_cast->_initialRect.height() >> 1);
		break;
	default:
		break;
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

	res.x += (_sprite->_width - _width) / 2;
	res.y += (_sprite->_height - _height) / 2;

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

	if (g_director->getVersion() >= 300 && shape->spriteType == kCastMemberSprite) {
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

		if (g_director->getVersion() >= 400) {
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

uint32 Channel::getBackColor() {
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

uint32 Channel::getForeColor() {
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
