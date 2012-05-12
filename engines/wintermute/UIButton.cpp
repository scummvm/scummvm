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
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/UIButton.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BActiveRect.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIButton, false)

//////////////////////////////////////////////////////////////////////////
CUIButton::CUIButton(CBGame *inGame): CUIObject(inGame) {
	_backPress = _backHover = _backDisable = _backFocus = NULL;

	_fontHover = _fontPress = _fontDisable = _fontFocus = NULL;

	_imageDisable = _imagePress = _imageHover = _imageFocus = NULL;

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
CUIButton::~CUIButton() {
	if (_backPress) delete _backPress;
	if (_backHover) delete _backHover;
	if (_backDisable) delete _backDisable;
	if (_backFocus) delete _backFocus;

	if (!_sharedFonts) {
		if (_fontHover)   Game->_fontStorage->RemoveFont(_fontHover);
		if (_fontPress)   Game->_fontStorage->RemoveFont(_fontPress);
		if (_fontDisable) Game->_fontStorage->RemoveFont(_fontDisable);
		if (_fontFocus)   Game->_fontStorage->RemoveFont(_fontFocus);
	}

	if (!_sharedImages) {
		if (_imageHover)   delete _imageHover;
		if (_imagePress)   delete _imagePress;
		if (_imageDisable) delete _imageDisable;
		if (_imageFocus)   delete _imageFocus;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIButton::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing BUTTON file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CUIButton::LoadBuffer(byte  *Buffer, bool Complete) {
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

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_BUTTON) {
			Game->LOG(0, "'BUTTON' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while (cmd > 0 && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_BACK:
			delete _back;
			_back = new CUITiledImage(Game);
			if (!_back || FAILED(_back->LoadFile((char *)params))) {
				delete _back;
				_back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_HOVER:
			delete _backHover;
			_backHover = new CUITiledImage(Game);
			if (!_backHover || FAILED(_backHover->LoadFile((char *)params))) {
				delete _backHover;
				_backHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_PRESS:
			delete _backPress;
			_backPress = new CUITiledImage(Game);
			if (!_backPress || FAILED(_backPress->LoadFile((char *)params))) {
				delete _backPress;
				_backPress = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_DISABLE:
			delete _backDisable;
			_backDisable = new CUITiledImage(Game);
			if (!_backDisable || FAILED(_backDisable->LoadFile((char *)params))) {
				delete _backDisable;
				_backDisable = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_FOCUS:
			delete _backFocus;
			_backFocus = new CUITiledImage(Game);
			if (!_backFocus || FAILED(_backFocus->LoadFile((char *)params))) {
				delete _backFocus;
				_backFocus = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSprite(Game);
			if (!_image || FAILED(_image->LoadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_HOVER:
			delete _imageHover;
			_imageHover = new CBSprite(Game);
			if (!_imageHover || FAILED(_imageHover->LoadFile((char *)params))) {
				delete _imageHover;
				_imageHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_PRESS:
			delete _imagePress;
			_imagePress = new CBSprite(Game);
			if (!_imagePress || FAILED(_imagePress->LoadFile((char *)params))) {
				delete _imagePress;
				_imagePress = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_DISABLE:
			delete _imageDisable;
			_imageDisable = new CBSprite(Game);
			if (!_imageDisable || FAILED(_imageDisable->LoadFile((char *)params))) {
				delete _imageDisable;
				_imageDisable = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_FOCUS:
			delete _imageFocus;
			_imageFocus = new CBSprite(Game);
			if (!_imageFocus || FAILED(_imageFocus->LoadFile((char *)params))) {
				delete _imageFocus;
				_imageFocus = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) Game->_fontStorage->RemoveFont(_font);
			_font = Game->_fontStorage->AddFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_HOVER:
			if (_fontHover) Game->_fontStorage->RemoveFont(_fontHover);
			_fontHover = Game->_fontStorage->AddFont((char *)params);
			if (!_fontHover) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_PRESS:
			if (_fontPress) Game->_fontStorage->RemoveFont(_fontPress);
			_fontPress = Game->_fontStorage->AddFont((char *)params);
			if (!_fontPress) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_DISABLE:
			if (_fontDisable) Game->_fontStorage->RemoveFont(_fontDisable);
			_fontDisable = Game->_fontStorage->AddFont((char *)params);
			if (!_fontDisable) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_FOCUS:
			if (_fontFocus) Game->_fontStorage->RemoveFont(_fontFocus);
			_fontFocus = Game->_fontStorage->AddFont((char *)params);
			if (!_fontFocus) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			SetText((char *)params);
			Game->_stringTable->Expand(&_text);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) _align = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) _align = TAL_RIGHT;
			else _align = TAL_CENTER;
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_WIDTH:
			parser.ScanStr((char *)params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_height);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->LoadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.ScanStr((char *)params, "%b", &_parentNotify);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_FOCUSABLE:
			parser.ScanStr((char *)params, "%b", &_canFocus);
			break;

		case TOKEN_CENTER_IMAGE:
			parser.ScanStr((char *)params, "%b", &_centerImage);
			break;

		case TOKEN_PRESSED:
			parser.ScanStr((char *)params, "%b", &_stayPressed);
			break;

		case TOKEN_PIXEL_PERFECT:
			parser.ScanStr((char *)params, "%b", &_pixelPerfect);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in BUTTON definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading BUTTON definition");
		return E_FAIL;
	}

	CorrectSize();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "BUTTON\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_back && _back->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK=\"%s\"\n", _back->_filename);
	if (_backHover && _backHover->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_HOVER=\"%s\"\n", _backHover->_filename);
	if (_backPress && _backPress->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_PRESS=\"%s\"\n", _backPress->_filename);
	if (_backDisable && _backDisable->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_DISABLE=\"%s\"\n", _backDisable->_filename);
	if (_backFocus && _backFocus->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_FOCUS=\"%s\"\n", _backFocus->_filename);

	if (_image && _image->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", _image->_filename);
	if (_imageHover && _imageHover->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_HOVER=\"%s\"\n", _imageHover->_filename);
	if (_imagePress && _imagePress->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_PRESS=\"%s\"\n", _imagePress->_filename);
	if (_imageDisable && _imageDisable->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_DISABLE=\"%s\"\n", _imageDisable->_filename);
	if (_imageFocus && _imageFocus->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_FOCUS=\"%s\"\n", _imageFocus->_filename);

	if (_font && _font->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontHover && _fontHover->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->_filename);
	if (_fontPress && _fontPress->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_PRESS=\"%s\"\n", _fontPress->_filename);
	if (_fontDisable && _fontDisable->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_DISABLE=\"%s\"\n", _fontDisable->_filename);
	if (_fontFocus && _fontFocus->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_FOCUS=\"%s\"\n", _fontFocus->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);


	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_text)
		Buffer->PutTextIndent(Indent + 2, "TEXT=\"%s\"\n", _text);

	switch (_align) {
	case TAL_LEFT:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", _posX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", _posY);
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", _width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", _height);


	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "FOCUSABLE=%s\n", _canFocus ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CENTER_IMAGE=%s\n", _centerImage ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PRESSED=%s\n", _stayPressed ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PIXEL_PERFECT=%s\n", _pixelPerfect ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CUIButton::CorrectSize() {
	RECT rect;

	CBSprite *img = NULL;
	if (_image) img = _image;
	else if (_imageDisable) img = _imageDisable;
	else if (_imageHover) img = _imageHover;
	else if (_imagePress) img = _imagePress;
	else if (_imageFocus) img = _imageFocus;

	if (_width <= 0) {
		if (img) {
			img->GetBoundingRect(&rect, 0, 0);
			_width = rect.right - rect.left;
		} else _width = 100;
	}

	if (_height <= 0) {
		if (img) {
			img->GetBoundingRect(&rect, 0, 0);
			_height = rect.bottom - rect.top;
		}
	}

	if (_text) {
		int text_height;
		if (_font) text_height = _font->GetTextHeight((byte *)_text, _width);
		else text_height = Game->_systemFont->GetTextHeight((byte *)_text, _width);

		if (text_height > _height) _height = text_height;
	}

	if (_height <= 0) _height = 100;

	if (_back) _back->CorrectSize(&_width, &_height);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::Display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;

	CUITiledImage *back = NULL;
	CBSprite *image = NULL;
	CBFont *font = 0;

	//RECT rect;
	//CBPlatform::SetRect(&rect, OffsetX + _posX, OffsetY + _posY, OffsetX+_posX+_width, OffsetY+_posY+_height);
	//_hover = (!_disable && CBPlatform::PtInRect(&rect, Game->_mousePos)!=FALSE);
	_hover = (!_disable && Game->_activeObject == this && (Game->_interactive || Game->_state == GAME_SEMI_FROZEN));

	if ((_press && _hover && !Game->_mouseLeftDown) ||
	        _oneTimePress && CBPlatform::GetTime() - _oneTimePressTime >= 100) Press();


	if (_disable) {
		if (_backDisable) back = _backDisable;
		if (_imageDisable) image = _imageDisable;
		if (_text && _fontDisable) font = _fontDisable;
	} else if (_press || _oneTimePress || _stayPressed) {
		if (_backPress) back = _backPress;
		if (_imagePress) image = _imagePress;
		if (_text && _fontPress) font = _fontPress;
	} else if (_hover) {
		if (_backHover) back = _backHover;
		if (_imageHover) image = _imageHover;
		if (_text && _fontHover) font = _fontHover;
	} else if (_canFocus && IsFocused()) {
		if (_backFocus) back = _backFocus;
		if (_imageFocus) image = _imageFocus;
		if (_text && _fontFocus) font = _fontFocus;
	}

	if (!back && _back) back = _back;
	if (!image && _image) image = _image;
	if (_text && !font) {
		if (_font) font = _font;
		else font = Game->_systemFont;
	}

	int ImageX = OffsetX + _posX;
	int ImageY = OffsetY + _posY;

	if (image && _centerImage) {
		RECT rc;
		image->GetBoundingRect(&rc, 0, 0);
		ImageX += (_width - (rc.right - rc.left)) / 2;
		ImageY += (_height - (rc.bottom - rc.top)) / 2;
	}

	if (back) back->Display(OffsetX + _posX, OffsetY + _posY, _width, _height);
	//if(image) image->Draw(ImageX +((_press||_oneTimePress)&&back?1:0), ImageY +((_press||_oneTimePress)&&back?1:0), NULL);
	if (image) image->Draw(ImageX + ((_press || _oneTimePress) && back ? 1 : 0), ImageY + ((_press || _oneTimePress) && back ? 1 : 0), _pixelPerfect ? this : NULL);

	if (font && _text) {
		int text_offset = (_height - font->GetTextHeight((byte *)_text, _width)) / 2;
		font->DrawText((byte *)_text, OffsetX + _posX + ((_press || _oneTimePress) ? 1 : 0), OffsetY + _posY + text_offset + ((_press || _oneTimePress) ? 1 : 0), _width, _align);
	}

	if (!_pixelPerfect || !_image) Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, OffsetX + _posX, OffsetY + _posY, _width, _height, 100, 100, false));

	// reset unused sprites
	if (_image && _image != image) _image->Reset();
	if (_imageDisable && _imageDisable != image) _imageDisable->Reset();
	if (_imageFocus && _imageFocus != image) _imageFocus->Reset();
	if (_imagePress && _imagePress != image) _imagePress->Reset();
	if (_imageHover && _imageHover != image) _imageHover->Reset();

	_press = _hover && Game->_mouseLeftDown && Game->_capturedObject == this;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIButton::Press() {
	ApplyEvent("Press");
	if (_listenerObject) _listenerObject->Listen(_listenerParamObject, _listenerParamDWORD);
	if (_parentNotify && _parent) _parent->ApplyEvent(_name);

	_oneTimePress = false;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetDisabledFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetDisabledFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (_fontDisable) Game->_fontStorage->RemoveFont(_fontDisable);
		if (Val->IsNULL()) {
			_fontDisable = NULL;
			Stack->PushBool(true);
		} else {
			_fontDisable = Game->_fontStorage->AddFont(Val->GetString());
			Stack->PushBool(_fontDisable != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHoverFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (_fontHover) Game->_fontStorage->RemoveFont(_fontHover);
		if (Val->IsNULL()) {
			_fontHover = NULL;
			Stack->PushBool(true);
		} else {
			_fontHover = Game->_fontStorage->AddFont(Val->GetString());
			Stack->PushBool(_fontHover != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPressedFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (_fontPress) Game->_fontStorage->RemoveFont(_fontPress);
		if (Val->IsNULL()) {
			_fontPress = NULL;
			Stack->PushBool(true);
		} else {
			_fontPress = Game->_fontStorage->AddFont(Val->GetString());
			Stack->PushBool(_fontPress != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFocusedFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (_fontFocus) Game->_fontStorage->RemoveFont(_fontFocus);
		if (Val->IsNULL()) {
			_fontFocus = NULL;
			Stack->PushBool(true);
		} else {
			_fontFocus = Game->_fontStorage->AddFont(Val->GetString());
			Stack->PushBool(_fontFocus != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDisabledImage") == 0) {
		Stack->CorrectParams(1);

		delete _imageDisable;
		_imageDisable = new CBSprite(Game);
		const char *Filename = Stack->Pop()->GetString();
		if (!_imageDisable || FAILED(_imageDisable->LoadFile(Filename))) {
			delete _imageDisable;
			_imageDisable = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDisabledImage") == 0) {
		Stack->CorrectParams(0);
		if (!_imageDisable || !_imageDisable->_filename) Stack->PushNULL();
		else Stack->PushString(_imageDisable->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDisabledImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_imageDisable) Stack->PushNULL();
		else Stack->PushNative(_imageDisable, true);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHoverImage") == 0) {
		Stack->CorrectParams(1);

		delete _imageHover;
		_imageHover = new CBSprite(Game);
		const char *Filename = Stack->Pop()->GetString();
		if (!_imageHover || FAILED(_imageHover->LoadFile(Filename))) {
			delete _imageHover;
			_imageHover = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverImage") == 0) {
		Stack->CorrectParams(0);
		if (!_imageHover || !_imageHover->_filename) Stack->PushNULL();
		else Stack->PushString(_imageHover->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_imageHover) Stack->PushNULL();
		else Stack->PushNative(_imageHover, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPressedImage") == 0) {
		Stack->CorrectParams(1);

		delete _imagePress;
		_imagePress = new CBSprite(Game);
		const char *Filename = Stack->Pop()->GetString();
		if (!_imagePress || FAILED(_imagePress->LoadFile(Filename))) {
			delete _imagePress;
			_imagePress = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetPressedImage") == 0) {
		Stack->CorrectParams(0);
		if (!_imagePress || !_imagePress->_filename) Stack->PushNULL();
		else Stack->PushString(_imagePress->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetPressedImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_imagePress) Stack->PushNULL();
		else Stack->PushNative(_imagePress, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFocusedImage") == 0) {
		Stack->CorrectParams(1);

		delete _imageFocus;
		_imageFocus = new CBSprite(Game);
		const char *Filename = Stack->Pop()->GetString();
		if (!_imageFocus || FAILED(_imageFocus->LoadFile(Filename))) {
			delete _imageFocus;
			_imageFocus = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFocusedImage") == 0) {
		Stack->CorrectParams(0);
		if (!_imageFocus || !_imageFocus->_filename) Stack->PushNULL();
		else Stack->PushString(_imageFocus->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFocusedImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_imageFocus) Stack->PushNULL();
		else Stack->PushNative(_imageFocus, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Press
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Press") == 0) {
		Stack->CorrectParams(0);

		if (_visible && !_disable) {
			_oneTimePress = true;
			_oneTimePressTime = CBPlatform::GetTime();
		}
		Stack->PushNULL();

		return S_OK;
	}


	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIButton::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("button");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextAlign") == 0) {
		_scValue->SetInt(_align);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Focusable") == 0) {
		_scValue->SetBool(_canFocus);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pressed") == 0) {
		_scValue->SetBool(_stayPressed);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		_scValue->SetBool(_pixelPerfect);
		return _scValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "TextAlign") == 0) {
		int i = Value->GetInt();
		if (i < 0 || i >= NUM_TEXT_ALIGN) i = 0;
		_align = (TTextAlign)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Focusable") == 0) {
		_canFocus = Value->GetBool();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pressed") == 0) {
		_stayPressed = Value->GetBool();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		_pixelPerfect = Value->GetBool();
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIButton::ScToString() {
	return "[button]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER_INT(_align));
	PersistMgr->Transfer(TMEMBER(_backDisable));
	PersistMgr->Transfer(TMEMBER(_backFocus));
	PersistMgr->Transfer(TMEMBER(_backHover));
	PersistMgr->Transfer(TMEMBER(_backPress));
	PersistMgr->Transfer(TMEMBER(_centerImage));
	PersistMgr->Transfer(TMEMBER(_fontDisable));
	PersistMgr->Transfer(TMEMBER(_fontFocus));
	PersistMgr->Transfer(TMEMBER(_fontHover));
	PersistMgr->Transfer(TMEMBER(_fontPress));
	PersistMgr->Transfer(TMEMBER(_hover));
	PersistMgr->Transfer(TMEMBER(_image));
	PersistMgr->Transfer(TMEMBER(_imageDisable));
	PersistMgr->Transfer(TMEMBER(_imageFocus));
	PersistMgr->Transfer(TMEMBER(_imageHover));
	PersistMgr->Transfer(TMEMBER(_imagePress));
	PersistMgr->Transfer(TMEMBER(_pixelPerfect));
	PersistMgr->Transfer(TMEMBER(_press));
	PersistMgr->Transfer(TMEMBER(_stayPressed));

	if (!PersistMgr->_saving) {
		_oneTimePress = false;
		_oneTimePressTime = 0;
	}

	return S_OK;
}

} // end of namespace WinterMute
