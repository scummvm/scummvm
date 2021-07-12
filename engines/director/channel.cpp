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
#include "director/window.h"

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macbutton.h"

namespace Director {

Channel::Channel(Sprite *sp, int priority) {
	if (!sp)
		_sprite = nullptr;
	else
		_sprite = new Sprite(*sp);

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
	delete _widget;
	delete _mask;
	delete _sprite;
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

	Common::Rect bbox(getBbox());

	if (needsMatte || forceMatte) {
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast->_type == kCastBitmap) {
			return ((BitmapCastMember *)_sprite->_cast)->getMatte(bbox);
		} else {
			return nullptr;
		}
	} else if (_sprite->_ink == kInkTypeMask) {
		CastMemberID maskID(_sprite->_castId.member + 1, _sprite->_castId.castLib);
		CastMember *member = g_director->getCurrentMovie()->getCastMember(maskID);

		if (member && member->_initialRect == _sprite->_cast->_initialRect) {
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

// TODO: eliminate this function when we got the correct method to deal with sprite size
// since we didn't handle sprites very well for text cast members. thus we don't replace our text castmembers when only size changes
// for explicitly changing, we have isModified to check
bool hasTextCastMember(Sprite *sprite) {
	if (sprite && sprite->_cast)
		return sprite->_cast->_type == kCastText || sprite->_cast->_type == kCastButton;
	return false;
}

bool Channel::isDirty(Sprite *nextSprite) {
	// When a sprite is puppeted setTheSprite ensures that the dirty flag here is
	// set. Otherwise, we need to rerender when the position, bounding box, or
	// cast of the sprite changes.
	if (!nextSprite)
		return false;

	bool isDirtyFlag = _dirty ||
		_delta != Common::Point(0, 0) ||
		(_sprite->_cast && _sprite->_cast->isModified());

	if (!_sprite->_puppet) {
		// When puppet is set, the overall dirty flag should be set when sprite is
		// modified.
		isDirtyFlag |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink;
		if (!_sprite->_moveable)
			isDirtyFlag |= _currentPoint != nextSprite->_startPoint;
		if (!_sprite->_stretch && !hasTextCastMember(_sprite))
			isDirtyFlag |= _width != nextSprite->_width || _height != nextSprite->_height;
	}

	return isDirtyFlag;
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
			Graphics::Surface *matte = ((BitmapCastMember *)_sprite->_cast)->getMatte(bbox);
			return matte ? !(*(byte *)(matte->getBasePtr(pos.x - bbox.left, pos.y - bbox.top))) : true;
		}
	}

	return true;
}

bool Channel::isMatteIntersect(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (intersectRect.isEmpty())
		return false;
	Graphics::Surface *myMatte = nullptr;
	Graphics::Surface *yourMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);
	if (channel->_sprite->_cast && channel->_sprite->_cast->_type == kCastBitmap)
		yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte(yourBbox);

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

// this contains channel. i.e. myBox contain yourBox
bool Channel::isMatteWithin(Channel *channel) {
	Common::Rect myBbox = getBbox();
	Common::Rect yourBbox = channel->getBbox();
	Common::Rect intersectRect = myBbox.findIntersectingRect(yourBbox);

	if (!myBbox.contains(yourBbox))
		return false;
	Graphics::Surface *myMatte = nullptr;
	Graphics::Surface *yourMatte = nullptr;

	if (_sprite->_cast && _sprite->_cast->_type == kCastBitmap)
		myMatte = ((BitmapCastMember *)_sprite->_cast)->getMatte(myBbox);
	if (channel->_sprite->_cast && channel->_sprite->_cast->_type == kCastBitmap)
		yourMatte = ((BitmapCastMember *)channel->_sprite->_cast)->getMatte(yourBbox);

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

void Channel::setCast(CastMemberID memberID) {
	_sprite->setCast(memberID);
	_width = _sprite->_width;
	_height = _sprite->_height;
	replaceWidget();
}

void Channel::setClean(Sprite *nextSprite, int spriteId, bool partial) {
	if (!nextSprite)
		return;

	CastMemberID previousCastId(0, 0);
	bool replace = isDirty(nextSprite);

	// for dirty situation that we need to replace widget.
	// if cast are modified, then we need to replace it
	// if cast size are changed, and we may need to replace it, because we may having the scaled bitmap castmember
	// other situation, e.g. position changing, we will let channel to handle it. So we don't have to replace widget
	bool dimsChanged = !_sprite->_stretch && !hasTextCastMember(_sprite) && (_sprite->_width != nextSprite->_width || _sprite->_height != nextSprite->_height);

	if (nextSprite) {
		if (nextSprite->_cast && (_dirty || _sprite->_castId != nextSprite->_castId)) {
			if (nextSprite->_cast->_type == kCastDigitalVideo) {
				Common::String path = nextSprite->_cast->getCast()->getVideoPath(nextSprite->_castId.member);

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
			previousCastId = _sprite->_castId;
			replaceSprite(nextSprite);
		}

		_currentPoint += _delta;
		_delta = Common::Point(0, 0);
	}

	if (replace) {
		_sprite->updateCast();
		replaceWidget(previousCastId, dimsChanged);
	}

	updateTextCast();
	updateGlobalAttr();

	_dirty = false;
}

// this is used to for setting and updating text castmember
// e.g. set editable, update dims for auto expanding
void Channel::updateTextCast() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastText)
		return;
	setEditable(_sprite->_editable);

	if (_widget) {
		Graphics::MacText *textWidget = (Graphics::MacText *)_widget;
		// if we got auto expand text, then we update dims to sprite
		if (!textWidget->getFixDims() && (_sprite->_width != _widget->_dims.width() || _sprite->_height != _widget->_dims.height())) {
			_sprite->_width = _widget->_dims.width();
			_sprite->_height = _widget->_dims.height();
			_width = _sprite->_width;
			_height = _sprite->_height;
			g_director->getCurrentWindow()->addDirtyRect(_widget->_dims);
		}
	}
}

void Channel::setEditable(bool editable) {
	if (_sprite->_cast && _sprite->_cast->_type == kCastText) {
		// if the sprite is editable, then we refresh the selEnd and setStart
		if (_sprite->_cast->isEditable() != editable)
			_sprite->_cast->setEditable(editable);

		if (_widget) {
			((Graphics::MacText *)_widget)->setEditable(editable);
			// we only set the first editable text member in score active
			if (editable) {
				Graphics::MacWidget *activewidget = g_director->_wm->getActiveWidget();
				if (activewidget == nullptr || !activewidget->isEditable())
					g_director->_wm->setActiveWidget(_widget);
			}
		}
	}
}

// we may optimize this by only update those attributes when we are changing them
// but not to pass them to widgets everytime
void Channel::updateGlobalAttr() {
	if (!_sprite->_cast)
		return;
	// update text info, including selEnd and selStart
	if (_sprite->_cast->_type == kCastText && _sprite->_editable && _widget)
		((Graphics::MacText *)_widget)->setSelRange(g_director->getCurrentMovie()->_selStart, g_director->getCurrentMovie()->_selEnd);
	// update button info, including checkBoxType
	if (_sprite->_cast->_type == kCastButton && _widget)
		((Graphics::MacButton *)_widget)->setCheckBoxType(g_director->getCurrentMovie()->_checkBoxType);
}

void Channel::replaceSprite(Sprite *nextSprite) {
	if (!nextSprite)
		return;

	bool newSprite = (_sprite->_spriteType == kInactiveSprite && nextSprite->_spriteType != kInactiveSprite);
	bool widgetKeeped = _sprite->_cast && _widget;

	// update the _sprite we stored in channel, and point the originalSprite to the new one
	// release the widget, because we may having the new one
	if (_sprite->_cast && !canKeepWidget(_sprite, nextSprite)) {
		widgetKeeped = false;
		_sprite->_cast->releaseWidget();
		newSprite = true;
	}

	int width = _width;
	int height = _height;

	*_sprite = *nextSprite;

	// TODO: auto expand text size is meaning less for us, not all text
	// since we are using initialRect for the text cast member now, then the sprite size is meaning less for us.
	// thus, we keep the _sprite size here
	if (hasTextCastMember(_sprite) && widgetKeeped) {
		_sprite->_width = width;
		_sprite->_height = height;
	}

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
		_width = MAX<int>(w, 0);
	}
}

