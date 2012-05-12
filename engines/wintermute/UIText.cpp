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
#include "engines/wintermute/UIText.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIText, false)

//////////////////////////////////////////////////////////////////////////
CUIText::CUIText(CBGame *inGame): CUIObject(inGame) {
	_textAlign = TAL_LEFT;
	_verticalAlign = VAL_CENTER;
	_type = UI_STATIC;
	_canFocus = false;
}


//////////////////////////////////////////////////////////////////////////
CUIText::~CUIText() {

}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::Display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;


	CBFont *font = _font;
	if (!font) font = Game->_systemFont;

	if (_back) _back->Display(OffsetX + _posX, OffsetY + _posY, _width, _height);
	if (_image) _image->Draw(OffsetX + _posX, OffsetY + _posY, NULL);

	if (font && _text) {
		int text_offset;
		switch (_verticalAlign) {
		case VAL_TOP:
			text_offset = 0;
			break;
		case VAL_BOTTOM:
			text_offset = _height - font->GetTextHeight((byte *)_text, _width);
			break;
		default:
			text_offset = (_height - font->GetTextHeight((byte *)_text, _width)) / 2;
		}
		font->DrawText((byte *)_text, OffsetX + _posX, OffsetY + _posY + text_offset, _width, _textAlign, _height);
	}

	//Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, OffsetX + _posX, OffsetY + _posY, _width, _height, 100, 100, false));

	return S_OK;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIText::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing STATIC file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(STATIC)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(VERTICAL_ALIGN)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(STATIC)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(VERTICAL_ALIGN)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_STATIC) {
			Game->LOG(0, "'STATIC' keyword expected.");
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

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSprite(Game);
			if (!_image || FAILED(_image->LoadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) Game->_fontStorage->RemoveFont(_font);
			_font = Game->_fontStorage->AddFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			SetText((char *)params);
			Game->_stringTable->Expand(&_text);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) _textAlign = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) _textAlign = TAL_RIGHT;
			else _textAlign = TAL_CENTER;
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp((char *)params, "top") == 0) _verticalAlign = VAL_TOP;
			else if (scumm_stricmp((char *)params, "bottom") == 0) _verticalAlign = VAL_BOTTOM;
			else _verticalAlign = VAL_CENTER;
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

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in STATIC definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading STATIC definition");
		return E_FAIL;
	}

	CorrectSize();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "STATIC\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_back && _back->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK=\"%s\"\n", _back->_filename);

	if (_image && _image->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", _image->_filename);

	if (_font && _font->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	if (_text)
		Buffer->PutTextIndent(Indent + 2, "TEXT=\"%s\"\n", _text);

	switch (_textAlign) {
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

	switch (_verticalAlign) {
	case VAL_TOP:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
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
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SizeToFit
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SizeToFit") == 0) {
		Stack->CorrectParams(0);
		SizeToFit();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HeightToFit
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HeightToFit") == 0) {
		Stack->CorrectParams(0);
		if (_font && _text) _height = _font->GetTextHeight((byte *)_text, _width);
		Stack->PushNULL();
		return S_OK;
	}

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIText::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("static");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextAlign") == 0) {
		_scValue->SetInt(_textAlign);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// VerticalAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VerticalAlign") == 0) {
		_scValue->SetInt(_verticalAlign);
		return _scValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "TextAlign") == 0) {
		int i = Value->GetInt();
		if (i < 0 || i >= NUM_TEXT_ALIGN) i = 0;
		_textAlign = (TTextAlign)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// VerticalAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "VerticalAlign") == 0) {
		int i = Value->GetInt();
		if (i < 0 || i >= NUM_VERTICAL_ALIGN) i = 0;
		_verticalAlign = (TVerticalAlign)i;
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIText::ScToString() {
	return "[static]";
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER_INT(_textAlign));
	PersistMgr->Transfer(TMEMBER_INT(_verticalAlign));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIText::SizeToFit() {
	if (_font && _text) {
		_width = _font->GetTextWidth((byte *)_text);
		_height = _font->GetTextHeight((byte *)_text, _width);
	}
	return S_OK;
}

} // end of namespace WinterMute
