/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "director/director.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/cursor.h"
#include "director/cast.h"
#include "director/channel.h"
#include "director/sprite.h"
#include "director/types.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/filmloop.h"

#include "graphics/macgui/mactext.h"
#include "graphics/macgui/macbutton.h"

namespace Director {

Channel::Channel(Score *sc, Sprite *sp, int priority) {
	_score = sc;
	if (!sp)
		_sprite = nullptr;
	else
		_sprite = new Sprite(*sp);

	_widget = nullptr;
	_currentPoint = _sprite ? _sprite->_startPoint : Common::Point(0, 0);
	_constraint = 0;
	_mask = nullptr;

	_priority = priority;
	_width = _sprite ? _sprite->_width : 0;
	_height = _sprite ? _sprite->_height : 0;

	_movieRate = 0.0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;

	_filmLoopFrame = 0;

	_visible = true;
	_dirty = true;

	if (_sprite)
		_sprite->updateEditable();
}

Channel::Channel(const Channel &channel) {
	*this = channel;
}

Channel& Channel::operator=(const Channel &channel) {
	_score = channel._score;
	_sprite = channel._sprite ? new Sprite(*channel._sprite) : nullptr;

	_widget = nullptr;
	_currentPoint = channel._currentPoint;
	_constraint = channel._constraint;
	_mask = nullptr;

	_priority = channel._priority;
	_width = channel._width;
	_height = channel._height;

	_movieRate = channel._movieRate;
	_movieTime = channel._movieTime;
	_startTime = channel._startTime;
	_stopTime = channel._stopTime;

	_filmLoopFrame = channel._filmLoopFrame;

	_visible = channel._visible;
	_dirty = channel._dirty;

	return *this;
}


Channel::~Channel() {
	if (_widget)
		delete _widget;
	if (_mask)
		delete _mask;
	if (_sprite)
		delete _sprite;
}

DirectorPlotData Channel::getPlotData() {
	DirectorPlotData pd(g_director, _sprite->_spriteType, _sprite->_ink, _sprite->_blendAmount, _sprite->getBackColor(), _sprite->getForeColor());
	pd.colorWhite = 0;
	pd.colorBlack = 255;
	pd.dst = nullptr;

	pd.srf = getSurface();
	if (_sprite->_spriteType == kBitmapSprite &&
		_sprite->_cast && _sprite->_cast->_type == kCastBitmap &&
		((BitmapCastMember *)_sprite->_cast)->_bitsPerPixel == 1) {
		// Add override flag for 1-bit images
		pd.oneBitImage = true;
	}
	if (!pd.srf && _sprite->_spriteType != kBitmapSprite) {
		// Shapes come colourized from macDrawPixel
		pd.ms = _sprite->getShape();
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
		_sprite->_ink == kInkTypeBlend ||
		_sprite->_ink == kInkTypeAddPin ||
		_sprite->_ink == kInkTypeAdd ||
		_sprite->_ink == kInkTypeSubPin ||
		_sprite->_ink == kInkTypeLight ||
		_sprite->_ink == kInkTypeSub ||
		_sprite->_ink == kInkTypeDark ||
		_sprite->_blendAmount > 0;

	Common::Rect bbox(getBbox());

	if (needsMatte || forceMatte) {
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast->_type == kCastBitmap) {
			BitmapCastMember *bitmap = ((BitmapCastMember *)_sprite->_cast);
			// 1-bit images only require a matte for the matte ink type
			if (bitmap->_bitsPerPixel == 1 && _sprite->_ink != kInkTypeMatte) {
				return nullptr;
			}
			return bitmap->getMatte(bbox);
		} else {
			return nullptr;
		}
	} else if (_sprite->_ink == kInkTypeMask) {
		CastMemberID maskID(_sprite->_castId.member + 1, _sprite->_castId.castLib);
		CastMember *member = g_director->getCurrentMovie()->getCastMember(maskID);

		if (member && member->_initialRect == _sprite->_cast->_initialRect) {
			Graphics::MacWidget *widget = member->createWidget(bbox, this, _sprite->_spriteType);
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
		(_sprite->_cast && _sprite->_cast->isModified());

	if (_sprite && !_sprite->_puppet) {
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
	if (!_visible)
		return false;

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
	if (_sprite && (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo))
		return false;

	return true;
}

void Channel::updateVideoTime() {
	if (isActiveVideo())
		_movieTime = ((DigitalVideoCastMember *)_sprite->_cast)->getMovieCurrentTime();
}

bool Channel::isVideoDirectToStage() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastDigitalVideo)
		return false;

	return ((DigitalVideoCastMember *)_sprite->_cast)->_directToStage;
}

Common::Rect Channel::getBbox(bool unstretched) {
	Common::Rect result(unstretched ? _sprite->_width : _width,
						unstretched ? _sprite->_height : _height);
	if (_sprite->_cast) {
		result = _sprite->_cast->getBbox(_width, _height);
	}
	result.translate(_currentPoint.x, _currentPoint.y);
	return result;
}

void Channel::setCast(CastMemberID memberID) {
	// release previous widget
	if (_sprite->_cast)
		_sprite->_cast->releaseWidget();

	_sprite->setCast(memberID);
	_width = _sprite->_width;
	_height = _sprite->_height;
	replaceWidget();

	if (!_sprite->_puppet && g_director->getVersion() >= 600) {
		// Based on Director in a Nutshell, page 15
		_sprite->_autoPuppet = true;
	}
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

	// if spriteType is changing, then we may need to re-create the widget since spriteType will guide when we creating widget
	bool spriteTypeChanged = _sprite->_spriteType != nextSprite->_spriteType;

	if (nextSprite) {
		if (nextSprite->_cast && (_dirty || _sprite->_castId != nextSprite->_castId)) {
			if (nextSprite->_cast->_type == kCastDigitalVideo) {
				Common::String path = nextSprite->_cast->getCast()->getVideoPath(nextSprite->_castId.member);

				if (!path.empty()) {
					((DigitalVideoCastMember *)nextSprite->_cast)->loadVideo(path);
					_movieTime = 0;
					((DigitalVideoCastMember *)nextSprite->_cast)->startVideo(this);
				}
			} else if (nextSprite->_cast->_type == kCastFilmLoop) {
				// brand new film loop, reset the frame counter
				_filmLoopFrame = 0;
			}
		}

		// if the next sprite in the channel shares the cast member
		if (nextSprite->_cast && _sprite->_castId == nextSprite->_castId) {
			if (nextSprite->_cast->_type == kCastFilmLoop) {
				// increment the film loop counter
				_filmLoopFrame += 1;
				_filmLoopFrame %= ((FilmLoopCastMember *)nextSprite->_cast)->_frames.size();
			}
		}

		// for the non-puppet QDShape, since we won't use isDirty to check whether the QDShape is changed.
		// so we may always keep the sprite info because we need it to draw QDShape.
		if (_sprite->_puppet || (!nextSprite->isQDShape() && partial)) {
			// Updating scripts, etc. does not require a full re-render
			_sprite->_scriptId = nextSprite->_scriptId;
		} else {
			previousCastId = _sprite->_castId;
			replaceSprite(nextSprite);
		}
	}

	// FIXME: organize the logic here.
	// for the dirty puppet sprites, we will always replaceWidget since previousCastId is 0, but we shouldn't replace the widget of there are only position changing
	// e.g. we won't want a puppet editable text sprite changing because that will cause the loss of text.
	if (replace) {
		_sprite->updateEditable();
		replaceWidget(previousCastId, dimsChanged || spriteTypeChanged);
	}

	updateTextCast();
	updateGlobalAttr();

	// reset the stop time when we are not playing video
	if (_stopTime && (!_sprite->_cast || (_sprite->_cast && _sprite->_cast->_type != kCastDigitalVideo)))
		_stopTime = 0;

	_dirty = false;
}

// this is used to for setting and updating text castmember
// e.g. set editable, update dims for auto expanding
void Channel::updateTextCast() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastText)
		return;

