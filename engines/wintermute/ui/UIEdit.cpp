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

#include "engines/wintermute/ui/UIEdit.h"
#include "engines/wintermute/ui/UIObject.h"
#include "engines/wintermute/ui/UITiledImage.h"
#include "engines/wintermute/utils/StringUtil.h"
#include "engines/wintermute/base/BActiveRect.h"
#include "engines/wintermute/base/BFileManager.h"
#include "engines/wintermute/base/font/BFont.h"
#include "engines/wintermute/base/font/BFontStorage.h"
#include "engines/wintermute/base/BKeyboardState.h"
#include "engines/wintermute/base/BDynBuffer.h"
#include "engines/wintermute/base/BParser.h"
#include "engines/wintermute/base/BSprite.h"
#include "engines/wintermute/base/BStringTable.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/scriptables/ScValue.h"
#include "engines/wintermute/base/scriptables/ScStack.h"
#include "engines/wintermute/base/scriptables/ScScript.h"
#include "engines/wintermute/utils/utils.h"
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
	setCursorChar("|");

	_cursorBlinkRate = 600;

	_frameWidth = 0;

	setText("");

	_lastBlinkTime = 0;
	_cursorVisible = true;

	_maxLength = -1;

	_canFocus = true;
}


//////////////////////////////////////////////////////////////////////////
CUIEdit::~CUIEdit() {
	if (!_sharedFonts) {
		if (_fontSelected)   _gameRef->_fontStorage->removeFont(_fontSelected);
	}

	delete[] _cursorChar;
	_cursorChar = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::loadFile(const char *filename) {
	byte *buffer = _gameRef->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "CUIEdit::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) _gameRef->LOG(0, "Error parsing EDIT file '%s'", filename);

	delete [] buffer;

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
bool CUIEdit::loadBuffer(byte *buffer, bool complete) {
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
	CBParser parser(_gameRef);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_EDIT) {
			_gameRef->LOG(0, "'EDIT' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_BACK:
			delete _back;
			_back = new CUITiledImage(_gameRef);
			if (!_back || DID_FAIL(_back->loadFile((char *)params))) {
				delete _back;
				_back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSprite(_gameRef);
			if (!_image || DID_FAIL(_image->loadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) _gameRef->_fontStorage->removeFont(_font);
			_font = _gameRef->_fontStorage->addFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_SELECTED:
			if (_fontSelected) _gameRef->_fontStorage->removeFont(_fontSelected);
			_fontSelected = _gameRef->_fontStorage->addFont((char *)params);
			if (!_fontSelected) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			setText((char *)params);
			_gameRef->_stringTable->expand(&_text);
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_WIDTH:
			parser.scanStr((char *)params, "%d", &_width);
			break;

		case TOKEN_HEIGHT:
			parser.scanStr((char *)params, "%d", &_height);
			break;

		case TOKEN_MAX_LENGTH:
			parser.scanStr((char *)params, "%d", &_maxLength);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(_gameRef);
			if (!_cursor || DID_FAIL(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_CURSOR_BLINK_RATE:
			parser.scanStr((char *)params, "%d", &_cursorBlinkRate);
			break;

		case TOKEN_FRAME_WIDTH:
			parser.scanStr((char *)params, "%d", &_frameWidth);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.scanStr((char *)params, "%b", &_parentNotify);
			break;

		case TOKEN_DISABLED:
			parser.scanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in EDIT definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading EDIT definition");
		return STATUS_FAILED;
	}

	correctSize();

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CUIEdit::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "EDIT\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->_filename)
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->_filename);

	if (_image && _image->_filename)
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->_filename);

	if (_font && _font->_filename)
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontSelected && _fontSelected->_filename)
		buffer->putTextIndent(indent + 2, "FONT_SELECTED=\"%s\"\n", _fontSelected->_filename);

	if (_cursor && _cursor->_filename)
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	buffer->putTextIndent(indent + 2, "\n");

	if (_text)
		buffer->putTextIndent(indent + 2, "TEXT=\"%s\"\n", _text);

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);
	buffer->putTextIndent(indent + 2, "MAX_LENGTH=%d\n", _maxLength);
	buffer->putTextIndent(indent + 2, "CURSOR_BLINK_RATE=%d\n", _cursorBlinkRate);
	buffer->putTextIndent(indent + 2, "FRAME_WIDTH=%d\n", _frameWidth);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	// scripts
	for (int i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool CUIEdit::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSelectedFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSelectedFont") == 0) {
		stack->correctParams(1);

		if (_fontSelected) _gameRef->_fontStorage->removeFont(_fontSelected);
		_fontSelected = _gameRef->_fontStorage->addFont(stack->pop()->getString());
		stack->pushBool(_fontSelected != NULL);

		return STATUS_OK;
	}

	else return CUIObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIEdit::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("editor");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelStart") == 0) {
		_scValue->setInt(_selStart);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelEnd") == 0) {
		_scValue->setInt(_selEnd);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorBlinkRate") == 0) {
		_scValue->setInt(_cursorBlinkRate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorChar") == 0) {
		_scValue->setString(_cursorChar);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FrameWidth") == 0) {
		_scValue->setInt(_frameWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxLength") == 0) {
		_scValue->setInt(_maxLength);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::ansiToWide(_text);
			_scValue->setString(StringUtil::wideToUtf8(wstr).c_str());
		} else {
			_scValue->setString(_text);
		}
		return _scValue;
	}

	else return CUIObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelStart") == 0) {
		_selStart = value->getInt();
		_selStart = MAX(_selStart, 0);
		_selStart = MIN((size_t)_selStart, strlen(_text));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelEnd") == 0) {
		_selEnd = value->getInt();
		_selEnd = MAX(_selEnd, 0);
		_selEnd = MIN((size_t)_selEnd, strlen(_text));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorBlinkRate") == 0) {
		_cursorBlinkRate = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorChar") == 0) {
		setCursorChar(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FrameWidth") == 0) {
		_frameWidth = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MaxLength") == 0) {
		_maxLength = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::utf8ToWide(value->getString());
			setText(StringUtil::wideToAnsi(wstr).c_str());
		} else {
			setText(value->getString());
		}
		return STATUS_OK;
	}

	else return CUIObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIEdit::scToString() {
	return "[edit]";
}


//////////////////////////////////////////////////////////////////////////
void CUIEdit::setCursorChar(const char *character) {
	if (!character) return;
	delete[] _cursorChar;
	_cursorChar = new char [strlen(character) + 1];
	if (_cursorChar) strcpy(_cursorChar, character);
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::display(int offsetX, int offsetY) {
	if (!_visible) return STATUS_OK;


	// hack!
	TTextEncoding OrigEncoding = _gameRef->_textEncoding;
	_gameRef->_textEncoding = TEXT_ANSI;

	if (_back) _back->display(offsetX + _posX, offsetY + _posY, _width, _height);
	if (_image) _image->draw(offsetX + _posX, offsetY + _posY, NULL);

	// prepare fonts
	CBFont *font;
	CBFont *sfont;

	if (_font) font = _font;
	else font = _gameRef->_systemFont;

	if (_fontSelected) sfont = _fontSelected;
	else sfont = font;

	bool focused = isFocused();

	_selStart = MAX(_selStart, 0);
	_selEnd   = MAX(_selEnd, 0);

	_selStart = MIN((size_t)_selStart, strlen(_text));
	_selEnd   = MIN((size_t)_selEnd,   strlen(_text));

	//int CursorWidth = font->GetCharWidth(_cursorChar[0]);
	int cursorWidth = font->getTextWidth((byte *)_cursorChar);

	int s1, s2;
	bool curFirst;
	// modify scroll offset
	if (_selStart >= _selEnd) {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX(0, _selEnd - _scrollOffset)) > _width - cursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) break;
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selEnd;
		s2 = _selStart;
		curFirst = true;
	} else {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX(0, _selStart - _scrollOffset)) +
		        sfont->getTextWidth((byte *)(_text + MAX(_scrollOffset, _selStart)), _selEnd - MAX(_scrollOffset, _selStart))

		        > _width - cursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) break;
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selStart;
		s2 = _selEnd;
		curFirst = false;
	}


	int AlignOffset = 0;

	for (int Count = 0; Count < 2; Count++) {
		// draw text
		int xxx, yyy, width, height;

		xxx = _posX + _frameWidth + offsetX;
		yyy = _posY + _frameWidth + offsetY;

		width = _posX + _width + offsetX - _frameWidth;
		height = MAX(font->getLetterHeight(), sfont->getLetterHeight());

		if (_gameRef->_textRTL) xxx += AlignOffset;

		TTextAlign Align = TAL_LEFT;


		// unselected 1
		if (s1 > _scrollOffset) {
			if (Count) font->drawText((byte *)_text + _scrollOffset, xxx, yyy, width - xxx, Align, height, s1 - _scrollOffset);
			xxx += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
			AlignOffset += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
		}

		// cursor
		if (focused && curFirst) {
			if (Count) {
				if (CBPlatform::getTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::getTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += cursorWidth;
			AlignOffset += cursorWidth;
		}

		// selected
		int s3 = MAX(s1, _scrollOffset);

		if (s2 - s3 > 0) {
			if (Count) sfont->drawText((byte *)_text + s3, xxx, yyy, width - xxx, Align, height, s2 - s3);
			xxx += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
			AlignOffset += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
		}

		// cursor
		if (focused && !curFirst) {
			if (Count) {
				if (CBPlatform::getTime() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = CBPlatform::getTime();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible)
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, Align, height, 1);
			}
			xxx += cursorWidth;
			AlignOffset += cursorWidth;
		}

		// unselected 2
		if (Count) font->drawText((byte *)_text + s2, xxx, yyy, width - xxx, Align, height);
		AlignOffset += font->getTextWidth((byte *)_text + s2);

		AlignOffset = (_width - 2 * _frameWidth) - AlignOffset;
		if (AlignOffset < 0) AlignOffset = 0;
	}


	_gameRef->_renderer->_rectList.add(new CBActiveRect(_gameRef,  this, NULL, offsetX + _posX, offsetY + _posY, _width, _height, 100, 100, false));


	_gameRef->_textEncoding = OrigEncoding;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CUIEdit::handleKeypress(Common::Event *event, bool printable) {
	bool handled = false;

	if (event->type == Common::EVENT_KEYDOWN && !printable) {
		switch (event->kbd.keycode) {
		case Common::KEYCODE_ESCAPE:
		case Common::KEYCODE_TAB:
		case Common::KEYCODE_RETURN:
			return false;

			// ctrl+A
		case Common::KEYCODE_a:
			if (CBKeyboardState::isControlDown()) {
				_selStart = 0;
				_selEnd = strlen(_text);
				handled = true;
			}
			break;

		case Common::KEYCODE_BACKSPACE:
			if (_selStart == _selEnd) {
				if (_gameRef->_textRTL) deleteChars(_selStart, _selStart + 1);
				else deleteChars(_selStart - 1, _selStart);
			} else deleteChars(_selStart, _selEnd);
			if (_selEnd >= _selStart) _selEnd -= MAX(1, _selEnd - _selStart);
			_selStart = _selEnd;

			handled = true;
			break;

		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_UP:
			_selEnd--;
			if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			handled = true;
			break;

		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_DOWN:
			_selEnd++;
			if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			handled = true;
			break;

		case Common::KEYCODE_HOME:
			if (_gameRef->_textRTL) {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = 0;
				if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			}
			handled = true;
			break;

		case Common::KEYCODE_END:
			if (_gameRef->_textRTL) {
				_selEnd = 0;
				if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			} else {
				_selEnd = strlen(_text);
				if (!CBKeyboardState::isShiftDown()) _selStart = _selEnd;
			}
			handled = true;
			break;

		case Common::KEYCODE_DELETE:
			if (_selStart == _selEnd) {
				if (_gameRef->_textRTL) {
					deleteChars(_selStart - 1, _selStart);
					_selEnd--;
					if (_selEnd < 0) _selEnd = 0;
				} else deleteChars(_selStart, _selStart + 1);
			} else deleteChars(_selStart, _selEnd);
			if (_selEnd > _selStart) _selEnd -= (_selEnd - _selStart);

			_selStart = _selEnd;
			handled = true;
			break;
		default:
			break;
		}
		return handled;
	} else if (event->type == Common::EVENT_KEYDOWN && printable) {
		if (_selStart != _selEnd) deleteChars(_selStart, _selEnd);

		//WideString wstr = StringUtil::Utf8ToWide(event->kbd.ascii);
		WideString wstr;
		wstr += (char)event->kbd.ascii;
		_selEnd += insertChars(_selEnd, (byte *)StringUtil::wideToAnsi(wstr).c_str(), 1);

		if (_gameRef->_textRTL) _selEnd = _selStart;
		else _selStart = _selEnd;

		return true;
	}

	return false;
}



//////////////////////////////////////////////////////////////////////////
int CUIEdit::deleteChars(int start, int end) {
	if (start > end) CBUtils::swap(&start, &end);

	start = MAX(start, (int)0);
	end = MIN((size_t)end, strlen(_text));

	char *str = new char[strlen(_text) - (end - start) + 1];
	if (str) {
		if (start > 0) memcpy(str, _text, start);
		memcpy(str + MAX(0, start), _text + end, strlen(_text) - end + 1);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->applyEvent(_name);

	return end - start;
}


//////////////////////////////////////////////////////////////////////////
int CUIEdit::insertChars(int pos, byte *chars, int num) {
	if ((int)strlen(_text) + num > _maxLength) {
		num -= (strlen(_text) + num - _maxLength);
	}

	pos = MAX(pos, (int)0);
	pos = MIN((size_t)pos, strlen(_text));

	char *str = new char[strlen(_text) + num + 1];
	if (str) {
		if (pos > 0) memcpy(str, _text, pos);
		memcpy(str + pos + num, _text + pos, strlen(_text) - pos + 1);

		memcpy(str + pos, chars, num);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) _parent->applyEvent(_name);

	return num;
}



//////////////////////////////////////////////////////////////////////////
bool CUIEdit::persist(CBPersistMgr *persistMgr) {

	CUIObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_cursorBlinkRate));
	persistMgr->transfer(TMEMBER(_cursorChar));
	persistMgr->transfer(TMEMBER(_fontSelected));
	persistMgr->transfer(TMEMBER(_frameWidth));
	persistMgr->transfer(TMEMBER(_maxLength));
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_selEnd));
	persistMgr->transfer(TMEMBER(_selStart));

	if (!persistMgr->_saving) {
		_cursorVisible = false;
		_lastBlinkTime = 0;
	}

	return STATUS_OK;
}

} // end of namespace WinterMute
