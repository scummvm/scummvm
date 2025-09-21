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
#include "director/cast.h"
#include "director/channel.h"
#include "director/picture.h"
#include "director/sprite.h"
#include "director/types.h"
#include "director/window.h"
#include "director/castmember/castmember.h"
#include "director/castmember/bitmap.h"
#include "director/castmember/digitalvideo.h"
#include "director/castmember/filmloop.h"
#include "director/castmember/movie.h"

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
	_constraint = 0;
	_mask = nullptr;

	_priority = priority;

	_movieRate = 0.0;
	_movieTime = 0;
	_startTime = 0;
	_stopTime = 0;

	_filmLoopFrame = 0;

	_visible = true;
	_dirty = true;

	if (sp) {
		_startFrame = sp->_spriteInfo.startFrame;
		_endFrame = sp->_spriteInfo.endFrame;
	} else {
		_startFrame = -1;
		_endFrame = -1;
	}
}

Channel::Channel(const Channel &channel) {
	*this = channel;
}

Channel& Channel::operator=(const Channel &channel) {
	_score = channel._score;
	_sprite = channel._sprite ? new Sprite(*channel._sprite) : nullptr;

	_widget = nullptr;
	_constraint = channel._constraint;
	_mask = nullptr;

	_priority = channel._priority;

	_movieRate = channel._movieRate;
	_movieTime = channel._movieTime;
	_startTime = channel._startTime;
	_stopTime = channel._stopTime;

	_filmLoopFrame = channel._filmLoopFrame;

	_visible = channel._visible;
	_dirty = channel._dirty;

	_startFrame = channel._startFrame;
	_endFrame = channel._endFrame;

	return *this;
}


Channel::~Channel() {
	if (_widget) {
		if (dynamic_cast<Graphics::MacWindow *>(_widget))
			g_director->_wm->removeWindow((Graphics::MacWindow *)_widget);
		else
			delete _widget;
	}

	if (_mask)
		delete _mask;
	if (_sprite)
		delete _sprite;
}

