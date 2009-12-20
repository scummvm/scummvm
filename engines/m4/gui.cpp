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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"

#include "m4/globals.h"
#include "m4/events.h"
#include "m4/font.h"
#include "m4/graphics.h"
#include "m4/viewmgr.h"
#include "m4/gui.h"
#include "m4/midi.h"
#include "m4/scene.h"
#include "m4/m4.h"

namespace M4 {

//--------------------------------------------------------------------------
// DialogView class
//
// Defines a generic base class for dialogs, that some of the classes
// in the object hierharchy require as a parent
//--------------------------------------------------------------------------

void DialogView::close() {
	// Default to simply destroying the given dialog
	_vm->_viewManager->deleteView(this);
}

//--------------------------------------------------------------------------
// GUIObject class
//
// Defines a generic object that appears in a view
//--------------------------------------------------------------------------

GUIObject::GUIObject(View *owner, const Common::Rect &bounds) {
	_parent = owner;
	_bounds = bounds;
}

//--------------------------------------------------------------------------
// MenuObject class
//
// Defines a specialised GUI object that appears in a dialog
//--------------------------------------------------------------------------

MenuObject::MenuObject(DialogView *owner, int objectId, int xs, int ys, int width, int height,
					   bool greyed, bool transparent):
		GUIObject(owner, Common::Rect(xs, ys, xs + width, ys + height)) {

	_objectId = objectId;
	_bounds.top = ys;
	_bounds.bottom = ys + height - 1;
	_bounds.left = xs;
	_bounds.right = xs + width - 1;
	_transparent = transparent;
	_objectState = greyed ? OS_GREYED : OS_NORMAL;
	_callback = NULL;

	if (transparent) {
		_background = new M4Surface(width, height);
		Common::Rect srcBounds(xs, ys, xs + width - 1, ys + height - 1);
		_background->copyFrom(owner, srcBounds, 0, 0);
	} else {
		_background = NULL;
	}
}

MenuObject::~MenuObject() {
	if (_background)
		delete _background;
}

void MenuObject::onExecute() {
	// If a callback function has been specified, then execute it
	if (_callback)
		_callback(parent(), this);
}

//--------------------------------------------------------------------------
// MenuButton class
//
// Defines a button object
//--------------------------------------------------------------------------

MenuButton::MenuButton(DialogView *owner, int buttonId, int xs, int ys, int width, int height,
					   MenuButton::Callback callbackFn, bool greyed, bool transparent,
					   ObjectType buttonType):
		MenuObject(owner, buttonId, xs, ys, width, height, greyed, transparent) {

	_objectType = buttonType;
	_callback = callbackFn;
}

bool MenuButton::onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem) {
	bool redrawFlag = false;
	bool callbackFlag = false;
	bool handledFlag = true;

	if (_objectState == OS_GREYED)
		return false;

	switch (event) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_DOUBLECLICK:
		if (isInside(x, y)) {
			_objectState = OS_PRESSED;
			if (currentItem != NULL)
				currentItem = this;
			redrawFlag = true;
		} else {
			currentItem = NULL;
			if (_objectState != OS_NORMAL) {
				_objectState = OS_PRESSED;
				redrawFlag = true;
			}
		}
		break;

	case MEVENT_LEFT_DRAG:
	case MEVENT_DOUBLECLICK_DRAG:
		if (!currentItem) {
			return true;
		}
		if (isInside(x, y)) {
			if (_objectState != OS_PRESSED) {
				_objectState = OS_PRESSED;
				redrawFlag = true;
			}
		}
		else {
			if (_objectState != OS_MOUSEOVER) {
				_objectState = OS_MOUSEOVER;
				redrawFlag = true;
			}
		}
		break;

	case MEVENT_LEFT_RELEASE:
	case MEVENT_DOUBLECLICK_RELEASE:
		if (isInside(x, y)) {
			if (currentItem) {
				callbackFlag = true;
				if (_objectType == OBJTYPE_OM_SWITCH_ON)
					_objectType = OBJTYPE_OM_SWITCH_OFF;
				else if (_objectType == OBJTYPE_OM_SWITCH_OFF)
					_objectType = OBJTYPE_OM_SWITCH_ON;
			}
			else {
				currentItem = this;
			}

			_objectState = OS_MOUSEOVER;
			redrawFlag = true;

		} else {
			currentItem = NULL;
			_objectState = OS_MOUSEOVER;
			redrawFlag = true;
			handledFlag = false;
		}
		break;

	case MEVENT_MOVE:
		if (isInside(x, y)) {
			currentItem = this;
			if (_objectState != OS_MOUSEOVER) {
				_objectState = OS_MOUSEOVER;
				redrawFlag = true;
			}
		}
		else {
			currentItem = NULL;
			if (_objectState != OS_NORMAL) {
				_objectState = OS_NORMAL;
				redrawFlag = true;
				handledFlag = false;
			}
		}
		break;

	case MEVENT_LEFT_HOLD:
	case MEVENT_DOUBLECLICK_HOLD:
		break;

	default:
		break;
	}

	//see if we need to redraw the button
	if (redrawFlag) {
		onRefresh();

		// TODO: There may be a more efficient mechanism than refreshing the entire screen
		// when a menu object refreshes itself
		if (parent()->screenFlags().visible)
			_vm->_viewManager->refreshAll();
	}

	// If a callback is flagged, then handle it

	if (callbackFlag)
		onExecute();

	return handledFlag;
}