void Channel::setHeight(int h) {
	if (_sprite->_puppet && _sprite->_stretch) {
		_height = MAX<int>(h, 0);
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

// here is the place for deciding whether the widget can be keep or not
// here's the definition, we first need to have widgets to keep, and the cast is not modified(modified means we need to re-create the widget)
// and the castId should be same while castId should not be zero
bool Channel::canKeepWidget(CastMemberID castId) {
	if (_widget && _sprite && _sprite->_cast && !_sprite->_cast->isModified() && castId.member && castId == _sprite->_castId) {
		return true;
	}
	return false;
}

bool Channel::canKeepWidget(Sprite *currentSprite, Sprite *nextSprite) {
	if (_widget && currentSprite && currentSprite->_cast && nextSprite && nextSprite->_cast && !currentSprite->_cast->isModified() && currentSprite->_castId == nextSprite->_castId && currentSprite->_castId.member) {
		return true;
	}
	return false;
}

// currently, when we are setting hilite, we delete the widget and the re-create it
// so we may optimize this if this operation takes much time
void Channel::replaceWidget(CastMemberID previousCastId, bool force) {
	// if the castmember is the same, and we are not modifying anything which cannot be handle by channel. Then we don't replace the widget
	if (!force && canKeepWidget(previousCastId)) {
		debug(5, "Channel::replaceWidget(): skip deleting %s", _sprite->_castId.asString().c_str());
		return;
	}

	if (_widget) {
		delete _widget;
		_widget = nullptr;
	}

	if (_sprite && _sprite->_cast) {
		Common::Rect bbox(getBbox());
		_sprite->_cast->setModified(false);

		_widget = _sprite->_cast->createWidget(bbox, this);
		if (_widget) {
			_widget->_priority = _priority;
			_widget->draw();

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
	case kCastBitmap: {
		BitmapCastMember *bc = (BitmapCastMember *)(_sprite->_cast);

		Common::Point point(0, 0);
		// stretch the offset
		if (!_sprite->_stretch && (_width < bc->_initialRect.width() || _height < bc->_initialRect.height())) {
			point.x = (bc->_initialRect.left - bc->_regX) * _width / bc->_initialRect.width();
			point.y = (bc->_initialRect.top - bc->_regY) * _height / bc->_initialRect.height();
		} else {
			point.x = bc->_initialRect.left - bc->_regX;
			point.y = bc->_initialRect.top - bc->_regY;
		}
		if (subtract)
			pos -= point;
		else
			pos += point;
	} break;
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

		// offset for the boundary
		constraintBbox.right++;
		constraintBbox.bottom++;

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
