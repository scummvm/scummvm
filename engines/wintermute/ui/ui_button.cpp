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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIButton, false)

//////////////////////////////////////////////////////////////////////////
UIButton::UIButton(BaseGame *inGame) : UIObject(inGame) {
	_backPress = _backHover = _backDisable = _backFocus = nullptr;

	_fontHover = _fontPress = _fontDisable = _fontFocus = nullptr;

	_imageDisable = _imagePress = _imageHover = _imageFocus = nullptr;

	_align = TAL_CENTER;

	_hover = _press = false;

	_type = UI_BUTTON;

	_canFocus = false;
	_stayPressed = false;

	_oneTimePress = false;
	_centerImage = false;

	_pixelPerfect = false;
}


//////////////////////////////////////////////////////////////////////////
UIButton::~UIButton() {
	delete _backPress;
	delete _backHover;
	delete _backDisable;
	delete _backFocus;

	if (!_sharedFonts) {
		if (_fontHover) {
			_gameRef->_fontStorage->removeFont(_fontHover);
		}
		if (_fontPress) {
			_gameRef->_fontStorage->removeFont(_fontPress);
		}
		if (_fontDisable) {
			_gameRef->_fontStorage->removeFont(_fontDisable);
		}
		if (_fontFocus) {
			_gameRef->_fontStorage->removeFont(_fontFocus);
		}
	}

	if (!_sharedImages) {
		delete _imageHover;
		delete _imagePress;
		delete _imageDisable;
		delete _imageFocus;
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIButton::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "UIButton::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing BUTTON file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(BUTTON)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(FOCUSABLE)
TOKEN_DEF(BACK_HOVER)
TOKEN_DEF(BACK_PRESS)
TOKEN_DEF(BACK_DISABLE)
TOKEN_DEF(BACK_FOCUS)
TOKEN_DEF(BACK)
TOKEN_DEF(CENTER_IMAGE)
TOKEN_DEF(IMAGE_HOVER)
TOKEN_DEF(IMAGE_PRESS)
TOKEN_DEF(IMAGE_DISABLE)
TOKEN_DEF(IMAGE_FOCUS)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_HOVER)
TOKEN_DEF(FONT_PRESS)
TOKEN_DEF(FONT_DISABLE)
TOKEN_DEF(FONT_FOCUS)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(EVENTS)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(PRESSED)
TOKEN_DEF(PIXEL_PERFECT)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool UIButton::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(BUTTON)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(FOCUSABLE)
	TOKEN_TABLE(BACK_HOVER)
	TOKEN_TABLE(BACK_PRESS)
	TOKEN_TABLE(BACK_DISABLE)
	TOKEN_TABLE(BACK_FOCUS)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(CENTER_IMAGE)
	TOKEN_TABLE(IMAGE_HOVER)
	TOKEN_TABLE(IMAGE_PRESS)
	TOKEN_TABLE(IMAGE_DISABLE)
	TOKEN_TABLE(IMAGE_FOCUS)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_HOVER)
	TOKEN_TABLE(FONT_PRESS)
	TOKEN_TABLE(FONT_DISABLE)
	TOKEN_TABLE(FONT_FOCUS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(PRESSED)
	TOKEN_TABLE(PIXEL_PERFECT)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	char *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_BUTTON) {
			_gameRef->LOG(0, "'BUTTON' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_NAME:
			setName(params);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
			break;

		case TOKEN_BACK:
			delete _back;
			_back = new UITiledImage(_gameRef);
			if (!_back || DID_FAIL(_back->loadFile(params))) {
				delete _back;
				_back = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_HOVER:
			delete _backHover;
			_backHover = new UITiledImage(_gameRef);
			if (!_backHover || DID_FAIL(_backHover->loadFile(params))) {
				delete _backHover;
				_backHover = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_PRESS:
			delete _backPress;
			_backPress = new UITiledImage(_gameRef);
			if (!_backPress || DID_FAIL(_backPress->loadFile(params))) {
				delete _backPress;
				_backPress = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_DISABLE:
			delete _backDisable;
			_backDisable = new UITiledImage(_gameRef);
			if (!_backDisable || DID_FAIL(_backDisable->loadFile(params))) {
				delete _backDisable;
				_backDisable = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_FOCUS:
			delete _backFocus;
			_backFocus = new UITiledImage(_gameRef);
			if (!_backFocus || DID_FAIL(_backFocus->loadFile(params))) {
				delete _backFocus;
				_backFocus = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new BaseSprite(_gameRef);
			if (!_image || DID_FAIL(_image->loadFile(params))) {
				delete _image;
				_image = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
			delete _imageHover;
			_imageHover = new BaseSprite(_gameRef);
			if (!_imageHover || DID_FAIL(_imageHover->loadFile(params))) {
				delete _imageHover;
				_imageHover = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_PRESS:
			delete _imagePress;
			_imagePress = new BaseSprite(_gameRef);
			if (!_imagePress || DID_FAIL(_imagePress->loadFile(params))) {
				delete _imagePress;
				_imagePress = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_DISABLE:
			delete _imageDisable;
			_imageDisable = new BaseSprite(_gameRef);
			if (!_imageDisable || DID_FAIL(_imageDisable->loadFile(params))) {
				delete _imageDisable;
				_imageDisable = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_FOCUS:
			delete _imageFocus;
			_imageFocus = new BaseSprite(_gameRef);
			if (!_imageFocus || DID_FAIL(_imageFocus->loadFile(params))) {
				delete _imageFocus;
				_imageFocus = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) {
				_gameRef->_fontStorage->removeFont(_font);
			}
			_font = _gameRef->_fontStorage->addFont(params);
			if (!_font) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_HOVER:
			if (_fontHover) {
				_gameRef->_fontStorage->removeFont(_fontHover);
			}
			_fontHover = _gameRef->_fontStorage->addFont(params);
			if (!_fontHover) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_PRESS:
			if (_fontPress) {
				_gameRef->_fontStorage->removeFont(_fontPress);
			}
			_fontPress = _gameRef->_fontStorage->addFont(params);
			if (!_fontPress) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_DISABLE:
			if (_fontDisable) {
				_gameRef->_fontStorage->removeFont(_fontDisable);
			}
			_fontDisable = _gameRef->_fontStorage->addFont(params);
			if (!_fontDisable) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_FOCUS:
			if (_fontFocus) {
				_gameRef->_fontStorage->removeFont(_fontFocus);
			}
			_fontFocus = _gameRef->_fontStorage->addFont(params);
			if (!_fontFocus) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_TEXT:
			setText(params);
			_gameRef->expandStringByStringTable(&_text);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp(params, "left") == 0) {
				_align = TAL_LEFT;
			} else if (scumm_stricmp(params, "right") == 0) {
				_align = TAL_RIGHT;
			} else {
				_align = TAL_CENTER;
			}
			break;

		case TOKEN_X:
			parser.scanStr(params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr(params, "%d", &_posY);
			break;

		case TOKEN_WIDTH:
			parser.scanStr(params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.scanStr(params, "%d", &_height);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				delete _cursor;
				_cursor = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.scanStr(params, "%b", &_parentNotify);
			break;

		case TOKEN_DISABLED:
			parser.scanStr(params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr(params, "%b", &_visible);
			break;

		case TOKEN_FOCUSABLE:
			parser.scanStr(params, "%b", &_canFocus);
			break;

		case TOKEN_CENTER_IMAGE:
			parser.scanStr(params, "%b", &_centerImage);
			break;

		case TOKEN_PRESSED:
			parser.scanStr(params, "%b", &_stayPressed);
			break;

		case TOKEN_PIXEL_PERFECT:
			parser.scanStr(params, "%b", &_pixelPerfect);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in BUTTON definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading BUTTON definition");
		return STATUS_FAILED;
	}

	correctSize();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIButton::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "BUTTON\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->getFilename());
	}
	if (_backHover && _backHover->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK_HOVER=\"%s\"\n", _backHover->getFilename());
	}
	if (_backPress && _backPress->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK_PRESS=\"%s\"\n", _backPress->getFilename());
	}
	if (_backDisable && _backDisable->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK_DISABLE=\"%s\"\n", _backDisable->getFilename());
	}
	if (_backFocus && _backFocus->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK_FOCUS=\"%s\"\n", _backFocus->getFilename());
	}

	if (_image && _image->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->getFilename());
	}
	if (_imageHover && _imageHover->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE_HOVER=\"%s\"\n", _imageHover->getFilename());
	}
	if (_imagePress && _imagePress->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE_PRESS=\"%s\"\n", _imagePress->getFilename());
	}
	if (_imageDisable && _imageDisable->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE_DISABLE=\"%s\"\n", _imageDisable->getFilename());
	}
	if (_imageFocus && _imageFocus->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE_FOCUS=\"%s\"\n", _imageFocus->getFilename());
	}

	if (_font && _font->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->getFilename());
	}
	if (_fontHover && _fontHover->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->getFilename());
	}
	if (_fontPress && _fontPress->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT_PRESS=\"%s\"\n", _fontPress->getFilename());
	}
	if (_fontDisable && _fontDisable->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT_DISABLE=\"%s\"\n", _fontDisable->getFilename());
	}
	if (_fontFocus && _fontFocus->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT_FOCUS=\"%s\"\n", _fontFocus->getFilename());
	}

	if (_cursor && _cursor->getFilename()) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->getFilename());
	}


	buffer->putTextIndent(indent + 2, "\n");

	if (_text) {
		buffer->putTextIndent(indent + 2, "TEXT=\"%s\"\n", _text);
	}

	switch (_align) {
	case TAL_LEFT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	default:
		warning("UIButton::SaveAsText - unhandled enum");
		break;
	}

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);


	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "FOCUSABLE=%s\n", _canFocus ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "CENTER_IMAGE=%s\n", _centerImage ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PRESSED=%s\n", _stayPressed ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PIXEL_PERFECT=%s\n", _pixelPerfect ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "\n");

	// scripts
	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void UIButton::correctSize() {
	Rect32 rect;

	BaseSprite *img = nullptr;
	if (_image) {
		img = _image;
	} else if (_imageDisable) {
		img = _imageDisable;
	} else if (_imageHover) {
		img = _imageHover;
	} else if (_imagePress) {
		img = _imagePress;
	} else if (_imageFocus) {
		img = _imageFocus;
	}

	if (_width <= 0) {
		if (img) {
			img->getBoundingRect(&rect, 0, 0);
			_width = rect.right - rect.left;
		} else {
			_width = 100;
		}
	}

	if (_height <= 0) {
		if (img) {
			img->getBoundingRect(&rect, 0, 0);
			_height = rect.bottom - rect.top;
		}
	}

	if (_text) {
		int textHeight = 0;
		BaseFont *font = _font ? _font : _gameRef->getSystemFont();

		if (font) {
			textHeight = font->getTextHeight((byte *)_text, _width);
		}

		if (textHeight > _height) {
			_height = textHeight;
		}
	}

	if (_height <= 0) {
		_height = 100;
	}

	if (_back) {
		_back->correctSize(&_width, &_height);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIButton::display(int offsetX, int offsetY) {
	if (!_visible) {
		return STATUS_OK;
	}

	UITiledImage *back = nullptr;
	BaseSprite *image = nullptr;
	BaseFont *font = 0;

	//RECT rect;
	//rect.setRect(OffsetX + _posX, OffsetY + _posY, OffsetX+_posX+_width, OffsetY+_posY+_height);
	//_hover = (!_disable && BasePlatform::ptInRect(&rect, _gameRef->_mousePos)!=FALSE);
	_hover = (!_disable && _gameRef->_activeObject == this && (_gameRef->_interactive || _gameRef->_state == GAME_SEMI_FROZEN));

	if ((_press && _hover && !_gameRef->_mouseLeftDown) ||
			(_oneTimePress && g_system->getMillis() - _oneTimePressTime >= 100)) {
		press();
	}


	if (_disable) {
		if (_backDisable) {
			back = _backDisable;
		}
		if (_imageDisable) {
			image = _imageDisable;
		}
		if (_text && _fontDisable) {
			font = _fontDisable;
		}
	} else if (_press || _oneTimePress || _stayPressed) {
		if (_backPress) {
			back = _backPress;
		}
		if (_imagePress) {
			image = _imagePress;
		}
		if (_text && _fontPress) {
			font = _fontPress;
		}
	} else if (_hover) {
		if (_backHover) {
			back = _backHover;
		}
		if (_imageHover) {
			image = _imageHover;
		}
		if (_text && _fontHover) {
			font = _fontHover;
		}
	} else if (_canFocus && isFocused()) {
		if (_backFocus) {
			back = _backFocus;
		}
		if (_imageFocus) {
			image = _imageFocus;
		}
		if (_text && _fontFocus) {
			font = _fontFocus;
		}
	}

	if (!back && _back) {
		back = _back;
	}
	if (!image && _image) {
		image = _image;
	}
	if (_text && !font) {
		if (_font) {
			font = _font;
		} else {
			font = _gameRef->getSystemFont();
		}
	}

	int imageX = offsetX + _posX;
	int imageY = offsetY + _posY;

	if (image && _centerImage) {
		Rect32 rc;
		image->getBoundingRect(&rc, 0, 0);
		imageX += (_width - (rc.right - rc.left)) / 2;
		imageY += (_height - (rc.bottom - rc.top)) / 2;
	}

	if (back) {
		back->display(offsetX + _posX, offsetY + _posY, _width, _height);
	}
	//if (image) image->Draw(ImageX +((_press||_oneTimePress)&&back?1:0), ImageY +((_press||_oneTimePress)&&back?1:0), nullptr);
	if (image) {
		image->draw(imageX + ((_press || _oneTimePress) && back ? 1 : 0), imageY + ((_press || _oneTimePress) && back ? 1 : 0), _pixelPerfect ? this : nullptr);
	}

	if (font && _text) {
		int text_offset = (_height - font->getTextHeight((byte *)_text, _width)) / 2;
		font->drawText((byte *)_text, offsetX + _posX + ((_press || _oneTimePress) ? 1 : 0), offsetY + _posY + text_offset + ((_press || _oneTimePress) ? 1 : 0), _width, _align);
	}

	if (!_pixelPerfect || !_image) {
		_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef, this, nullptr, offsetX + _posX, offsetY + _posY, _width, _height, 100, 100, false));
	}

	// reset unused sprites
	if (_image && _image != image) {
		_image->reset();
	}
	if (_imageDisable && _imageDisable != image) {
		_imageDisable->reset();
	}
	if (_imageFocus && _imageFocus != image) {
		_imageFocus->reset();
	}
	if (_imagePress && _imagePress != image) {
		_imagePress->reset();
	}
	if (_imageHover && _imageHover != image) {
		_imageHover->reset();
	}

	_press = _hover && _gameRef->_mouseLeftDown && _gameRef->_capturedObject == this;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void UIButton::press() {
	applyEvent("Press");
	if (_listenerObject) {
		_listenerObject->listen(_listenerParamObject, _listenerParamDWORD);
	}
	if (_parentNotify && _parent) {
		_parent->applyEvent(getName());
	}

	_oneTimePress = false;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIButton::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetDisabledFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetDisabledFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_fontDisable) {
			_gameRef->_fontStorage->removeFont(_fontDisable);
		}
		if (val->isNULL()) {
			_fontDisable = nullptr;
			stack->pushBool(true);
		} else {
			_fontDisable = _gameRef->_fontStorage->addFont(val->getString());
			stack->pushBool(_fontDisable != nullptr);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetHoverFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_fontHover) {
			_gameRef->_fontStorage->removeFont(_fontHover);
		}
		if (val->isNULL()) {
			_fontHover = nullptr;
			stack->pushBool(true);
		} else {
			_fontHover = _gameRef->_fontStorage->addFont(val->getString());
			stack->pushBool(_fontHover != nullptr);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPressedFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_fontPress) {
			_gameRef->_fontStorage->removeFont(_fontPress);
		}
		if (val->isNULL()) {
			_fontPress = nullptr;
			stack->pushBool(true);
		} else {
			_fontPress = _gameRef->_fontStorage->addFont(val->getString());
			stack->pushBool(_fontPress != nullptr);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetFocusedFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_fontFocus) {
			_gameRef->_fontStorage->removeFont(_fontFocus);
		}
		if (val->isNULL()) {
			_fontFocus = nullptr;
			stack->pushBool(true);
		} else {
			_fontFocus = _gameRef->_fontStorage->addFont(val->getString());
			stack->pushBool(_fontFocus != nullptr);
		}
		return STATUS_OK;
	}