void MenuButton::onRefresh() {
	M4Sprite *sprite = NULL;
	SpriteAsset &sprites = *parent()->sprites();

	// Switch handling for the various button types
	switch (_objectType) {
	case OBJTYPE_BUTTON:
		sprite = sprites[GM_BUTTON_GREYED + _objectState];
		break;

	case OBJTYPE_OM_SWITCH_ON:
		switch (_objectState) {
		case OS_MOUSEOVER:
			sprite = sprites[MENU_SS_SWITCH_ON_MOUSEOVER];
			break;
		case OS_PRESSED:
			sprite = sprites[MENU_SS_SWITCH_ON_PRESSED];
			break;
		default:
			sprite = sprites[MENU_SS_SWITCH_ON_NORMAL];
			break;
		}
		break;

	case OBJTYPE_OM_SWITCH_OFF:
		switch (_objectState) {
		case OS_MOUSEOVER:
			sprite = sprites[MENU_SS_SWITCH_OFF_MOUSEOVER];
			break;
		case OS_PRESSED:
			sprite = sprites[MENU_SS_SWITCH_OFF_PRESSED];
			break;
		default:
			sprite = sprites[MENU_SS_SWITCH_OFF_NORMAL];
			break;
		}
		break;

	case OBJTYPE_OM_DONE:
		sprite = sprites[OM_DONE_BTN_GREYED + _objectState];
		break;

	case OBJTYPE_OM_CANCEL:
		sprite = (_objectState == OS_GREYED) ? sprites[OM_CANCEL_BTN_NORMAL] :
			sprites[OM_CANCEL_BTN_NORMAL + _objectState - 1];
		break;

	case OBJTYPE_SL_SAVE:
		sprite = sprites[SL_SAVE_BTN_GREYED + _objectState];
		break;

	case OBJTYPE_SL_LOAD:
		sprite = sprites[SL_LOAD_BTN_GREYED + _objectState];
		break;

	case OBJTYPE_SL_CANCEL:
		sprite = (_objectState == OS_GREYED) ? sprites[SL_CANCEL_BTN_NORMAL] :
			sprites[SL_CANCEL_BTN_NORMAL + _objectState - 1];
		break;

	case OBJTYPE_SL_TEXT:
		switch (_objectState) {
		case OS_MOUSEOVER:
			_vm->_font->setColors(TEXT_COLOR_MOUSEOVER_SHADOW, TEXT_COLOR_MOUSEOVER_FOREGROUND,
				TEXT_COLOR_MOUSEOVER_HILIGHT);
			sprite = sprites[SL_LINE_MOUSEOVER];
			break;

		case OS_PRESSED:
			_vm->_font->setColors(TEXT_COLOR_PRESSED_SHADOW, TEXT_COLOR_PRESSED_FOREGROUND,
				TEXT_COLOR_PRESSED_HILIGHT);
			sprite = sprites[SL_LINE_PRESSED];
			break;

		case OS_GREYED:
			_vm->_font->setColors(TEXT_COLOR_GREYED_SHADOW, TEXT_COLOR_GREYED_FOREGROUND,
				TEXT_COLOR_GREYED_HILIGHT);
			sprite = sprites[SL_LINE_NORMAL];
			break;

		default:
		case OS_NORMAL:
			_vm->_font->setColors(TEXT_COLOR_NORMAL_SHADOW, TEXT_COLOR_NORMAL_FOREGROUND,
				TEXT_COLOR_NORMAL_HILIGHT);
			sprite = sprites[SL_LINE_NORMAL];
			break;
		}
		break;

	default:
		error("Unknown object type");
		break;
	}

	// If no sprite object was set, then exit without doing anything
	if (!sprite)
		return;

	// Draw the button
	if (_transparent) {
		// Transparent button, so restore original background
		if (!_background)
			return;
		else
			_background->copyTo(parent(), _bounds.left, _bounds.top);
	}

	sprite->copyTo(parent(), _bounds.left, _bounds.top, 0);
}

//--------------------------------------------------------------------------
// MenuHorizSlider class
//
// Defines a horizontal slider that allows selection of a percentage
//--------------------------------------------------------------------------