	_sprite->updateEditable();
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
	if ((_sprite->_cast->_type == kCastButton || isButtonSprite(_sprite->_spriteType)) && _widget) {
		((Graphics::MacButton *)_widget)->setCheckBoxType(g_director->getCurrentMovie()->_checkBoxType);
		((Graphics::MacButton *)_widget)->setCheckBoxAccess(g_director->getCurrentMovie()->_checkBoxAccess);
	}
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
	if (_sprite->_castId != nextSprite->_castId && _sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		((DigitalVideoCastMember *)_sprite->_cast)->stopVideo();
		((DigitalVideoCastMember *)_sprite->_cast)->rewindVideo();
	}

	int width = _width;
	int height = _height;
	bool immediate = _sprite->_immediate;

	*_sprite = *nextSprite;

	// Persist the immediate flag
	_sprite->_immediate = immediate;

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
	if (!(_sprite->_cast && _sprite->_cast->_type == kCastShape) && !_sprite->_stretch)
		return;
	_width = MAX<int>(w, 0);

	if (!_sprite->_puppet && g_director->getVersion() >= 600) {
		// Based on Director in a Nutshell, page 15
		_sprite->_autoPuppet = true;
	}
}

void Channel::setHeight(int h) {
	if (!(_sprite->_cast && _sprite->_cast->_type == kCastShape) && !_sprite->_stretch)
		return;
	_height = MAX<int>(h, 0);

	if (!_sprite->_puppet && g_director->getVersion() >= 600) {
		// Based on Director in a Nutshell, page 15
		_sprite->_autoPuppet = true;
	}
}

