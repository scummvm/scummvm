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

#include "engines/wintermute/ui/ui_edit.h"
#include "engines/wintermute/ui/ui_object.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/utils/string_util.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/utils/utils.h"
#include "common/util.h"
#include "common/keyboard.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIEdit, false)

//////////////////////////////////////////////////////////////////////////
UIEdit::UIEdit(BaseGame *inGame) : UIObject(inGame) {
	_type = UI_EDIT;

	_fontSelected = nullptr;

	_selStart = _selEnd = 10000;
	_scrollOffset = 0;

	_cursorChar = nullptr;
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
UIEdit::~UIEdit() {
	if (!_sharedFonts) {
		if (_fontSelected) {
			_gameRef->_fontStorage->removeFont(_fontSelected);
		}
	}

	delete[] _cursorChar;
	_cursorChar = nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool UIEdit::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "UIEdit::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing EDIT file '%s'", filename);
	}

	delete[] buffer;

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
bool UIEdit::loadBuffer(char *buffer, bool complete) {
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

	char *params;
	int cmd = 2;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_EDIT) {
			_gameRef->LOG(0, "'EDIT' keyword expected.");
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

		case TOKEN_BACK:
			delete _back;
			_back = new UITiledImage(_gameRef);
			if (!_back || DID_FAIL(_back->loadFile(params))) {
				delete _back;
				_back = nullptr;
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

		case TOKEN_FONT:
			if (_font) {
				_gameRef->_fontStorage->removeFont(_font);
			}
			_font = _gameRef->_fontStorage->addFont(params);
			if (!_font) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_SELECTED:
			if (_fontSelected) {
				_gameRef->_fontStorage->removeFont(_fontSelected);
			}
			_fontSelected = _gameRef->_fontStorage->addFont(params);
			if (!_fontSelected) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_TEXT:
			setText(params);
			_gameRef->expandStringByStringTable(&_text);
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

		case TOKEN_MAX_LENGTH:
			parser.scanStr(params, "%d", &_maxLength);
			break;

		case TOKEN_CAPTION:
			setCaption(params);
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

		case TOKEN_CURSOR_BLINK_RATE:
			parser.scanStr(params, "%d", &_cursorBlinkRate);
			break;

		case TOKEN_FRAME_WIDTH:
			parser.scanStr(params, "%d", &_frameWidth);
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

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		default:
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
bool UIEdit::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "EDIT\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->getFilename()) {
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->getFilename());
	}

	if (_image && _image->getFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->getFilename());
	}

	if (_font && _font->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->getFilename());
	}
	if (_fontSelected && _fontSelected->getFilename()) {
		buffer->putTextIndent(indent + 2, "FONT_SELECTED=\"%s\"\n", _fontSelected->getFilename());
	}

	if (_cursor && _cursor->getFilename()) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->getFilename());
	}

	buffer->putTextIndent(indent + 2, "\n");

	if (_text) {
		buffer->putTextIndent(indent + 2, "TEXT=\"%s\"\n", _text);
	}

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
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIEdit::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetSelectedFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSelectedFont") == 0) {
		stack->correctParams(1);

		if (_fontSelected) {
			_gameRef->_fontStorage->removeFont(_fontSelected);
		}
		_fontSelected = _gameRef->_fontStorage->addFont(stack->pop()->getString());
		stack->pushBool(_fontSelected != nullptr);

		return STATUS_OK;
	} else {
		return UIObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIEdit::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("editor");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SelStart") {
		_scValue->setInt(_selStart);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SelEnd") {
		_scValue->setInt(_selEnd);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CursorBlinkRate") {
		_scValue->setInt(_cursorBlinkRate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorChar
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CursorChar") {
		_scValue->setString(_cursorChar);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FrameWidth
	//////////////////////////////////////////////////////////////////////////
	else if (name == "FrameWidth") {
		_scValue->setInt(_frameWidth);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MaxLength
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MaxLength") {
		_scValue->setInt(_maxLength);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Text") {
		if (_gameRef->_textEncoding == TEXT_UTF8) {
			WideString wstr = StringUtil::ansiToWide(_text);
			_scValue->setString(StringUtil::wideToUtf8(wstr).c_str());
		} else {
			_scValue->setString(_text);
		}
		return _scValue;
	} else {
		return UIObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIEdit::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// SelStart
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SelStart") == 0) {
		_selStart = value->getInt();
		_selStart = MAX<int32>(_selStart, 0);
		_selStart = (int)MIN((size_t)_selStart, strlen(_text));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SelEnd
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SelEnd") == 0) {
		_selEnd = value->getInt();
		_selEnd = MAX<int32>(_selEnd, 0);
		_selEnd = (int)MIN((size_t)_selEnd, strlen(_text));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CursorBlinkRate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CursorBlinkRate") == 0) {
		_cursorBlinkRate = (uint32)value->getInt();
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
	} else {
		return UIObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIEdit::scToString() {
	return "[edit]";
}


//////////////////////////////////////////////////////////////////////////
void UIEdit::setCursorChar(const char *character) {
	if (!character) {
		return;
	}
	delete[] _cursorChar;
	_cursorChar = new char [strlen(character) + 1];
	if (_cursorChar) {
		strcpy(_cursorChar, character);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIEdit::display(int offsetX, int offsetY) {
	if (!_visible) {
		return STATUS_OK;
	}


	// hack!
	TTextEncoding OrigEncoding = _gameRef->_textEncoding;
	_gameRef->_textEncoding = TEXT_ANSI;

	if (_back) {
		_back->display(offsetX + _posX, offsetY + _posY, _width, _height);
	}
	if (_image) {
		_image->draw(offsetX + _posX, offsetY + _posY, nullptr);
	}

	// prepare fonts
	BaseFont *font;
	BaseFont *sfont;

	if (_font) {
		font = _font;
	} else {
		font = _gameRef->getSystemFont();
	}

	if (_fontSelected) {
		sfont = _fontSelected;
	} else {
		sfont = font;
	}

	bool focused = isFocused();

	_selStart = MAX<int32>(_selStart, 0);
	_selEnd   = MAX<int32>(_selEnd, 0);

	_selStart = (int)MIN((size_t)_selStart, strlen(_text));
	_selEnd   = (int)MIN((size_t)_selEnd,   strlen(_text));

	//int CursorWidth = font->GetCharWidth(_cursorChar[0]);
	int cursorWidth = font->getTextWidth((byte *)_cursorChar);

	int32 s1, s2;
	bool curFirst;
	// modify scroll offset
	if (_selStart >= _selEnd) {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX<int32>(0, _selEnd - _scrollOffset)) > _width - cursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) {
				break;
			}
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selEnd;
		s2 = _selStart;
		curFirst = true;
	} else {
		while (font->getTextWidth((byte *)_text + _scrollOffset, MAX<int32>(0, _selStart - _scrollOffset)) +
				sfont->getTextWidth((byte *)(_text + MAX<int32>(_scrollOffset, _selStart)), _selEnd - MAX(_scrollOffset, _selStart))

				> _width - cursorWidth - 2 * _frameWidth) {
			_scrollOffset++;
			if (_scrollOffset >= (int)strlen(_text)) {
				break;
			}
		}

		_scrollOffset = MIN(_scrollOffset, _selEnd);

		s1 = _selStart;
		s2 = _selEnd;
		curFirst = false;
	}


	int alignOffset = 0;

	for (int count = 0; count < 2; count++) {
		// draw text
		int xxx, yyy, width, height;

		xxx = _posX + _frameWidth + offsetX;
		yyy = _posY + _frameWidth + offsetY;

		width = _posX + _width + offsetX - _frameWidth;
		height = MAX(font->getLetterHeight(), sfont->getLetterHeight());

		if (_gameRef->_textRTL) {
			xxx += alignOffset;
		}

		TTextAlign align = TAL_LEFT;


		// unselected 1
		if (s1 > _scrollOffset) {
			if (count) {
				font->drawText((byte *)_text + _scrollOffset, xxx, yyy, width - xxx, align, height, s1 - _scrollOffset);
			}
			xxx += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
			alignOffset += font->getTextWidth((byte *)_text + _scrollOffset, s1 - _scrollOffset);
		}

		// cursor
		if (focused && curFirst) {
			if (count) {
				if (g_system->getMillis() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = g_system->getMillis();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible) {
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, align, height, 1);
				}
			}
			xxx += cursorWidth;
			alignOffset += cursorWidth;
		}

		// selected
		int s3 = MAX(s1, _scrollOffset);

		if (s2 - s3 > 0) {
			if (count) {
				sfont->drawText((byte *)_text + s3, xxx, yyy, width - xxx, align, height, s2 - s3);
			}
			xxx += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
			alignOffset += sfont->getTextWidth((byte *)_text + s3, s2 - s3);
		}

		// cursor
		if (focused && !curFirst) {
			if (count) {
				if (g_system->getMillis() - _lastBlinkTime >= _cursorBlinkRate) {
					_lastBlinkTime = g_system->getMillis();
					_cursorVisible = !_cursorVisible;
				}
				if (_cursorVisible) {
					font->drawText((byte *)_cursorChar, xxx, yyy, width - xxx, align, height, 1);
				}
			}
			xxx += cursorWidth;
			alignOffset += cursorWidth;
		}

		// unselected 2
		if (count) {
			font->drawText((byte *)_text + s2, xxx, yyy, width - xxx, align, height);
		}
		alignOffset += font->getTextWidth((byte *)_text + s2);

		alignOffset = (_width - 2 * _frameWidth) - alignOffset;
		if (alignOffset < 0) {
			alignOffset = 0;
		}
	}


	_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef,  this, nullptr, offsetX + _posX, offsetY + _posY, _width, _height, 100, 100, false));


	_gameRef->_textEncoding = OrigEncoding;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIEdit::handleKeypress(Common::Event *event, bool printable) {
	if (event->type != Common::EVENT_KEYDOWN) {
		return false;
	}

	switch (event->kbd.keycode) {

	// Those keys are 'printable' in WME 1.x, let's ignore them
	case Common::KEYCODE_ESCAPE:
	case Common::KEYCODE_TAB:
	case Common::KEYCODE_KP_ENTER:
	case Common::KEYCODE_RETURN:
		return false;

	// Movement keys are controlling _selEnd and _selStart
	case Common::KEYCODE_HOME:
	case Common::KEYCODE_END:
	case Common::KEYCODE_LEFT:
	case Common::KEYCODE_UP:
	case Common::KEYCODE_RIGHT:
	case Common::KEYCODE_DOWN:
		if (event->kbd.keycode == Common::KEYCODE_HOME) {
			_selEnd = _gameRef->_textRTL ? strlen(_text) : 0;
		} else if (event->kbd.keycode == Common::KEYCODE_END) {
			_selEnd = _gameRef->_textRTL ? 0 : strlen(_text);
		} else if (event->kbd.keycode == Common::KEYCODE_LEFT ||
				   event->kbd.keycode == Common::KEYCODE_UP) {
			_selEnd--;
		} else {
			_selEnd++;
		}
			
		if (!BaseKeyboardState::isShiftDown()) {
			_selStart = _selEnd;
		}
		return true;

	// Delete right
	case Common::KEYCODE_DELETE:
		if (_selStart == _selEnd) {
			if (_gameRef->_textRTL) {
				deleteChars(_selStart - 1, _selStart);
				_selEnd--;
				if (_selEnd < 0) {
					_selEnd = 0;
				}
			} else {
				deleteChars(_selStart, _selStart + 1);
			}
		} else {
			deleteChars(_selStart, _selEnd);
		}
		if (_selEnd > _selStart) {
			_selEnd -= (_selEnd - _selStart);
		}
		_selStart = _selEnd;
		return true;

	// Delete left
	case Common::KEYCODE_BACKSPACE:
		if (_selStart == _selEnd) {
			if (_gameRef->_textRTL) {
				deleteChars(_selStart, _selStart + 1);
			} else {
				deleteChars(_selStart - 1, _selStart);
			}
		} else {
			deleteChars(_selStart, _selEnd);
		}
		if (_selEnd >= _selStart) {
			_selEnd -= MAX<int32>(1, _selEnd - _selStart);
		}
		_selStart = _selEnd;
		return true;

	default:
		break;
	}

	// Ctrl+A = Select All
	if (BaseKeyboardState::isControlDown() && event->kbd.keycode == Common::KEYCODE_a) {
		_selStart = 0;
		_selEnd = strlen(_text);
		return true;
	}

	// Those are actually printable characters
	else if (printable) {
		if (_selStart != _selEnd) {
			deleteChars(_selStart, _selEnd);
		}

		//WideString wstr = StringUtil::Utf8ToWide(event->kbd.ascii);
		WideString wstr;
		wstr += (char)event->kbd.ascii;
		_selEnd += insertChars(_selEnd, (const byte *)StringUtil::wideToAnsi(wstr).c_str(), 1);

		if (_gameRef->_textRTL) {
			_selEnd = _selStart;
		} else {
			_selStart = _selEnd;
		}

		return true;
	}

	return false;
}



//////////////////////////////////////////////////////////////////////////
int UIEdit::deleteChars(int start, int end) {
	if (start > end) {
		BaseUtils::swap(&start, &end);
	}

	start = MAX(start, (int)0);
	end = MIN((size_t)end, strlen(_text));

	char *str = new char[strlen(_text) - (end - start) + 1];
	if (str) {
		if (start > 0) {
			memcpy(str, _text, start);
		}
		memcpy(str + MAX(0, start), _text + end, strlen(_text) - end + 1);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) {
		_parent->applyEvent(getName());
	}

	return end - start;
}


//////////////////////////////////////////////////////////////////////////
int UIEdit::insertChars(int pos, const byte *chars, int num) {
	if ((_maxLength != -1) && (int)strlen(_text) + num > _maxLength) {
		num -= (strlen(_text) + num - _maxLength);
	}

	pos = MAX(pos, (int)0);
	pos = MIN((size_t)pos, strlen(_text));

	char *str = new char[strlen(_text) + num + 1];
	if (str) {
		if (pos > 0) {
			memcpy(str, _text, pos);
		}
		memcpy(str + pos + num, _text + pos, strlen(_text) - pos + 1);

		memcpy(str + pos, chars, num);

		delete[] _text;
		_text = str;
	}
	if (_parentNotify && _parent) {
		_parent->applyEvent(getName());
	}

	return num;
}



//////////////////////////////////////////////////////////////////////////
bool UIEdit::persist(BasePersistenceManager *persistMgr) {

	UIObject::persist(persistMgr);

	persistMgr->transferUint32(TMEMBER(_cursorBlinkRate));
	persistMgr->transferCharPtr(TMEMBER(_cursorChar));
	persistMgr->transferPtr(TMEMBER_PTR(_fontSelected));
	persistMgr->transferSint32(TMEMBER(_frameWidth));
	persistMgr->transferSint32(TMEMBER(_maxLength));
	persistMgr->transferSint32(TMEMBER(_scrollOffset));
	persistMgr->transferSint32(TMEMBER(_selEnd));
	persistMgr->transferSint32(TMEMBER(_selStart));

	if (!persistMgr->getIsSaving()) {
		_cursorVisible = false;
		_lastBlinkTime = 0;
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