MenuHorizSlider::MenuHorizSlider(DialogView *owner, int objectId, int xs, int ys,
		int width, int height, int initialPercentage, Callback callbackFn, bool transparent):
		MenuObject(owner, objectId, xs, ys, width, height, false, transparent) {

	_objectType = OBJTYPE_SLIDER;
	_callback = callbackFn;

	SpriteAsset &sprites = *owner->sprites();
	_sliderState = HSLIDER_THUMB_NORMAL;
	_thumbSize.x = sprites[OM_SLIDER_BTN_NORMAL]->width();
	_thumbSize.y = sprites[OM_SLIDER_BTN_NORMAL]->height();
	_maxThumbX = width - _thumbSize.x;
	_percent = MAX(MIN(initialPercentage, 100), 0);
	_thumbX = initialPercentage * _maxThumbX / 100;
}

void MenuHorizSlider::onRefresh() {
	// If the slider is transparent, first copy in the background
	if (_transparent) {
		// Transparent button
		if (!_background)
			return;

		_background->copyTo(parent(), _bounds.left, _bounds.top, 0);
	}

	// Get the thumb sprite for the slider
	SpriteAsset &sprites = *parent()->sprites();
	M4Sprite *sprite = sprites[OM_SLIDER_BTN_NORMAL + _sliderState];
	assert(sprite);

	// Fill in the area to the left of the thumbnail
	if (_thumbX > 2) {
		Common::Rect leftBounds(_bounds.left + 3, _bounds.top + 9, _bounds.left + _thumbX,
			_bounds.top + _thumbSize.y - 9);
		parent()->fillRect(leftBounds, SLIDER_BAR_COLOR);
	}

	// Draw the thumbnail
	sprite->copyTo(parent(), _bounds.left + _thumbX, _bounds.top, 0);
}

bool MenuHorizSlider::onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem) {
	static bool movingFlag = false;
	static int movingX = 0;
	bool redrawFlag = false, handledFlag = false, callbackFlag = false;

	if (event == KEVENT_KEY)
		return false;

	switch (event) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_DOUBLECLICK:
		if (isInside(x, y) && (x - _bounds.left >= _thumbX) &&
			(x - _bounds.left <= _thumbX + _thumbSize.x - 1)) {
			// The thumbnail has been clicked
			_sliderState = HSLIDER_THUMB_PRESSED;
			movingFlag = true;
			movingX = x;
			currentItem = this;
			redrawFlag = true;
		} else {
			currentItem = NULL;
			_sliderState = HSLIDER_THUMB_NORMAL;
			redrawFlag = true;
		}
		redrawFlag = true;
		break;

	case MEVENT_LEFT_DRAG:
	case MEVENT_DOUBLECLICK_DRAG:
		if (!currentItem)
			return true;

		if (movingFlag) {
			if (x != movingX) {
				if (x < movingX)
					_thumbX -= MIN(_thumbX, movingX - x);
				else
					_thumbX += MIN(_maxThumbX - _thumbX, x - movingX);
				_percent = _thumbX * 100 / _maxThumbX;
				redrawFlag = callbackFlag = true;
			}
			movingX = CLIP(x, _bounds.left + _thumbX,
							_bounds.left + _thumbX + _thumbSize.x - 1);
		} else {
			currentItem = NULL;
		}
		break;

	case MEVENT_LEFT_RELEASE:
	case MEVENT_DOUBLECLICK_RELEASE:
		if (!currentItem)
			return true;

		movingFlag = false;
		if (isInside(x, y) && (x - _bounds.left >= _thumbX) &&
				(x - _bounds.left <= _thumbX + _thumbSize.x - 1)) {
			_sliderState = HSLIDER_THUMB_MOUSEOVER;
			currentItem = this;
		} else {
			_sliderState = HSLIDER_THUMB_NORMAL;
			currentItem = NULL;
		}

		redrawFlag = true;
		callbackFlag = true;
		break;

	case MEVENT_MOVE:
		if (isInside(x, y) && (x - _bounds.left >= _thumbX) &&
				(x - _bounds.left <= _thumbX + _thumbSize.x - 1)) {
			if (_sliderState != HSLIDER_THUMB_MOUSEOVER) {
				_sliderState = HSLIDER_THUMB_MOUSEOVER;
				currentItem = this;
			}
		} else {
			if (_sliderState != HSLIDER_THUMB_NORMAL) {
				_sliderState = HSLIDER_THUMB_NORMAL;
				currentItem = NULL;
				handledFlag = false;
			}
		}
		redrawFlag = true;
		break;

	default:
		break;
	}

	if (redrawFlag)
		onRefresh();

	if (callbackFlag)
		onExecute();

	return handledFlag;
}

//--------------------------------------------------------------------------
// MenuVertSlider class
//
// Defines a vertical slider that's used in the save/load dialog
//--------------------------------------------------------------------------