#ifdef ENABLE_FOXTAIL
	//////////////////////////////////////////////////////////////////////////
	// [FoxTail] HeightToFit
	// Used to autofit widget's height to it's content
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HeightToFit") == 0) {
		stack->correctParams(0);

		correctSize();

		stack->pushNULL();
		return STATUS_OK;
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	// SetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetDisabledImage") == 0) {
		stack->correctParams(1);

		delete _imageDisable;
		_imageDisable = new BaseSprite(_gameRef);
		const char *filename = stack->pop()->getString();
		if (!_imageDisable || DID_FAIL(_imageDisable->loadFile(filename))) {
			delete _imageDisable;
			_imageDisable = nullptr;
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDisabledImage") == 0) {
		stack->correctParams(0);
		if (!_imageDisable || !_imageDisable->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_imageDisable->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetDisabledImageObject") == 0) {
		stack->correctParams(0);
		if (!_imageDisable) {
			stack->pushNULL();
		} else {
			stack->pushNative(_imageDisable, true);
		}

		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetHoverImage") == 0) {
		stack->correctParams(1);

		delete _imageHover;
		_imageHover = new BaseSprite(_gameRef);
		const char *filename = stack->pop()->getString();
		if (!_imageHover || DID_FAIL(_imageHover->loadFile(filename))) {
			delete _imageHover;
			_imageHover = nullptr;
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverImage") == 0) {
		stack->correctParams(0);
		if (!_imageHover || !_imageHover->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_imageHover->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetHoverImageObject") == 0) {
		stack->correctParams(0);
		if (!_imageHover) {
			stack->pushNULL();
		} else {
			stack->pushNative(_imageHover, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetPressedImage") == 0) {
		stack->correctParams(1);

		delete _imagePress;
		_imagePress = new BaseSprite(_gameRef);
		const char *filename = stack->pop()->getString();
		if (!_imagePress || DID_FAIL(_imagePress->loadFile(filename))) {
			delete _imagePress;
			_imagePress = nullptr;
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPressedImage") == 0) {
		stack->correctParams(0);
		if (!_imagePress || !_imagePress->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_imagePress->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetPressedImageObject") == 0) {
		stack->correctParams(0);
		if (!_imagePress) {
			stack->pushNULL();
		} else {
			stack->pushNative(_imagePress, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetFocusedImage") == 0) {
		stack->correctParams(1);

		delete _imageFocus;
		_imageFocus = new BaseSprite(_gameRef);
		const char *filename = stack->pop()->getString();
		if (!_imageFocus || DID_FAIL(_imageFocus->loadFile(filename))) {
			delete _imageFocus;
			_imageFocus = nullptr;
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFocusedImage") == 0) {
		stack->correctParams(0);
		if (!_imageFocus || !_imageFocus->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_imageFocus->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetFocusedImageObject") == 0) {
		stack->correctParams(0);
		if (!_imageFocus) {
			stack->pushNULL();
		} else {
			stack->pushNative(_imageFocus, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Press
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Press") == 0) {
		stack->correctParams(0);

		if (_visible && !_disable) {
			_oneTimePress = true;
			_oneTimePressTime = g_system->getMillis();
		}
		stack->pushNULL();

		return STATUS_OK;
	} else {
		return UIObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIButton::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("button");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TextAlign") {
		_scValue->setInt(_align);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Focusable") {
		_scValue->setBool(_canFocus);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Pressed") {
		_scValue->setBool(_stayPressed);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PixelPerfect") {
		_scValue->setBool(_pixelPerfect);
		return _scValue;
	} else {
		return UIObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIButton::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "TextAlign") == 0) {
		int i = value->getInt();
		if (i < 0 || i >= NUM_TEXT_ALIGN) {
			i = 0;
		}
		_align = (TTextAlign)i;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Focusable") == 0) {
		_canFocus = value->getBool();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pressed") == 0) {
		_stayPressed = value->getBool();
		return STATUS_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_pixelPerfect = value->getBool();
		return STATUS_OK;
	} else {
		return UIObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIButton::scToString() {
	return "[button]";
}


//////////////////////////////////////////////////////////////////////////
bool UIButton::persist(BasePersistenceManager *persistMgr) {

	UIObject::persist(persistMgr);

	persistMgr->transferSint32(TMEMBER_INT(_align));
	persistMgr->transferPtr(TMEMBER_PTR(_backDisable));
	persistMgr->transferPtr(TMEMBER_PTR(_backFocus));
	persistMgr->transferPtr(TMEMBER_PTR(_backHover));
	persistMgr->transferPtr(TMEMBER_PTR(_backPress));
	persistMgr->transferBool(TMEMBER(_centerImage));
	persistMgr->transferPtr(TMEMBER_PTR(_fontDisable));
	persistMgr->transferPtr(TMEMBER_PTR(_fontFocus));
	persistMgr->transferPtr(TMEMBER_PTR(_fontHover));
	persistMgr->transferPtr(TMEMBER_PTR(_fontPress));
	persistMgr->transferBool(TMEMBER(_hover));
	persistMgr->transferPtr(TMEMBER_PTR(_image));
	persistMgr->transferPtr(TMEMBER_PTR(_imageDisable));
	persistMgr->transferPtr(TMEMBER_PTR(_imageFocus));
	persistMgr->transferPtr(TMEMBER_PTR(_imageHover));
	persistMgr->transferPtr(TMEMBER_PTR(_imagePress));
	persistMgr->transferBool(TMEMBER(_pixelPerfect));
	persistMgr->transferBool(TMEMBER(_press));
	persistMgr->transferBool(TMEMBER(_stayPressed));

	if (!persistMgr->getIsSaving()) {
		_oneTimePress = false;
		_oneTimePressTime = 0;
	}

	return STATUS_OK;
}

void UIButton::setFontHover(BaseFont *font) {
	_fontHover = font;
}

BaseFont *UIButton::getFontHover() {
	return _fontHover;
}

void UIButton::setFontPress(BaseFont *font) {
	_fontPress = font;
}

void UIButton::setImageHover(BaseSprite *sprite) {
	_imageHover = sprite;
}

void UIButton::setImagePress(BaseSprite *sprite) {
	_imagePress = sprite;
}

void UIButton::setTextAlign(TTextAlign align) {
	_align = align;
}

} // End of namespace Wintermute
