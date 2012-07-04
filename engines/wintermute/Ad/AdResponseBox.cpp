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
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdResponseBox.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/UI/UIButton.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Ad/AdResponse.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
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

	delete _window;
	_window = NULL;
	delete _shieldWindow;
	_shieldWindow = NULL;
	delete[] _lastResponseText;
	_lastResponseText = NULL;
	delete[] _lastResponseTextOrig;
	_lastResponseTextOrig = NULL;

	if (_font) Game->_fontStorage->RemoveFont(_font);
	if (_fontHover) Game->_fontStorage->RemoveFont(_fontHover);

	clearResponses();
	clearButtons();

	_waitingScript = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::clearResponses() {
	for (int i = 0; i < _responses.GetSize(); i++) {
		delete _responses[i];
	}
	_responses.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::clearButtons() {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		delete _respButtons[i];
	}
	_respButtons.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::invalidateButtons() {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		_respButtons[i]->_image = NULL;
		_respButtons[i]->_cursor = NULL;
		_respButtons[i]->_font = NULL;
		_respButtons[i]->_fontHover = NULL;
		_respButtons[i]->_fontPress = NULL;
		_respButtons[i]->setText("");
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::createButtons() {
	clearButtons();

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

				btn->setCaption(_responses[i]->_text);
				if (_cursor) btn->_cursor = _cursor;
				else if (Game->_activeCursor) btn->_cursor = Game->_activeCursor;
			}
			// textual
			else {
				btn->setText(_responses[i]->_text);
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
			btn->setName("response");
			btn->correctSize();

			// make the responses touchable
			if (Game->_touchInterface)
				btn->_height = MAX(btn->_height, 50);

			//btn->SetListener(this, btn, _responses[i]->_iD);
			btn->setListener(this, btn, i);
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
HRESULT CAdResponseBox::loadFile(const char *filename) {
	byte *Buffer = Game->_fileManager->readWholeFile(filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdResponseBox::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(Buffer, true))) Game->LOG(0, "Error parsing RESPONSE_BOX file '%s'", filename);


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
HRESULT CAdResponseBox::loadBuffer(byte  *Buffer, bool Complete) {
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
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_WINDOW:
			delete _window;
			_window = new CUIWindow(Game);
			if (!_window || FAILED(_window->loadBuffer(params, false))) {
				delete _window;
				_window = NULL;
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
			parseEditorProperty(params, false);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
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
				_window->_widgets[i]->setListener(this, _window->_widgets[i], 0);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::saveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->putTextIndent(Indent, "RESPONSE_BOX\n");
	Buffer->putTextIndent(Indent, "{\n");

	Buffer->putTextIndent(Indent + 2, "AREA { %d, %d, %d, %d }\n", _responseArea.left, _responseArea.top, _responseArea.right, _responseArea.bottom);

	if (_font && _font->_filename)
		Buffer->putTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontHover && _fontHover->_filename)
		Buffer->putTextIndent(Indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->putTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	Buffer->putTextIndent(Indent + 2, "HORIZONTAL=%s\n", _horizontal ? "TRUE" : "FALSE");

	switch (_align) {
	case TAL_LEFT:
		Buffer->putTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->putTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->putTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("CAdResponseBox::SaveAsText - Unhandled enum");
		break;
	}

	switch (_verticalAlign) {
	case VAL_TOP:
		Buffer->putTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		Buffer->putTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		Buffer->putTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
		break;
	}

	Buffer->putTextIndent(Indent + 2, "SPACING=%d\n", _spacing);

	Buffer->putTextIndent(Indent + 2, "\n");

	// window
	if (_window) _window->saveAsText(Buffer, Indent + 2);

	Buffer->putTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(Buffer, Indent + 2);

	Buffer->putTextIndent(Indent, "}\n");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::display() {
	RECT rect = _responseArea;
	if (_window) {
		CBPlatform::OffsetRect(&rect, _window->_posX, _window->_posY);
		//_window->display();
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
		_window->showWidget("prev", _scrollOffset > 0);
		_window->showWidget("next", scroll_needed);
	}

	// go exclusive
	if (_shieldWindow) {
		_shieldWindow->_posX = _shieldWindow->_posY = 0;
		_shieldWindow->_width = Game->_renderer->_width;
		_shieldWindow->_height = Game->_renderer->_height;

		_shieldWindow->display();
	}

	// display window
	if (_window) _window->display();


	// display response buttons
	for (i = _scrollOffset; i < _respButtons.GetSize(); i++) {
		_respButtons[i]->display();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset--;
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset++;
		} else if (scumm_stricmp(obj->_name, "response") == 0) {
			if (_waitingScript) _waitingScript->_stack->pushInt(_responses[param2]->_iD);
			handleResponse(_responses[param2]);
			_waitingScript = NULL;
			Game->_state = GAME_RUNNING;
			((CAdGame *)Game)->_stateEx = GAME_NORMAL;
			_ready = true;
			invalidateButtons();
			clearResponses();
		} else return CBObject::listen(param1, param2);
		break;
	default:
		error("AdResponseBox::Listen - Unhandled enum");
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_font));
	persistMgr->transfer(TMEMBER(_fontHover));
	persistMgr->transfer(TMEMBER(_horizontal));
	persistMgr->transfer(TMEMBER(_lastResponseText));
	persistMgr->transfer(TMEMBER(_lastResponseTextOrig));
	_respButtons.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_responseArea));
	_responses.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER(_spacing));
	persistMgr->transfer(TMEMBER(_waitingScript));
	persistMgr->transfer(TMEMBER(_window));

	persistMgr->transfer(TMEMBER_INT(_verticalAlign));
	persistMgr->transfer(TMEMBER_INT(_align));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::weedResponses() {
	CAdGame *AdGame = (CAdGame *)Game;

	for (int i = 0; i < _responses.GetSize(); i++) {
		switch (_responses[i]->_responseType) {
		case RESPONSE_ONCE:
			if (AdGame->branchResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.RemoveAt(i);
				i--;
			}
			break;

		case RESPONSE_ONCE_GAME:
			if (AdGame->gameResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.RemoveAt(i);
				i--;
			}
			break;
		default:
			warning("CAdResponseBox::WeedResponses - Unhandled enum");
			break;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::setLastResponseText(const char *Text, const char *TextOrig) {
	CBUtils::setString(&_lastResponseText, Text);
	CBUtils::setString(&_lastResponseTextOrig, TextOrig);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::handleResponse(CAdResponse *Response) {
	setLastResponseText(Response->_text, Response->_textOrig);

	CAdGame *AdGame = (CAdGame *)Game;

	switch (Response->_responseType) {
	case RESPONSE_ONCE:
		AdGame->addBranchResponse(Response->_iD);
		break;

	case RESPONSE_ONCE_GAME:
		AdGame->addGameResponse(Response->_iD);
		break;
	default:
		warning("CAdResponseBox::HandleResponse - Unhandled enum");
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::getNextAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	getObjects(Objects, true);

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
CBObject *CAdResponseBox::getPrevAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	getObjects(Objects, true);

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
HRESULT CAdResponseBox::getObjects(CBArray<CUIObject *, CUIObject *> &Objects, bool InteractiveOnly) {
	for (int i = 0; i < _respButtons.GetSize(); i++) {
		Objects.Add(_respButtons[i]);
	}
	if (_window) _window->getWindowObjects(Objects, InteractiveOnly);

	return S_OK;
}

} // end of namespace WinterMute