DirectorPlotData Channel::getPlotData() {
	int blend = (_sprite->_thickness & kTHasBlend) || _sprite->_ink == kInkTypeBlend ? _sprite->_blendAmount : 0;
	DirectorPlotData pd(g_director, _sprite->_spriteType, _sprite->_ink, blend, _sprite->getBackColor(), _sprite->getForeColor());
	pd.colorWhite = g_director->getColorWhite();
	pd.colorBlack = g_director->getColorBlack();
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
		(((_sprite->_thickness & kTHasBlend) || _sprite->_ink == kInkTypeBlend) && _sprite->_blendAmount > 0);

	Common::Rect bbox(getBbox());

	if (needsMatte || forceMatte) {
		// Mattes are only supported in bitmaps for now. Shapes don't need mattes,
		// as they already have all non-enclosed white pixels transparent.
		// Matte on text has a trivial enough effect to not worry about implementing.
		if (_sprite->_cast->_type == kCastBitmap) {
			BitmapCastMember *bitmap = ((BitmapCastMember *)_sprite->_cast);
			// 1-bit images only require a matte for the matte ink type
			if (bitmap->_bitsPerPixel == 1 && _sprite->_ink != kInkTypeMatte) {
				// 1-bit images will not blend with kInkTypeCopy, whereas 8-bit images will.
				if (_sprite->_ink == kInkTypeCopy) {
                    _sprite->_blendAmount = 0;
                }
				return nullptr;
			}
			return bitmap->getMatte(bbox);
		} else {
			return nullptr;
		}
	} else if (_sprite->_ink == kInkTypeMask) {
		CastMemberID maskID(_sprite->_castId.member + 1, _sprite->_castId.castLib);
		CastMember *member = g_director->getCurrentMovie()->getCastMember(maskID);

		if (member) {
			if (member->_type != kCastBitmap) {
				warning("Channel::getMask(): Requested cast mask %s, but type is %s, not bitmap", maskID.asString().c_str(), castType2str(member->_type));
				return nullptr;
			}
			BitmapCastMember *bitmap = (BitmapCastMember *)member;
			if (bitmap->_bitsPerPixel != 1) {
				warning("Channel::getMask(): Requested cast mask %s, but bitmap isn't 1bpp", maskID.asString().c_str());
				return nullptr;
			}

			if (_mask) {
				delete _mask;
				_mask = nullptr;
			}
			if (bitmap->_picture) {
				// reposition channel bounding box, so origin is at registration offset
				Common::Point originPos = getPosition();
				bbox.translate(-originPos.x, -originPos.y);
				// create new mask surface, with the exact dimensions of the channel.
				_mask = new Graphics::ManagedSurface(bbox.width(), bbox.height());
				// get the bounding box of the mask image (origin at registration offset)
				Common::Rect destRect = bitmap->getBbox();
				// get position of channel's registration offset (origin at top left)
				Common::Point channelRegOffset(-bbox.left, -bbox.top);
				// move destination rect to sit at the channel's registration offset
				destRect.translate(channelRegOffset.x, channelRegOffset.y);
				Common::Point destOrigin(destRect.left, destRect.top);
				// clip the destination rect so it is contained within the mask bounds
				destRect.clip(_mask->getBounds());
				// make a copy of the destination rect with the origin at the top left of the mask bitmap
				Common::Rect srcRect = destRect;
				srcRect.translate(-destOrigin.x, -destOrigin.y);
				debugC(8, kDebugImages, "Channel::getMask(): cast mask %s, orig %dx%d, dest %dx%d, crop %d,%d %dx%d",  maskID.asString().c_str(), bitmap->_picture->_surface.w, bitmap->_picture->_surface.h, bbox.width(), bbox.height(), destRect.left, destRect.top, destRect.width(), destRect.height());
				_mask->copyRectToSurface(bitmap->_picture->_surface, destRect.left, destRect.top, srcRect);
				return &_mask->rawSurface();
			} else {
				warning("Channel::getMask(): Requested cast mask %s, but no picture found", maskID.asString().c_str());
				return nullptr;
			}
		} else {
			warning("Channel::getMask(): Requested cast mask %s, but was not found", maskID.asString().c_str());
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

	if (_sprite && !_sprite->_puppet && !_sprite->_autoPuppet) {
		// When puppet is set, the overall dirty flag should be set when sprite is
		// modified.
		isDirtyFlag |= _sprite->_castId != nextSprite->_castId ||
			_sprite->_ink != nextSprite->_ink || _sprite->_backColor != nextSprite->_backColor ||
			_sprite->_foreColor != nextSprite->_foreColor ||
			_sprite->_blendAmount != nextSprite->_blendAmount || _sprite->_thickness != nextSprite->_thickness;
		if (!_sprite->_moveable)
			isDirtyFlag |= _sprite->getPosition() != nextSprite->getPosition();
		if (isStretched() && !hasTextCastMember(_sprite))
			isDirtyFlag |= _sprite->_width != nextSprite->_width || _sprite->_height != nextSprite->_height;
	}

	return isDirtyFlag;
}

Common::Rect Channel::getRollOverBbox() {
	// In D4 and below, the rollOver command will check against whatever the last
	// contents of the sprite were, regardless of whether the score has zeroed it out.
	if (g_director->getVersion() < 500 && _sprite->_castId.member == 0) {
		return _rollOverBbox;
	}
	return getBbox();
}

bool Channel::isStretched() {
	return _sprite->_stretch;
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

CollisionTest Channel::isMouseIn(const Common::Point &pos) {
	if (!_visible)
		return kCollisionNo;

	const Common::Rect bbox = getBbox();

	if (_sprite->_cast) {
		return _sprite->_cast->isWithin(bbox, pos, _sprite->_ink);
	} else if (!bbox.contains(pos)) {
		return kCollisionNo;
	}

	return kCollisionYes;
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
				if (*my && *your)
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
				if (!*my && *your)
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

void Channel::setCast(CastMemberID memberID) {
	// release previous widget
	if (_sprite->_cast)
		_sprite->_cast->releaseWidget();

	bool hasChanged = _sprite->_castId != memberID;
	// Replace the cast member in the sprite.
	// Only change the dimensions if the "stretch" flag is set,
	// indicating that the sprite has already been warped away from cast
	// dimensions. In puppet mode Lingo can first change the
	// dimensions of the sprite, -then- change the cast ID, and expect
	// those custom dimensions to stick around.
	_sprite->setCast(memberID, !_sprite->_stretch);

	// Duplicate of the special cases in setClean.
	// Maybe it makes sense to force setClean to use setCast instead?
	if (hasChanged && _sprite->_cast) {
		if (_sprite->_cast->_type == kCastDigitalVideo) {
			DigitalVideoCastMember *dv = (DigitalVideoCastMember *)_sprite->_cast;
			if (dv->loadVideoFromCast()) {
				_movieTime = 0;
				dv->setChannel(this);
				dv->startVideo();
			}
		} else if (_sprite->_cast->_type == kCastFilmLoop ||
					_sprite->_cast->_type == kCastMovie) {
			// brand new film loop, reset the frame counter.
			_filmLoopFrame = 1;
		}
	}

	replaceWidget();

	// Based on Director in a Nutshell, page 15
	_sprite->setAutoPuppet(kAPCast, true);
}

void Channel::setClean(Sprite *nextSprite, bool partial) {
	if (!nextSprite)
		return;

	CastMemberID previousCastId(0, 0);
	bool replace = isDirty(nextSprite);

	// for dirty situation that we need to replace widget.
	// if cast are modified, then we need to replace it
	// if cast size are changed, and we may need to replace it, because we may having the scaled bitmap castmember
	// other situation, e.g. position changing, we will let channel to handle it. So we don't have to replace widget
	bool dimsChanged = !hasTextCastMember(_sprite) && (_sprite->_width != nextSprite->_width || _sprite->_height != nextSprite->_height);

	// if spriteType is changing, then we may need to re-create the widget since spriteType will guide when we creating widget
	bool spriteTypeChanged = _sprite->_spriteType != nextSprite->_spriteType;

	if (nextSprite) {
		if (nextSprite->_cast && (_dirty || _sprite->_castId != nextSprite->_castId)) {
			if (_sprite->_castId != nextSprite->_castId && nextSprite->_cast->_type == kCastDigitalVideo) {
				if (((DigitalVideoCastMember *)nextSprite->_cast)->loadVideoFromCast()) {
					_movieTime = 0;
					((DigitalVideoCastMember *)nextSprite->_cast)->setChannel(this);
					((DigitalVideoCastMember *)nextSprite->_cast)->startVideo();
				}
			} else if (nextSprite->_cast->_type == kCastFilmLoop || nextSprite->_cast->_type == kCastMovie) {
				// brand new film loop, reset the frame counter.
				_filmLoopFrame = 1;
			}
		}

		// for the non-puppet QDShape, since we won't use isDirty to check whether the QDShape is changed.
		// so we may always keep the sprite info because we need it to draw QDShape.
		if (_sprite->_puppet || _sprite->_autoPuppet || (!nextSprite->isQDShape() && partial)) {
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
		replaceWidget(previousCastId, dimsChanged || spriteTypeChanged);
	}

	updateTextCast();
	updateGlobalAttr();

	// reset the stop time when we are not playing video
	if (_stopTime && (!_sprite->_cast || (_sprite->_cast && _sprite->_cast->_type != kCastDigitalVideo)))
		_stopTime = 0;

	_dirty = false;
}

void Channel::setStretch(bool enabled) {
	if (!enabled) {
		// when the stretch flag is manually disabled,
		// revert whatever dimensions the sprite has to
		// the default in the cast
		g_director->getCurrentWindow()->addDirtyRect(getBbox());
		_dirty = true;

		if (_sprite->_cast) {
			Common::Rect bbox = _sprite->_cast->getBbox();
			_sprite->setWidth(bbox.width());
			_sprite->setHeight(bbox.height());
		}
	}
	_sprite->_stretch = enabled;
}

// this is used to for setting and updating text castmember
// e.g. set editable, update dims for auto expanding
void Channel::updateTextCast() {
	if (!_sprite->_cast || _sprite->_cast->_type != kCastText)
		return;

	setEditable(_sprite->getEditable());

	if (_widget) {
		Graphics::MacText *textWidget = (Graphics::MacText *)_widget;
		// if we got auto expand text, then we update dims to sprite
		if (!textWidget->getFixDims() && (_sprite->_width != _widget->_dims.width() || _sprite->_height != _widget->_dims.height())) {
			_sprite->_width = _widget->_dims.width();
			_sprite->_height = _widget->_dims.height();
			g_director->getCurrentWindow()->addDirtyRect(_widget->_dims);
		}
	}
}

bool Channel::getEditable() {
	if (_sprite->_cast && _sprite->_cast->_type == kCastText) {
		if (_widget && (Graphics::MacText *)_widget->isEditable()) {
			return true;
		}
	}
	return false;
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
// but not to pass them to widgets every time
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

	bool widgetKeeped = _sprite->_cast && _widget;

	// if there's a video in the old sprite that's different, stop it before we continue
	if (_sprite->_castId != nextSprite->_castId && _sprite->_cast && _sprite->_cast->_type == kCastDigitalVideo) {
		((DigitalVideoCastMember *)_sprite->_cast)->setChannel(nullptr);
		((DigitalVideoCastMember *)_sprite->_cast)->stopVideo();
		((DigitalVideoCastMember *)_sprite->_cast)->rewindVideo();
	}

	// update the _sprite we stored in channel, and point the originalSprite to the new one
	// release the widget, because we may having the new one
	if (_sprite->_cast && !canKeepWidget(_sprite, nextSprite)) {
		widgetKeeped = false;
		_sprite->_cast->releaseWidget();
	}

	// If the cast member is the same, persist the editable flag
	bool editable = nextSprite->_editable;
	if (_sprite->_castId == nextSprite->_castId) {
		editable = _sprite->_editable;
	}

	int width = _sprite->_width;
	int height = _sprite->_height;
	bool immediate = _sprite->_immediate;

	*_sprite = *nextSprite;

	// Persist the immediate flag
	_sprite->_immediate = immediate;

	_sprite->_editable = editable;

	// TODO: auto expand text size is meaning less for us, not all text
	// since we are using initialRect for the text cast member now, then the sprite size is meaning less for us.
	// thus, we keep the _sprite size here
	if (hasTextCastMember(_sprite) && widgetKeeped) {
		_sprite->_width = width;
		_sprite->_height = height;
	}

	if (g_director->getVersion() >= 600) {
		_startFrame = _sprite->_spriteInfo.startFrame;
		_endFrame = _sprite->_spriteInfo.endFrame;
	}
}

void Channel::setPosition(int x, int y, bool force) {
	Common::Point newPos(x, y);
	if (_constraint > 0 && _score && _constraint <= _score->_channels.size()) {
		Common::Rect constraintBbox = _score->_channels[_constraint]->getRollOverBbox();
		newPos.x = MIN(constraintBbox.right, MAX(constraintBbox.left, newPos.x));
		newPos.y = MIN(constraintBbox.bottom, MAX(constraintBbox.top, newPos.y));
	}
	_sprite->setPosition(newPos.x, newPos.y);
	// Update the dimensons on the widget
	if (_widget) {
		Common::Rect dims = _widget->getDimensions();
		dims.translate(newPos.x - dims.left, newPos.y - dims.top);
		_widget->setDimensions(dims);
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
		// Check if _widget is of type window, in which case we need to remove it from the window manager
		if (dynamic_cast<Graphics::MacWindow *>(_widget))
			g_director->_wm->removeWindow((Graphics::MacWindow *)_widget);
		else
			delete _widget;
		_widget = nullptr;
	}

	if (_sprite && _sprite->_cast) {
		// use sprite type to guide us how to draw the cast
		// if the type don't match, then we will set it as transparent. i.e. don't create widget
		if (!_sprite->checkSpriteType())
			return;

		if (_sprite->_cast->needsReload()) {
			_sprite->_cast->load();
		}
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
			}
		}
	}
}

bool Channel::updateWidget() {
	if (_sprite->_cast && (_sprite->_cast->_type == kCastDigitalVideo) && _sprite->_cast->isModified()) {
		replaceWidget();
		return true;
	}
	if (_widget && _widget->needsRedraw()) {
		if (_sprite->_cast) {
			_sprite->_cast->updateFromWidget(_widget, _sprite->_editable);
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
	if ((_sprite->_cast) && (_sprite->_cast->_type == kCastFilmLoop || _sprite->_cast->_type == kCastMovie)) {
		return true;
	}
	return false;
}

Common::Array<Channel> *Channel::getSubChannels() {
	if (_sprite->_cast) {
		Common::Rect bbox = getBbox();
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannels(bbox, _filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannels(bbox, _filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return nullptr;
}

CastMemberID Channel::getSubChannelSound1() {
	if (_sprite->_cast) {
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannelSound1(_filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannelSound2(_filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return CastMemberID();
}

CastMemberID Channel::getSubChannelSound2() {
	if (_sprite->_cast) {
		if (_sprite->_cast->_type == kCastFilmLoop)
			return ((FilmLoopCastMember *)_sprite->_cast)->getSubChannelSound1(_filmLoopFrame);
		else if (_sprite->_cast->_type == kCastMovie)
			return ((MovieCastMember *)_sprite->_cast)->getSubChannelSound2(_filmLoopFrame);
	}
	warning("Channel doesn't have any sub-channels");
	return CastMemberID();
}

} // End of namespace Director