MenuVertSlider::MenuVertSlider(DialogView *owner, int objectId, int xs, int ys,
		int width, int height, int initialPercentage, Callback callbackFn, bool transparent):
		MenuObject(owner, objectId, xs, ys, width, height, false, transparent) {

	_objectType = OBJTYPE_SLIDER;
	_callback = callbackFn;

	SpriteAsset &sprites = *owner->sprites();
	_sliderState = VSLIDER_NONE;
	_thumbSize.x = sprites[SL_SLIDER_BTN_NORMAL]->width();
	_thumbSize.y = sprites[SL_SLIDER_BTN_NORMAL]->height();
	_minThumbY = sprites[SL_UP_BTN_NORMAL]->height() + 1;
	_maxThumbY = sprites[SL_UP_BTN_NORMAL]->height() + sprites[SL_SCROLLBAR]->height() -
		sprites[SL_SLIDER_BTN_NORMAL]->height() - 1;

	_percent = MAX(MIN(initialPercentage, 100), 0);
	_thumbY = _minThumbY + ((_percent * (_maxThumbY - _minThumbY)) / 100);
}

MenuVertSliderState MenuVertSlider::getSliderArea(int y) {
	if (y < _minThumbY)
		return VSLIDER_UP;
	else if (y < _thumbY)
		return VSLIDER_PAGE_UP;
	else if (y < _thumbY + _thumbSize.y)
		return VSLIDER_THUMBNAIL;
	else if (y < _maxThumbY + _thumbSize.y)
		return VSLIDER_PAGE_DOWN;
	else
		return VSLIDER_DOWN;
}

void MenuVertSlider::onRefresh() {
	// If the slider is transparent, first copy in the background
	if (_transparent) {
		// Transparent button
		if (!_background)
			return;

		_background->copyTo(parent(), _bounds.left, _bounds.top, 0);
	}

	// Get the various needed sprites
	SpriteAsset &sprites = *parent()->sprites();
	M4Sprite *barSprite = sprites[SL_SCROLLBAR];
	M4Sprite *thumbSprite = sprites[SL_SLIDER_BTN_NORMAL];
	M4Sprite *upSprite = sprites[SL_UP_BTN_NORMAL];
	M4Sprite *downSprite = sprites[SL_DOWN_BTN_NORMAL];

	if (_objectState == OS_GREYED) {
		upSprite = sprites[SL_UP_BTN_GREYED];
		downSprite = sprites[SL_DOWN_BTN_GREYED];
		thumbSprite = NULL;

	} else if (_objectState == OS_MOUSEOVER) {
		if (_sliderState == VSLIDER_UP)
			upSprite = sprites[SL_UP_BTN_MOUSEOVER];
		else if (_sliderState == VSLIDER_THUMBNAIL)
			thumbSprite = sprites[SL_SLIDER_BTN_MOUSEOVER];
		else if (_sliderState == VSLIDER_DOWN)
			downSprite = sprites[SL_DOWN_BTN_MOUSEOVER];
	}
	else if (_objectState == OS_PRESSED) {
		if (_sliderState == VSLIDER_UP)
			upSprite = sprites[SL_UP_BTN_PRESSED];
		else if (_sliderState == VSLIDER_THUMBNAIL)
			thumbSprite = sprites[SL_SLIDER_BTN_PRESSED];
		else if (_sliderState == VSLIDER_DOWN)
			downSprite = sprites[SL_DOWN_BTN_PRESSED];
	}

	// Draw the sprites
	upSprite->copyTo(parent(), _bounds.left, _bounds.top, 0);
	barSprite->copyTo(parent(), _bounds.left, _bounds.top + upSprite->height(), 0);
	downSprite->copyTo(parent(), _bounds.left, _bounds.top + upSprite->height() + barSprite->height(), 0);
	if (thumbSprite)
		thumbSprite->copyTo(parent(), _bounds.left, _bounds.top + _thumbY, 0);
}