void Channel::setBbox(int l, int t, int r, int b) {
	if (!(_sprite->_cast && _sprite->_cast->_type == kCastShape) && !_sprite->_stretch)
		return;
	_width = r - l;
	_height = b - t;

	Common::Rect source(_width, _height);
	if (_sprite->_cast) {
		source = _sprite->_cast->getBbox(_width, _height);
	}
	_currentPoint.x = (int16)(l - source.left);
	_currentPoint.y = (int16)(t - source.top);

	if (_width <= 0 || _height <= 0)
		_width = _height = 0;

	if (!_sprite->_puppet && g_director->getVersion() >= 600) {
		// Based on Director in a Nutshell, page 15
		_sprite->_autoPuppet = true;
	}
}

void Channel::setPosition(int x, int y, bool force) {
	Common::Point newPos(x, y);
	if (_constraint > 0 && _score && _constraint <= _score->_channels.size()) {
		Common::Rect constraintBbox = _score->_channels[_constraint]->getBbox();
		newPos.x = MIN(constraintBbox.right, MAX(constraintBbox.left, newPos.x));
		newPos.y = MIN(constraintBbox.bottom, MAX(constraintBbox.top, newPos.y));
	}
	_currentPoint = newPos;

	if (!_sprite->_puppet && g_director->getVersion() >= 600) {
		// Based on Director in a Nutshell, page 15
		_sprite->_autoPuppet = true;
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
		// use sprite type to guide us how to draw the cast
		// if the type don't match, then we will set it as transparent. i.e. don't create widget
		if (!_sprite->checkSpriteType())
			return;
		// always use the unstretched dims.
		// because only the stretched sprite will have different channel size and sprite size
		// we need the original image to scale the sprite.
		// for the scaled bitmap castmember, it has scaled dims on sprite size, so we don't have to worry about it.
		Common::Rect bbox(getBbox(true));
		_sprite->_cast->setModified(false);

		_widget = _sprite->_cast->createWidget(bbox, this, _sprite->_spriteType);
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

bool Channel::isTrail() {
	return _sprite->_trails;
}

int Channel::getMouseChar(int x, int y) {
	if (_sprite->_spriteType != kTextSprite)
		return -1;

	if (!_widget) {
		warning("Channel::getMouseChar getting mouse char on a non-existing widget");
		return -1;
	}

	return ((Graphics::MacText *)_widget)->getMouseChar(x, y);
}

int Channel::getMouseWord(int x, int y) {
	if (_sprite->_spriteType != kTextSprite)
		return -1;

	if (!_widget) {
		warning("Channel::getMouseWord getting mouse word on a non-existing widget");
		return -1;
	}

	return ((Graphics::MacText *)_widget)->getMouseWord(x, y);
}

int Channel::getMouseItem(int x, int y) {
	if (_sprite->_spriteType != kTextSprite)
		return -1;

	if (!_widget) {
		warning("Channel::getMouseItem getting mouse item on a non-existing widget");
		return -1;
	}

	return ((Graphics::MacText *)_widget)->getMouseItem(x, y);
}

int Channel::getMouseLine(int x, int y) {
	if (_sprite->_spriteType != kTextSprite)
		return -1;

	if (!_widget) {
		warning("Channel::getMouseLine getting mouse line on a non-existing widget");
		return -1;
	}

	return ((Graphics::MacText *)_widget)->getMouseLine(x, y);
}

bool Channel::hasSubChannels() {
	if ((_sprite->_cast) && (_sprite->_cast->_type == kCastFilmLoop)) {
		return true;
	}
	return false;
}

Common::Array<Channel> *Channel::getSubChannels() {
	if ((!_sprite->_cast) || (_sprite->_cast->_type != kCastFilmLoop)) {
		warning("Channel doesn't have any sub-channels");
		return nullptr;
	}
	Common::Rect bbox = getBbox();
	return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannels(bbox, this);
}

} // End of namespace Director
