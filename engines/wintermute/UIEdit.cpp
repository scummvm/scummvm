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

#include "engines/wintermute/UIEdit.h"
#include "engines/wintermute/UIObject.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/StringUtil.h"
#include "engines/wintermute/BActiveRect.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BKeyboardState.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/utils.h"
#include "common/util.h"
#include "common/keyboard.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIEdit, false)

//////////////////////////////////////////////////////////////////////////
CUIEdit::CUIEdit(CBGame *inGame): CUIObject(inGame) {
	_type = UI_EDIT;

	_fontSelected = NULL;

	_selStart = _selEnd = 10000;
	_scrollOffset = 0;

	_cursorChar = NULL;
	SetCursorChar("|");

#ifdef __WIN32__
	_cursorBlinkRate = GetCaretBlinkTime();
#else
	_cursorBlinkRate = 600;
#endif
	_frameWidth = 0;

	SetText("");

	_lastBlinkTime = 0;
	_cursorVisible = true;

	_maxLength = -1;

	_canFocus = true;
}


//////////////////////////////////////////////////////////////////////////
CUIEdit::~CUIEdit() {
	if (!_sharedFonts) {
		if (_fontSelected)   Game->_fontStorage->RemoveFont(_fontSelected);
	}

	delete[] _cursorChar;
	_cursorChar = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::LoadFile(char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIEdit::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing EDIT file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_SELECTED)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR_BLINK_RATE)