bool MenuVertSlider::onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem) {
	static bool movingFlag = false;
	static int movingY = 0;
	static uint32 callbackTime;
	MenuVertSliderState tempState;
	int delta;
	uint32 currentTime = g_system->getMillis();
	bool redrawFlag = false;
	bool handledFlag = true;
	bool callbackFlag = false;

	if (event == KEVENT_KEY)
		return false;

	if (_objectState == OS_GREYED) {
		currentItem = NULL;
		return false;
	}

	switch (event) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_DOUBLECLICK:
		if (isInside(x, y)) {
			currentItem = this;
			tempState = getSliderArea(y - _bounds.top);
			if (tempState == VSLIDER_THUMBNAIL) {
				movingFlag = true;
				movingY = y;
			}
			if ((tempState == VSLIDER_PAGE_UP) || (tempState == VSLIDER_PAGE_DOWN)) {
				_sliderState = tempState;
				setState(OS_NORMAL);
			} else {
				_sliderState = tempState;
				setState(OS_PRESSED);
				redrawFlag = true;
			}
			callbackFlag = true;
		} else {
			currentItem = NULL;
			setState(OS_NORMAL);
			redrawFlag = true;
		}
		break;

	case MEVENT_LEFT_DRAG:
	case MEVENT_DOUBLECLICK_DRAG:
		if (!currentItem)
			return true;

		if (movingFlag) {
			if (y < movingY) {
				delta = MIN(_thumbY - _minThumbY, movingY - y);
				if (delta > 0) {
					_thumbY -= delta;
					_percent = ((_thumbY - _minThumbY) * 100) / (_maxThumbY - _minThumbY);
					redrawFlag = true;
					callbackFlag = true;
				}
			}
			else if (y > movingY) {
				delta = MIN(_maxThumbY - _thumbY, y - movingY);
				if (delta > 0) {
					_thumbY += delta;
					_percent = ((_thumbY - _minThumbY) * 100) / (_maxThumbY - _minThumbY);
					redrawFlag = true;
					callbackFlag = true;
				}
			}
			movingY = y;

			if (movingY < (_thumbY + _bounds.top))
				movingY = _thumbY + _bounds.top;
			else if (movingY > (_bounds.top + _thumbY + _thumbSize.y - 1))
				movingY = _bounds.top + _thumbY + _thumbSize.y - 1;

		} else {
			if (isInside(x, y)) {
				tempState = getSliderArea(y - _bounds.top);
				if (_sliderState == tempState) {
					if ((tempState != VSLIDER_PAGE_UP) && (tempState != VSLIDER_PAGE_DOWN) &&
						(_objectState != OS_PRESSED)) {
						_sliderState = tempState;
						setState(OS_PRESSED);
						redrawFlag = true;
					}
					if (currentTime - callbackTime > 100)
						callbackFlag = true;

				} else {
					if (_objectState != OS_MOUSEOVER)
						setState(OS_MOUSEOVER);
						redrawFlag = true;
				}
				callbackFlag = true;

			} else {
				if (_objectState != OS_MOUSEOVER) {
					setState(OS_MOUSEOVER);
					redrawFlag = true;
				}
			}
		}
		break;

	case MEVENT_LEFT_RELEASE:
	case MEVENT_DOUBLECLICK_RELEASE:
		movingFlag = false;
		if (isInside(x, y)) {
			tempState = getSliderArea(y - _bounds.top);
			if ((tempState == VSLIDER_PAGE_UP) || (tempState == VSLIDER_PAGE_DOWN))
				setState(OS_NORMAL);
			else {
				setState(OS_MOUSEOVER);
				currentItem = this;
			}
		} else {
			setState(OS_NORMAL);
			currentItem = NULL;
		}

		redrawFlag = true;
		if (parent()->getMenuType() == LOAD_MENU)
			updateThumbnails();
		break;

	case MEVENT_MOVE:
		if (isInside(x, y)) {
			currentItem = this;
			tempState = getSliderArea(y - _bounds.top);
			if (_sliderState != tempState) {
				if ((tempState == VSLIDER_PAGE_UP) || (tempState == VSLIDER_PAGE_DOWN))
					_objectState = OS_NORMAL;
				else {
					_sliderState = tempState;
					_objectState = OS_MOUSEOVER;
				}
				redrawFlag = true;
			}
		} else {
			currentItem = NULL;

			if (_objectState != OS_NORMAL) {
				_objectState = OS_NORMAL;
				redrawFlag = true;
				handledFlag = false;
			}
		}
		break;

	case MEVENT_LEFT_HOLD:
	case MEVENT_DOUBLECLICK_HOLD:
		if (!currentItem)
			return true;

		if (isInside(x, y)) {
			tempState = getSliderArea(y - _bounds.top);

			if (_sliderState == tempState) {
				if (currentTime - callbackTime > 100)
					callbackFlag = true;
			}
		}
		break;

	default:
		break;
	}

	if (redrawFlag)
		onRefresh();

	if (callbackFlag) {
		callbackTime = currentTime;
		onExecute();
	}

	return handledFlag;
}

void MenuVertSlider::setPercentage(int value) {
	_percent = value;
	_thumbY = _minThumbY + ((_percent * (_maxThumbY - _minThumbY)) / 100);
	onRefresh();
}

//--------------------------------------------------------------------------
// MenuMessage class
//
// Defines a message menu object
//--------------------------------------------------------------------------

MenuMessage::MenuMessage(DialogView *owner, int objectId, int xs, int ys, int width, int height,
						 bool transparent):
		MenuObject(owner, objectId, xs, ys, width, height, false, transparent) {
}

