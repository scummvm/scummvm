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

#include "dcgf.h"
#include "AdGame.h"
#include "AdResponseBox.h"
#include "BParser.h"
#include "BSurfaceStorage.h"
#include "UIButton.h"
#include "UIWindow.h"
#include "BDynBuffer.h"
#include "BFontStorage.h"
#include "BFont.h"
#include "AdResponse.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "BSprite.h"
#include "BFileManager.h"
#include "utils.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdResponseBox, false)

//////////////////////////////////////////////////////////////////////////
CAdResponseBox::CAdResponseBox(CBGame *inGame): CBObject(inGame) {
	_font = _fontHover = NULL;

	_window = NULL;
	_shieldWindow = new CUIWindow(Game);

	_horizontal = false;
	CBPlatform::SetRectEmpty(&_responseArea);
	_scrollOffset = 0;
	_spacing = 0;

	_waitingScript = NULL;
	_lastResponseText = NULL;
	_lastResponseTextOrig = NULL;

	_verticalAlign = VAL_BOTTOM;
	_align = TAL_LEFT;
}


//////////////////////////////////////////////////////////////////////////
CAdResponseBox::~CAdResponseBox() {

	SAFE_DELETE(_window);
	SAFE_DELETE(_shieldWindow);
	SAFE_DELETE_ARRAY(_lastResponseText);
	SAFE_DELETE_ARRAY(_lastResponseTextOrig);

	if (_font) Game->_fontStorage->RemoveFont(_font);
	if (_fontHover) Game->_fontStorage->RemoveFont(_fontHover);

	ClearResponses();
	ClearButtons();

	_waitingScript = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::ClearResponses() {
	for (int i = 0; i < _responses.GetSize(); i++) {
		delete _responses[i];
	}
	_responses.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::ClearButtons() {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		delete _respButtons[i];
	}
	_respButtons.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::InvalidateButtons() {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		_respButtons[i]->_image = NULL;
		_respButtons[i]->_cursor = NULL;
		_respButtons[i]->_font = NULL;
		_respButtons[i]->_fontHover = NULL;
		_respButtons[i]->_fontPress = NULL;
		_respButtons[i]->SetText("");
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::CreateButtons() {
	ClearButtons();

	_scrollOffset = 0;
	for (int i = 0; i < _responses.GetSize(); i++) {
		CUIButton *btn = new CUIButton(Game);
		if (btn) {
			btn->_parent = _window;
			btn->_sharedFonts = btn->_sharedImages = true;
			btn->_sharedCursors = true;
			// iconic
			if (_responses[i]->_icon) {
				btn->_image = _responses[i]->_icon;
				if (_responses[i]->_iconHover)   btn->_imageHover = _responses[i]->_iconHover;
				if (_responses[i]->_iconPressed) btn->_imagePress = _responses[i]->_iconPressed;

				btn->SetCaption(_responses[i]->_text);
				if (_cursor) btn->_cursor = _cursor;
				else if (Game->_activeCursor) btn->_cursor = Game->_activeCursor;
			}
			// textual
			else {
				btn->SetText(_responses[i]->_text);
				btn->_font = (_font == NULL) ? Game->_systemFont : _font;
				btn->_fontHover = (_fontHover == NULL) ? Game->_systemFont : _fontHover;
				btn->_fontPress = btn->_fontHover;
				btn->_align = _align;

				if (Game->_touchInterface)
					btn->_fontHover = btn->_font;


				if (_responses[i]->_font) btn->_font = _responses[i]->_font;

				btn->_width = _responseArea.right - _responseArea.left;
				if (btn->_width <= 0) btn->_width = Game->_renderer->_width;
			}
			btn->SetName("response");
			btn->CorrectSize();

			// make the responses touchable
			if (Game->_touchInterface)
				btn->_height = std::max(btn->_height, 50);

			//btn->SetListener(this, btn, _responses[i]->_iD);
			btn->SetListener(this, btn, i);
			btn->_visible = false;
			_respButtons.Add(btn);

			if (_responseArea.bottom - _responseArea.top < btn->_height) {
				Game->LOG(0, "Warning: Response '%s' is too high to be displayed within response box. Correcting.", _responses[i]->_text);
				_responseArea.bottom += (btn->_height - (_responseArea.bottom - _responseArea.top));
			}
		}
	}
	_ready = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::LoadFile(char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdResponseBox::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing RESPONSE_BOX file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(RESPONSE_BOX)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(FONT_HOVER)
TOKEN_DEF(FONT)
TOKEN_DEF(AREA)
TOKEN_DEF(HORIZONTAL)
TOKEN_DEF(SPACING)
TOKEN_DEF(WINDOW)
TOKEN_DEF(CURSOR)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(VERTICAL_ALIGN)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(RESPONSE_BOX)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(FONT_HOVER)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(AREA)
	TOKEN_TABLE(HORIZONTAL)
	TOKEN_TABLE(SPACING)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(VERTICAL_ALIGN)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END


	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_RESPONSE_BOX) {
			Game->LOG(0, "'RESPONSE_BOX' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_WINDOW:
			SAFE_DELETE(_window);
			_window = new CUIWindow(Game);
			if (!_window || FAILED(_window->LoadBuffer(params, false))) {
				SAFE_DELETE(_window);
				cmd = PARSERR_GENERIC;
			} else if (_shieldWindow) _shieldWindow->_parent = _window;
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

		case TOKEN_AREA:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_responseArea.left, &_responseArea.top, &_responseArea.right, &_responseArea.bottom);
			break;

		case TOKEN_HORIZONTAL:
			parser.ScanStr((char *)params, "%b", &_horizontal);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "center") == 0) _align = TAL_CENTER;
			else if (scumm_stricmp((char *)params, "right") == 0) _align = TAL_RIGHT;
			else _align = TAL_LEFT;
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp((char *)params, "top") == 0) _verticalAlign = VAL_TOP;
			else if (scumm_stricmp((char *)params, "center") == 0) _verticalAlign = VAL_CENTER;
			else _verticalAlign = VAL_BOTTOM;
			break;

		case TOKEN_SPACING:
			parser.ScanStr((char *)params, "%d", &_spacing);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_CURSOR:
			SAFE_DELETE(_cursor);
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->LoadFile((char *)params))) {
				SAFE_DELETE(_cursor);
				cmd = PARSERR_GENERIC;
			}
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in RESPONSE_BOX definition");
		return E_FAIL;
	}

	if (_window) {
		for (int i = 0; i < _window->_widgets.GetSize(); i++) {
			if (!_window->_widgets[i]->_listenerObject)
				_window->_widgets[i]->SetListener(this, _window->_widgets[i], 0);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "RESPONSE_BOX\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "AREA { %d, %d, %d, %d }\n", _responseArea.left, _responseArea.top, _responseArea.right, _responseArea.bottom);

	if (_font && _font->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontHover && _fontHover->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	Buffer->PutTextIndent(Indent + 2, "HORIZONTAL=%s\n", _horizontal ? "TRUE" : "FALSE");

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

	Buffer->PutTextIndent(Indent + 2, "SPACING=%d\n", _spacing);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// window
	if (_window) _window->SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Display() {
	RECT rect = _responseArea;
	if (_window) {
		CBPlatform::OffsetRect(&rect, _window->_posX, _window->_posY);
		//_window->Display();
	}

	int xxx, yyy, i;

	xxx = rect.left;
	yyy = rect.top;

	// shift down if needed
	if (!_horizontal) {
		int total_height = 0;
		for (i = 0; i < _respButtons.GetSize(); i++) total_height += (_respButtons[i]->_height + _spacing);
		total_height -= _spacing;

		switch (_verticalAlign) {
		case VAL_BOTTOM:
			if (yyy + total_height < rect.bottom)
				yyy = rect.bottom - total_height;
			break;

		case VAL_CENTER:
			if (yyy + total_height < rect.bottom)
				yyy += ((rect.bottom - rect.top) - total_height) / 2;
			break;

		case VAL_TOP:
			// do nothing
			break;
		}
	}

	// prepare response buttons
	bool scroll_needed = false;
	for (i = _scrollOffset; i < _respButtons.GetSize(); i++) {
		if ((_horizontal && xxx + _respButtons[i]->_width > rect.right)
		        || (!_horizontal && yyy + _respButtons[i]->_height > rect.bottom)) {

			scroll_needed = true;
			_respButtons[i]->_visible = false;
			break;
		}

		_respButtons[i]->_visible = true;
		_respButtons[i]->_posX = xxx;
		_respButtons[i]->_posY = yyy;

		if (_horizontal) {
			xxx += (_respButtons[i]->_width + _spacing);
		} else {
			yyy += (_respButtons[i]->_height + _spacing);
		}
	}

	// show appropriate scroll buttons
	if (_window) {
		_window->ShowWidget("prev", _scrollOffset > 0);
		_window->ShowWidget("next", scroll_needed);
	}

	// go exclusive
	if (_shieldWindow) {
		_shieldWindow->_posX = _shieldWindow->_posY = 0;
		_shieldWindow->_width = Game->_renderer->_width;
		_shieldWindow->_height = Game->_renderer->_height;

		_shieldWindow->Display();
	}

	// display window
	if (_window) _window->Display();


	// display response buttons
	for (i = _scrollOffset; i < _respButtons.GetSize(); i++) {
		_respButtons[i]->Display();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset--;
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset++;
		} else if (scumm_stricmp(obj->_name, "response") == 0) {
			if (_waitingScript) _waitingScript->_stack->PushInt(_responses[param2]->_iD);
			HandleResponse(_responses[param2]);
			_waitingScript = NULL;
			Game->_state = GAME_RUNNING;
			((CAdGame *)Game)->_stateEx = GAME_NORMAL;
			_ready = true;
			InvalidateButtons();
			ClearResponses();
		} else return CBObject::Listen(param1, param2);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_font));
	PersistMgr->Transfer(TMEMBER(_fontHover));
	PersistMgr->Transfer(TMEMBER(_horizontal));
	PersistMgr->Transfer(TMEMBER(_lastResponseText));
	PersistMgr->Transfer(TMEMBER(_lastResponseTextOrig));
	_respButtons.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_responseArea));
	_responses.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_scrollOffset));
	PersistMgr->Transfer(TMEMBER(_shieldWindow));
	PersistMgr->Transfer(TMEMBER(_spacing));
	PersistMgr->Transfer(TMEMBER(_waitingScript));
	PersistMgr->Transfer(TMEMBER(_window));

	PersistMgr->Transfer(TMEMBER_INT(_verticalAlign));
	PersistMgr->Transfer(TMEMBER_INT(_align));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::WeedResponses() {
	CAdGame *AdGame = (CAdGame *)Game;

	for (int i = 0; i < _responses.GetSize(); i++) {
		switch (_responses[i]->_responseType) {
		case RESPONSE_ONCE:
			if (AdGame->BranchResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.RemoveAt(i);
				i--;
			}
			break;

		case RESPONSE_ONCE_GAME:
			if (AdGame->GameResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.RemoveAt(i);
				i--;
			}
			break;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::SetLastResponseText(char *Text, char *TextOrig) {
	CBUtils::SetString(&_lastResponseText, Text);
	CBUtils::SetString(&_lastResponseTextOrig, TextOrig);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::HandleResponse(CAdResponse *Response) {
	SetLastResponseText(Response->_text, Response->_textOrig);

	CAdGame *AdGame = (CAdGame *)Game;

	switch (Response->_responseType) {
	case RESPONSE_ONCE:
		AdGame->AddBranchResponse(Response->_iD);
		break;

	case RESPONSE_ONCE_GAME:
		AdGame->AddGameResponse(Response->_iD);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::GetNextAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = 0; i < Objects.GetSize(); i++) {
				if (Objects[i] == CurrObject) {
					if (i < Objects.GetSize() - 1) return Objects[i + 1];
					else break;
				}
			}
		}
		return Objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::GetPrevAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = Objects.GetSize() - 1; i >= 0; i--) {
				if (Objects[i] == CurrObject) {
					if (i > 0) return Objects[i - 1];
					else break;
				}
			}
		}
		return Objects[Objects.GetSize() - 1];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::GetObjects(CBArray<CUIObject *, CUIObject *>& Objects, bool InteractiveOnly) {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		Objects.Add(_respButtons[i]);
	}
	if (_window) _window->GetWindowObjects(Objects, InteractiveOnly);

	return S_OK;
}

} // end of namespace WinterMute