TOKEN_DEF(CURSOR)
TOKEN_DEF(FRAME_WIDTH)
TOKEN_DEF(NAME)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(MAX_LENGTH)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(EDIT)
TOKEN_DEF(CAPTION)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_SELECTED)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR_BLINK_RATE)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(FRAME_WIDTH)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(MAX_LENGTH)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(EDIT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_EDIT) {
			Game->LOG(0, "'EDIT' keyword expected.");
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

		case TOKEN_FONT_SELECTED:
			if (_fontSelected) Game->_fontStorage->RemoveFont(_fontSelected);
			_fontSelected = Game->_fontStorage->AddFont((char *)params);
			if (!_fontSelected) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			SetText((char *)params);
			Game->_stringTable->Expand(&_text);
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

		case TOKEN_MAX_LENGTH:
			parser.ScanStr((char *)params, "%d", &_maxLength);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
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

		case TOKEN_CURSOR_BLINK_RATE:
			parser.ScanStr((char *)params, "%d", &_cursorBlinkRate);
			break;

		case TOKEN_FRAME_WIDTH:
			parser.ScanStr((char *)params, "%d", &_frameWidth);
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
		Game->LOG(0, "Syntax error in EDIT definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading EDIT definition");
		return E_FAIL;
	}

	CorrectSize();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "EDIT\n");
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
	if (_fontSelected && _fontSelected->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_SELECTED=\"%s\"\n", _fontSelected->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_text)
		Buffer->PutTextIndent(Indent + 2, "TEXT=\"%s\"\n", _text);

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", _posX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", _posY);
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", _width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", _height);
	Buffer->PutTextIndent(Indent + 2, "MAX_LENGTH=%d\n", _maxLength);
	Buffer->PutTextIndent(Indent + 2, "CURSOR_BLINK_RATE=%d\n", _cursorBlinkRate);
	Buffer->PutTextIndent(Indent + 2, "FRAME_WIDTH=%d\n", _frameWidth);

	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

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
HRESULT CUIEdit::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSelectedFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSelectedFont") == 0) {
		Stack->CorrectParams(1);

		if (_fontSelected) Game->_fontStorage->RemoveFont(_fontSelected);
		_fontSelected = Game->_fontStorage->AddFont(Stack->Pop()->GetString());
		Stack->PushBool(_fontSelected != NULL);

		return S_OK;
	}

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEdit::ScGetProperty(char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("editor");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SelStart") == 0) {
		_scValue->SetInt(_selStart);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SelEnd") == 0) {
		_scValue->SetInt(_selEnd);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorBlinkRate") == 0) {
		_scValue->SetInt(_cursorBlinkRate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorChar") == 0) {
		_scValue->SetString(_cursorChar);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FrameWidth") == 0) {
		_scValue->SetInt(_frameWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxLength") == 0) {
		_scValue->SetInt(_maxLength);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Text") == 0) {
		if (Game->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::AnsiToWide(_text);
			_scValue->SetString(StringUtil::WideToUtf8(wstr).c_str());
		} else {
			_scValue->SetString(_text);
		}
		return _scValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SelStart") == 0) {
		_selStart = Value->GetInt();
		_selStart = MAX(_selStart, 0);
		_selStart = MIN((size_t)_selStart, strlen(_text));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SelEnd") == 0) {
		_selEnd = Value->GetInt();
		_selEnd = MAX(_selEnd, 0);
		_selEnd = MIN((size_t)_selEnd, strlen(_text));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorBlinkRate") == 0) {
		_cursorBlinkRate = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CursorChar") == 0) {
		SetCursorChar(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FrameWidth") == 0) {
		_frameWidth = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MaxLength") == 0) {
		_maxLength = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Text") == 0) {
		if (Game->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::Utf8ToWide(Value->GetString());
			SetText(StringUtil::WideToAnsi(wstr).c_str());
		} else {
			SetText(Value->GetString());
		}
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CUIEdit::ScToString() {
	return "[edit]";
}


//////////////////////////////////////////////////////////////////////////
void CUIEdit::SetCursorChar(char *Char) {
	if (!Char) return;
	delete[] _cursorChar;
	_cursorChar = new char [strlen(Char) + 1];
	if (_cursorChar) strcpy(_cursorChar, Char);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::Display(int OffsetX, int OffsetY) {
	if (!_visible) return S_OK;


	// hack!
	TTextEncoding OrigEncoding = Game->_textEncoding;
	Game->_textEncoding = TEXT_ANSI;

	if (_back) _back->Display(OffsetX + _posX, OffsetY + _posY, _width, _height);
	if (_image) _image->Draw(OffsetX + _posX, OffsetY + _posY, NULL);

	// prepare fonts
	CBFont *font;
	CBFont *sfont;

	if (_font) font = _font;
	else font = Game->_systemFont;

	if (_fontSelected) sfont = _fontSelected;
	else sfont = font;

	bool focused = IsFocused();

	_selStart = MAX(_selStart, 0);
	_selEnd   = MAX(_selEnd, 0);

	_selStart = MIN((size_t)_selStart, strlen(_text));
	_selEnd   = MIN((size_t)_selEnd,   strlen(_text));

	//int CursorWidth = font->GetCharWidth(_cursorChar[0]);
	int CursorWidth = font->GetTextWidth((byte  *)_cursorChar);

	int s1, s2;
	bool CurFirst;
	// modify scroll offset
	if (_selStart >= _selEnd) {
		while (font->GetTextWidth((byte  *)_text + _scrollOffset, std::max(0, _selEnd - _scrollOffset)) > _width - CursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= strlen(_text)) break;
		}

		_scrollOffset = std::min(_scrollOffset, _selEnd);

		s1 = _selEnd;
		s2 = _selStart;
		CurFirst = true;
	} else {
		while (font->GetTextWidth((byte  *)_text + _scrollOffset, std::max(0, _selStart - _scrollOffset)) +
		        sfont->GetTextWidth((byte  *)(_text + std::max(_scrollOffset, _selStart)), _selEnd - std::max(_scrollOffset, _selStart))

		        > _width - CursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= strlen(_text)) break;
		}

		_scrollOffset = std::min(_scrollOffset, _selEnd);

		s1 = _selStart;
		s2 = _selEnd;
		CurFirst = false;
	}


	int AlignOffset = 0;

	for (int Count = 0; Count < 2; Count++) {
		// draw text
		int xxx, yyy, width, height;

		xxx = _posX + _frameWidth + OffsetX;
		yyy = _posY + _frameWidth + OffsetY;

		width = _posX + _width + OffsetX - _frameWidth;
		height = std::max(font->GetLetterHeight(), sfont->GetLetterHeight());

		if (Game->_textRTL) xxx += AlignOffset;

		TTextAlign Align = TAL_LEFT;


		// unselected 1
		if (s1 > _scrollOffset) {
			if (Count) font->DrawText((byte  *)_text + _scrollOffset, xxx, yyy, width - xxx, Align, height, s1 - _scrollOffset);
			xxx += font->GetTextWidth((byte  *)_text + _scrollOffset, s1 - _scrollOffset);
			AlignOffset += font->GetTextWidth((byte  *)_text + _scrollOffset, s1 - _scrollOffset);
		}

		// cursor
		if (focused && CurFirst) {
			if (Count) {
				if (CBPlatform::GetTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::GetTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->DrawText((byte  *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += CursorWidth;
			AlignOffset += CursorWidth;
		}

		// selected
		int s3 = std::max(s1, _scrollOffset);

		if (s2 - s3 > 0) {
			if (Count) sfont->DrawText((byte  *)_text + s3, xxx, yyy, width - xxx, Align, height, s2 - s3);
			xxx += sfont->GetTextWidth((byte  *)_text + s3, s2 - s3);
			AlignOffset += sfont->GetTextWidth((byte  *)_text + s3, s2 - s3);
		}

		// cursor
		if (focused && !CurFirst) {
			if (Count) {
				if (CBPlatform::GetTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::GetTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->DrawText((byte  *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += CursorWidth;
			AlignOffset += CursorWidth;
		}

		// unselected 2
		if (Count) font->DrawText((byte  *)_text + s2, xxx, yyy, width - xxx, Align, height);
		AlignOffset += font->GetTextWidth((byte  *)_text + s2);

		AlignOffset = (_width - 2 * _frameWidth) - AlignOffset;
		if (AlignOffset < 0) AlignOffset = 0;
	}


	Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, OffsetX + _posX, OffsetY + _posY, _width, _height, 100, 100, false));


	Game->_textEncoding = OrigEncoding;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::HandleKeypress(Common::Event *event) {
	bool Handled = false;

	if (event->type == Common::EVENT_KEYDOWN) {
		switch (event->kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
		case Common::KEYCODE_RETURN:
			return false;

			// ctrl+A
		case Common::KEYCODE_a:
			if (CBKeyboardState::IsControlDown()) {
				_selStart = 0;
				_selEnd = strlen(_text);
				Handled = true;
			}
			break;

		case Common::KEYCODE_BACKSPACE:
			if (_selStart == _selEnd) {
				if (Game->_textRTL) DeleteChars(_selStart, _selStart + 1);
				else DeleteChars(_selStart - 1, _selStart);
			} else DeleteChars(_selStart, _selEnd);
			if (_selEnd >= _selStart) _selEnd -= std::max(1, _selEnd - _selStart);
			_selStart = _selEnd;

			Handled = true;
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_UP:
			_selEnd--;
			if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			Handled = true;
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_DOWN:
			_selEnd++;
			if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			Handled = true;
			break;

		case Common::KEYCODE_HOME:
			if (Game->_textRTL) {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = 0;
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			}
			Handled = true;
			break;

		case Common::KEYCODE_END:
			if (Game->_textRTL) {
				_selEnd = 0;
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::IsShiftDown()) _selStart = _selEnd;
			}
			Handled = true;
			break;

		case Common::KEYCODE_DELETE:
			if (_selStart == _selEnd) {
				if (Game->_textRTL) {
					DeleteChars(_selStart - 1, _selStart);
					_selEnd--;
					if (_selEnd < 0) _selEnd = 0;
				} else DeleteChars(_selStart, _selStart + 1);
			} else DeleteChars(_selStart, _selEnd);
			if (_selEnd > _selStart) _selEnd -= (_selEnd - _selStart);

			_selStart = _selEnd;
			Handled = true;
			break;
		default:
			break;
		}
		return Handled;
	}
#if 0
	else if (event->type == SDL_TEXTINPUT) {
		if (_selStart != _selEnd) DeleteChars(_selStart, _selEnd);

		WideString wstr = StringUtil::Utf8ToWide(event->text.text);
		_selEnd += InsertChars(_selEnd, (byte  *)StringUtil::WideToAnsi(wstr).c_str(), 1);

		if (Game->_textRTL) _selEnd = _selStart;
		else _selStart = _selEnd;

		return true;
	}
#endif
	return false;
}



//////////////////////////////////////////////////////////////////////////
int CUIEdit::DeleteChars(int Start, int End) {
	if (Start > End) CBUtils::Swap(&Start, &End);

	Start = MAX(Start, (int)0);
	End = MIN((size_t)End, strlen(_text));

	char *str = new char[strlen(_text) - (End - Start) + 1];
	if (str) {
		if (Start > 0) memcpy(str, _text, Start);
		memcpy(str + std::max(0, Start), _text + End, strlen(_text) - End + 1);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->ApplyEvent(_name);

	return End - Start;
}


//////////////////////////////////////////////////////////////////////////
int CUIEdit::InsertChars(int Pos, byte *Chars, int Num) {
	if (strlen(_text) + Num > _maxLength) {
		Num -= (strlen(_text) + Num - _maxLength);
	}

	Pos = MAX(Pos, (int)0);
	Pos = MIN((size_t)Pos, strlen(_text));

	char *str = new char[strlen(_text) + Num + 1];
	if (str) {
		if (Pos > 0) memcpy(str, _text, Pos);
		memcpy(str + Pos + Num, _text + Pos, strlen(_text) - Pos + 1);

		memcpy(str + Pos, Chars, Num);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->ApplyEvent(_name);

	return Num;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIEdit::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_cursorBlinkRate));
	PersistMgr->Transfer(TMEMBER(_cursorChar));
	PersistMgr->Transfer(TMEMBER(_fontSelected));
	PersistMgr->Transfer(TMEMBER(_frameWidth));
	PersistMgr->Transfer(TMEMBER(_maxLength));
	PersistMgr->Transfer(TMEMBER(_scrollOffset));
	PersistMgr->Transfer(TMEMBER(_selEnd));
	PersistMgr->Transfer(TMEMBER(_selStart));

	if (!PersistMgr->_saving) {
		_cursorVisible = false;
		_lastBlinkTime = 0;
	}

	return S_OK;
}

} // end of namespace WinterMute