void MenuMessage::onRefresh() {
	SpriteAsset &sprites = *parent()->sprites();
	M4Surface *sprite = NULL;

	// Get the correct sprite to use
	switch (_objectId) {
	case SLTAG_SAVELOAD_LABEL:
		sprite = (parent()->getMenuType() == SAVE_MENU) ? sprites[SL_SAVE_LABEL] :
			sprites[SL_LOAD_LABEL];
		break;
	}
	assert(sprite);

	// Draw the sprite
	if (_transparent) {
		// Transparent button
		if (!_background)
			return;

		// Restore original background and then do a transparent copy of the sprite
		_background->copyTo(parent(), _bounds.left, _bounds.top);
	}

	sprite->copyTo(parent(), _bounds.left, _bounds.top, 0);
}

//--------------------------------------------------------------------------
// MenuImage class
//
// Defines a menu item that displays a given surface
//--------------------------------------------------------------------------

MenuImage::MenuImage(DialogView *owner, int objectId, int xs, int ys, int width, int height,
					 M4Surface *image, bool transparent):
		MenuObject(owner, objectId, xs, ys, width, height, false, transparent) {

	_sprite = image;
}

void MenuImage::onRefresh() {
	if (!_sprite)
		return;

	// Draw the sprite
	if (_transparent) {
		// Transparent button
		if (!_background)
			return;

		// Restore original background and then do a transparent copy of the sprite
		_background->copyTo(parent(), _bounds.left, _bounds.top);
	}

	_sprite->copyTo(parent(), _bounds.left + (_bounds.width() - _sprite->width()) / 2,
		_bounds.top + (_bounds.height() - _sprite->height()) / 2, 0);
}

//--------------------------------------------------------------------------
// MenuSaveLoadText class
//
// Defines a save/load dialog text entry
//--------------------------------------------------------------------------

MenuSaveLoadText::MenuSaveLoadText(DialogView *owner, int textId, int xs, int ys,
		int width, int height, Callback callbackFn, bool greyed, bool transparent,
		bool loadFlag, const char *displayText, int displayValue):
		MenuButton(owner, textId, xs, ys, width, height, callbackFn, greyed, transparent, OBJTYPE_SL_TEXT) {

	_loadFlag = loadFlag;
	_displayText = displayText;
	_displayValue = displayValue;
	_index = textId - SLTAG_SLOTS_START;
	_visible = true;
}

void MenuSaveLoadText::onRefresh() {
	if (!_visible) return;
	_vm->_font->setFont(FONT_MENU);
	MenuButton::onRefresh();

	if ((_objectType == OBJTYPE_SL_TEXT) && (_displayText != NULL)) {
		int xp = _bounds.left + 4;
		if (_displayValue != 0) {
			char tempBuffer[5];
			sprintf(tempBuffer, "%02d", _displayValue);
			_vm->_font->writeString(_parent, tempBuffer, xp, _bounds.top + 1, 0, -1);
			xp = _bounds.left + 26;
		}

		_vm->_font->writeString(_parent, _displayText, xp, _bounds.top + 1, 0, -1);
	}
}

bool MenuSaveLoadText::onEvent(M4::M4EventType event, int32 param, int x, int y, M4::MenuObject *&currentItem) {
	if (!_visible) return false;
	bool handledFlag = MenuButton::onEvent(event, param, x, y, currentItem);

	// Further handling will only be done when in load mode and a slot hasn't been selected
	if (!_loadFlag || (parent()->_selectedSlot != -1))
		return handledFlag;

	// Only handling for certain event types
	if ((event != MEVENT_MOVE) && (event != MEVENT_LEFT_DRAG) &&
		(event != MEVENT_LEFT_RELEASE) && (event != MEVENT_DOUBLECLICK_DRAG) &&
		(event != MEVENT_DOUBLECLICK_RELEASE))
		return handledFlag;

	MenuImage *thumbnail = (MenuImage *) parent()->getItem(SLTAG_THUMBNAIL);

	// Check whether the cursor is over the button
	if ((_objectState == OS_MOUSEOVER) || (_objectState == OS_PRESSED)) {
		if (_index != parent()->_highlightedSlot) {
			// Selected slot has changed
			if (parent()->_savegameThumbnail != NULL)
				delete parent()->_savegameThumbnail;

			parent()->_highlightedSlot = _index;
			parent()->_savegameThumbnail = _vm->_saveLoad->getThumbnail(_index + 1);
			thumbnail->setSprite(parent()->_savegameThumbnail);
		}

	} else {
		// If the mouse has moved outside the area of all the save slots, then the
		// thumbnail needs to be removed

		Common::Rect slotArea(50, 256, 288, 377);
		if (isInside(x, y) || !slotArea.contains(x, y)) {
			if (parent()->_savegameThumbnail) {
				delete parent()->_savegameThumbnail;
				parent()->_savegameThumbnail = NULL;
			}

			thumbnail->setSprite(parent()->sprites()->getFrame(SL_EMPTY_THUMBNAIL));
			parent()->_highlightedSlot = -1;
		}
	}

	return handledFlag;
}

void MenuSaveLoadText::setVisible(bool value) {
	_visible = value;
	parent()->refresh(_bounds);
}

//--------------------------------------------------------------------------
// MenuTextField class
//
// Defines a text entry field
//--------------------------------------------------------------------------

MenuTextField::MenuTextField(DialogView *owner, int fieldId, int xs, int ys, int width,
							 int height, bool greyed, Callback callbackFn,
							 const char *displayText, int displayValue, bool transparent):
		MenuObject(owner, fieldId, xs, ys, width, height, greyed, transparent) {

	_displayValue = displayValue;
	_callback = callbackFn;
	_pixelWidth = width - 27;
	if (displayText) {
		strcpy(_displayText, displayText);
		_promptEnd = &_displayText[strlen(_displayText)];
	} else {
		_displayText[0] = '\0';
		_promptEnd = &_displayText[0];
	}
	_cursor = _promptEnd;
}

void MenuTextField::onRefresh() {
	bool focused = _objectState != OS_GREYED;

	// Get the thumb sprite for the slider
	SpriteAsset &sprites = *parent()->sprites();
	M4Sprite *sprite = sprites[SL_LINE_NORMAL + OS_MOUSEOVER - 1];

	// If the slider is transparent, first copy in the background
	if (_transparent) {
		// Transparent button
		if (!_background)
			return;

		_background->copyTo(parent(), _bounds.left, _bounds.top, 0);
	}

	// Draw the sprite
	sprite->copyTo(parent(), _bounds.left, _bounds.top, 0);

	// Draw the text

	_vm->_font->setFont(FONT_MENU);
	_vm->_font->setColors(TEXT_COLOR_NORMAL_SHADOW, TEXT_COLOR_NORMAL_FOREGROUND,
		TEXT_COLOR_NORMAL_HILIGHT);
	int xp = _bounds.left + 4;

	if (_displayValue != 0) {
		char tempBuffer[5];
		sprintf(tempBuffer, "%02d", _displayValue);
		_vm->_font->writeString(_parent, tempBuffer, xp, _bounds.top + 1, 0, -1);
		xp = _bounds.left + 26;
	}

	_vm->_font->writeString(_parent, _displayText, xp, _bounds.top + 1, 0, -1);

	if (focused) {
		// Draw in the cursor

		if (_cursor) {
			// Get the width of the string up to the cursor position
			char tempCh = *_cursor;
			*_cursor = '\0';
			int stringWidth = _vm->_font->getWidth(_displayText);
			*_cursor = tempCh;

			parent()->setColor(TEXT_COLOR_MOUSEOVER_FOREGROUND);
			parent()->vLine(xp + stringWidth + 1, _bounds.top + 1, _bounds.top + 12);
		}
	}
}

bool MenuTextField::onEvent(M4EventType event, int32 param, int x, int y, MenuObject *&currentItem) {
	char tempStr[MAX_SAVEGAME_NAME];
	int tempLen;
	char *tempP;
	bool handledFlag = false, redrawFlag = false, callbackFlag = false;

	if (_objectState == OS_GREYED)
		return false;

	switch (event) {
	case MEVENT_LEFT_CLICK:
	case MEVENT_DOUBLECLICK:
		parent()->_deleteSaveDesc = false;
		if (isInside(x, y))
			currentItem = this;
		break;

	case MEVENT_LEFT_RELEASE:
	case MEVENT_DOUBLECLICK_RELEASE:
		if (!currentItem)
			return true;
		currentItem = NULL;

		if (isInside(x, y)) {
			if (_objectState == OS_MOUSEOVER) {
				tempLen = strlen(_displayText);
				if (tempLen > 0) {
					strcpy(tempStr, _displayText);
					tempP = &tempStr[tempLen];
					_vm->_font->setFont(FONT_MENU);

					tempLen = _vm->_font->getWidth(tempStr);
					while ((tempP != &tempStr[0]) && (tempLen > x - _bounds.left - 26)) {
						*--tempP = '\0';
						tempLen = _vm->_font->getWidth(tempStr);
					}

					_cursor = &_displayText[tempP - &tempStr[0]];
					redrawFlag = true;
				}
			} else if (event == MEVENT_DOUBLECLICK_RELEASE) {
				callbackFlag = true;
			}
		}
		break;

	case KEVENT_KEY:
		switch (param) {
		case Common::KEYCODE_RETURN:
		case Common::KEYCODE_KP_ENTER:
			parent()->_deleteSaveDesc = false;
			callbackFlag = true;
			break;
			break;

		case Common::KEYCODE_HOME:
			parent()->_deleteSaveDesc = false;
			_cursor = &_displayText[0];
			redrawFlag = true;
			break;

		case Common::KEYCODE_END:
			parent()->_deleteSaveDesc = false;
			_cursor = _promptEnd;
			redrawFlag = true;
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			parent()->_deleteSaveDesc = false;
			if (_cursor > &_displayText[0]) {
				--_cursor;
				redrawFlag = true;
			}
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			parent()->_deleteSaveDesc = false;
			if (_cursor < _promptEnd) {
				++_cursor;
				redrawFlag = true;
			}
			break;

		case Common::KEYCODE_DELETE:
			if (parent()->_deleteSaveDesc) {
				_displayText[0] = '\0';
				_promptEnd = &_displayText[0];
				_cursor = _promptEnd;
				redrawFlag = true;
			} else if (_cursor < _promptEnd) {
				strcpy(tempStr, _cursor + 1);
				strcpy(_cursor, tempStr);
				--_promptEnd;
				redrawFlag = true;
			}
			break;

		case Common::KEYCODE_BACKSPACE:
			parent()->_deleteSaveDesc = false;
			if (_cursor > &_displayText[0]) {
				strcpy(tempStr, _cursor);
				--_promptEnd;
				--_cursor;
				strcpy(_cursor, tempStr);
				redrawFlag = true;
			}
			break;

		default:
			parent()->_deleteSaveDesc = false;
			_vm->_font->setFont(FONT_MENU);

			tempLen = _vm->_font->getWidth(_displayText);
			if ((strlen(_displayText) < MAX_SAVEGAME_NAME - 1) &&
				(tempLen < _pixelWidth - 12) && (param >= 32) && (param <= 127)) {

				// Valid displayable character
				if (_cursor < _promptEnd) {
					strcpy(tempStr, _cursor);
					sprintf(_cursor, "%c%s", (char)param, tempStr);
				} else {
					*_cursor = (char)param;
					*(_cursor + 1) = '\0';
				}
				++_cursor;
				++_promptEnd;

				redrawFlag = true;
			}
			break;
		}
		break;

	default:
		break;
	}

	if (redrawFlag)
		onRefresh();

	if (callbackFlag)
		onExecute();

	return handledFlag;
}

//--------------------------------------------------------------------------


GUITextField::GUITextField(View *owner, const Common::Rect &bounds): GUIRect(owner, bounds, 0) {
}

void GUITextField::onRefresh() {
	_parent->fillRect(_bounds, _vm->_palette->BLACK);
	_vm->_font->setColors(3, 3, 3);
	_vm->_font->setFont(FONT_INTERFACE);
	_vm->_font->writeString(_parent, _text.c_str(), _bounds.left, _bounds.top, 0, 1);
}

//--------------------------------------------------------------------------

GUIButton::GUIButton(View *owner, const Common::Rect &bounds, int tag,
		M4Surface *normalSprite, M4Surface *mouseOverSprite,  M4Surface *pressedSprite):
		GUIRect(owner, bounds, tag) {

	_normalSprite = normalSprite;
	_mouseOverSprite = mouseOverSprite;
	_pressedSprite = pressedSprite;
	_visible = true;
	_tracking = false;
}

void GUIButton::onRefresh() {
	_parent->fillRect(_bounds, _vm->_palette->BLACK);

	if (_visible) {
		switch (_buttonState) {
		case BUTTON_MOUSEOVER:
			_mouseOverSprite->copyTo(_parent, _bounds.left, _bounds.top, 0);
			break;
		case BUTTON_PRESSED:
			_pressedSprite->copyTo(_parent, _bounds.left, _bounds.top, 0);
			break;
		default:
			_normalSprite->copyTo(_parent, _bounds.left, _bounds.top, 0);
			break;
		}
	}
}

bool GUIButton::onEvent(M4EventType eventType, int32 param, int x, int y, GUIObject *&currentItem) {
	bool result = false;
	bool isPressed = (eventType == MEVENT_LEFT_CLICK) || (eventType == MEVENT_LEFT_HOLD) ||
		(eventType == MEVENT_LEFT_DRAG);
	bool isOver = isInside(x, y);
	GUIButtonState oldState = _buttonState;

	if (isOver) {
		if (isPressed) {
			// Button is pressed
			if (!_tracking) {
				_tracking = true;
				_buttonState = BUTTON_PRESSED;
			}

			_vm->_globals->invSuppressClickSound = false;
		} else {
			// Button isn't pressed
			if (_tracking)
				result = true;
			else {
				result = false;
				_buttonState = BUTTON_MOUSEOVER;
				_tracking = false;
			}
		}
	} else {
		_tracking = false;
		_buttonState = BUTTON_NORMAL;
	}

	if (oldState != _buttonState)
		onRefresh();

	return result;
}

} // End of namespace M4
