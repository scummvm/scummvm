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
#include "engines/wintermute/UIWindow.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BActiveRect.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BKeyboardState.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/UIButton.h"
#include "engines/wintermute/UIEdit.h"
#include "engines/wintermute/UIText.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BViewport.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIWindow, false)

//////////////////////////////////////////////////////////////////////////
CUIWindow::CUIWindow(CBGame *inGame): CUIObject(inGame) {
	CBPlatform::SetRectEmpty(&_titleRect);
	CBPlatform::SetRectEmpty(&_dragRect);
	_titleAlign = TAL_LEFT;
	_transparent = false;

	_backInactive = NULL;
	_fontInactive = NULL;
	_imageInactive = NULL;

	_type = UI_WINDOW;
	_canFocus = true;

	_dragging = false;
	_dragFrom.x = _dragFrom.y = 0;

	_mode = WINDOW_NORMAL;
	_shieldWindow = NULL;
	_shieldButton = NULL;

	_fadeColor = 0x00000000;
	_fadeBackground = false;

	_ready = true;
	_isMenu = false;
	_inGame = false;

	_clipContents = false;
	_viewport = NULL;

	_pauseMusic = true;
}


//////////////////////////////////////////////////////////////////////////
CUIWindow::~CUIWindow() {
	Close();
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::Cleanup() {
	delete _shieldWindow;
	delete _shieldButton;
	delete _viewport;
	_shieldWindow = NULL;
	_shieldButton = NULL;
	_viewport = NULL;

	if (_backInactive) delete _backInactive;
	if (!_sharedFonts && _fontInactive) Game->_fontStorage->RemoveFont(_fontInactive);
	if (!_sharedImages && _imageInactive) delete _imageInactive;

	for (int i = 0; i < _widgets.GetSize(); i++) delete _widgets[i];
	_widgets.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Display(int OffsetX, int OffsetY) {
	// go exclusive
	if (_mode == WINDOW_EXCLUSIVE || _mode == WINDOW_SYSTEM_EXCLUSIVE) {
		if (!_shieldWindow) _shieldWindow = new CUIWindow(Game);
		if (_shieldWindow) {
			_shieldWindow->_posX = _shieldWindow->_posY = 0;
			_shieldWindow->_width = Game->_renderer->_width;
			_shieldWindow->_height = Game->_renderer->_height;

			_shieldWindow->Display();
		}
	} else if (_isMenu) {
		if (!_shieldButton) {
			_shieldButton = new CUIButton(Game);
			_shieldButton->SetName("close");
			_shieldButton->SetListener(this, _shieldButton, 0);
			_shieldButton->_parent = this;
		}
		if (_shieldButton) {
			_shieldButton->_posX = _shieldButton->_posY = 0;
			_shieldButton->_width = Game->_renderer->_width;
			_shieldButton->_height = Game->_renderer->_height;

			_shieldButton->Display();
		}
	}

	if (!_visible) return S_OK;

	if (_fadeBackground) Game->_renderer->FadeToColor(_fadeColor);

	if (_dragging) {
		_posX += (Game->_mousePos.x - _dragFrom.x);
		_posY += (Game->_mousePos.y - _dragFrom.y);

		_dragFrom.x = Game->_mousePos.x;
		_dragFrom.y = Game->_mousePos.y;
	}

	if (!_focusedWidget || (!_focusedWidget->_canFocus || _focusedWidget->_disable || !_focusedWidget->_visible)) {
		MoveFocus();
	}

	bool PopViewport = false;
	if (_clipContents) {
		if (!_viewport) _viewport = new CBViewport(Game);
		if (_viewport) {
			_viewport->SetRect(_posX + OffsetX, _posY + OffsetY, _posX + _width + OffsetX, _posY + _height + OffsetY);
			Game->PushViewport(_viewport);
			PopViewport = true;
		}
	}


	CUITiledImage *back = _back;
	CBSprite *image = _image;
	CBFont *font = _font;

	if (!IsFocused()) {
		if (_backInactive) back = _backInactive;
		if (_imageInactive) image = _imageInactive;
		if (_fontInactive) font = _fontInactive;
	}

	if (_alphaColor != 0) Game->_renderer->_forceAlphaColor = _alphaColor;
	if (back) back->Display(_posX + OffsetX, _posY + OffsetY, _width, _height);
	if (image) image->Draw(_posX + OffsetX, _posY + OffsetY, _transparent ? NULL : this);

	if (!CBPlatform::IsRectEmpty(&_titleRect) && font && _text) {
		font->DrawText((byte  *)_text, _posX + OffsetX + _titleRect.left, _posY + OffsetY + _titleRect.top, _titleRect.right - _titleRect.left, _titleAlign, _titleRect.bottom - _titleRect.top);
	}

	if (!_transparent && !image) Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, _posX + OffsetX, _posY + OffsetY, _width, _height, 100, 100, false));

	for (int i = 0; i < _widgets.GetSize(); i++) {
		_widgets[i]->Display(_posX + OffsetX, _posY + OffsetY);
	}

	if (_alphaColor != 0) Game->_renderer->_forceAlphaColor = 0;

	if (PopViewport) Game->PopViewport();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::LoadFile(char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIWindow::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing WINDOW file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(WINDOW)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK_INACTIVE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE_INACTIVE)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_INACTIVE)
TOKEN_DEF(FONT)
TOKEN_DEF(TITLE_ALIGN)
TOKEN_DEF(TITLE_RECT)
TOKEN_DEF(TITLE)
TOKEN_DEF(DRAG_RECT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(FADE_ALPHA)
TOKEN_DEF(FADE_COLOR)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(BUTTON)
TOKEN_DEF(STATIC)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(MENU)
TOKEN_DEF(IN_GAME)
TOKEN_DEF(CLIP_CONTENTS)
TOKEN_DEF(PAUSE_MUSIC)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(EDIT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK_INACTIVE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE_INACTIVE)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_INACTIVE)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TITLE_ALIGN)
	TOKEN_TABLE(TITLE_RECT)
	TOKEN_TABLE(TITLE)
	TOKEN_TABLE(DRAG_RECT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(FADE_ALPHA)
	TOKEN_TABLE(FADE_COLOR)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(BUTTON)
	TOKEN_TABLE(STATIC)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(MENU)
	TOKEN_TABLE(IN_GAME)
	TOKEN_TABLE(CLIP_CONTENTS)
	TOKEN_TABLE(PAUSE_MUSIC)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(EDIT)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	int FadeR = 0, FadeG = 0, FadeB = 0, FadeA = 0;
	int ar = 0, ag = 0, ab = 0, alpha = 0;

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_WINDOW) {
			Game->LOG(0, "'WINDOW' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while (cmd >= PARSERR_TOKENNOTFOUND && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) >= PARSERR_TOKENNOTFOUND) {
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

		case TOKEN_BACK_INACTIVE:
			delete _backInactive;
			_backInactive = new CUITiledImage(Game);
			if (!_backInactive || FAILED(_backInactive->LoadFile((char *)params))) {
				delete _backInactive;
				_backInactive = NULL;
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

		case TOKEN_IMAGE_INACTIVE:
			delete _imageInactive,
			_imageInactive = new CBSprite(Game);
			if (!_imageInactive || FAILED(_imageInactive->LoadFile((char *)params))) {
				delete _imageInactive;
				_imageInactive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) Game->_fontStorage->RemoveFont(_font);
			_font = Game->_fontStorage->AddFont((char *)params);
			if (!_font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_INACTIVE:
			if (_fontInactive) Game->_fontStorage->RemoveFont(_fontInactive);
			_fontInactive = Game->_fontStorage->AddFont((char *)params);
			if (!_fontInactive) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TITLE:
			SetText((char *)params);
			Game->_stringTable->Expand(&_text);
			break;

		case TOKEN_TITLE_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) _titleAlign = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) _titleAlign = TAL_RIGHT;
			else _titleAlign = TAL_CENTER;
			break;

		case TOKEN_TITLE_RECT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_titleRect.left, &_titleRect.top, &_titleRect.right, &_titleRect.bottom);
			break;

		case TOKEN_DRAG_RECT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_dragRect.left, &_dragRect.top, &_dragRect.right, &_dragRect.bottom);
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

		case TOKEN_BUTTON: {
			CUIButton *btn = new CUIButton(Game);
			if (!btn || FAILED(btn->LoadBuffer(params, false))) {
				delete btn;
				btn = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				btn->_parent = this;
				_widgets.Add(btn);
			}
		}
		break;

		case TOKEN_STATIC: {
			CUIText *text = new CUIText(Game);
			if (!text || FAILED(text->LoadBuffer(params, false))) {
				delete text;
				text = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				text->_parent = this;
				_widgets.Add(text);
			}
		}
		break;

		case TOKEN_EDIT: {
			CUIEdit *edit = new CUIEdit(Game);
			if (!edit || FAILED(edit->LoadBuffer(params, false))) {
				delete edit;
				edit = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				edit->_parent = this;
				_widgets.Add(edit);
			}
		}
		break;

		case TOKEN_WINDOW: {
			CUIWindow *win = new CUIWindow(Game);
			if (!win || FAILED(win->LoadBuffer(params, false))) {
				delete win;
				win = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				win->_parent = this;
				_widgets.Add(win);
			}
		}
		break;


		case TOKEN_TRANSPARENT:
			parser.ScanStr((char *)params, "%b", &_transparent);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.ScanStr((char *)params, "%b", &_parentNotify);
			break;

		case TOKEN_PAUSE_MUSIC:
			parser.ScanStr((char *)params, "%b", &_pauseMusic);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &_visible);
			break;

		case TOKEN_MENU:
			parser.ScanStr((char *)params, "%b", &_isMenu);
			break;

		case TOKEN_IN_GAME:
			parser.ScanStr((char *)params, "%b", &_inGame);
			break;

		case TOKEN_CLIP_CONTENTS:
			parser.ScanStr((char *)params, "%b", &_clipContents);
			break;

		case TOKEN_FADE_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &FadeR, &FadeG, &FadeB);
			_fadeBackground = true;
			break;

		case TOKEN_FADE_ALPHA:
			parser.ScanStr((char *)params, "%d", &FadeA);
			_fadeBackground = true;
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;


		default:
			if (FAILED(Game->WindowLoadHook(this, (char **)&Buffer, (char **)params))) {
				cmd = PARSERR_GENERIC;
			}
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in WINDOW definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading WINDOW definition");
		return E_FAIL;
	}

	CorrectSize();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = DRGBA(ar, ag, ab, alpha);

	if (_fadeBackground) _fadeColor = DRGBA(FadeR, FadeG, FadeB, FadeA);

	_focusedWidget = NULL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "WINDOW\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_back && _back->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK=\"%s\"\n", _back->_filename);
	if (_backInactive && _backInactive->_filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_INACTIVE=\"%s\"\n", _backInactive->_filename);

	if (_image && _image->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", _image->_filename);
	if (_imageInactive && _imageInactive->_filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_INACTIVE=\"%s\"\n", _imageInactive->_filename);

	if (_font && _font->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontInactive && _fontInactive->_filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_INACTIVE=\"%s\"\n", _fontInactive->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_text)
		Buffer->PutTextIndent(Indent + 2, "TITLE=\"%s\"\n", _text);

	switch (_titleAlign) {
	case TAL_LEFT:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "center");
		break;
	}

	if (!CBPlatform::IsRectEmpty(&_titleRect)) {
		Buffer->PutTextIndent(Indent + 2, "TITLE_RECT { %d, %d, %d, %d }\n", _titleRect.left, _titleRect.top, _titleRect.right, _titleRect.bottom);
	}

	if (!CBPlatform::IsRectEmpty(&_dragRect)) {
		Buffer->PutTextIndent(Indent + 2, "DRAG_RECT { %d, %d, %d, %d }\n", _dragRect.left, _dragRect.top, _dragRect.right, _dragRect.bottom);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", _posX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", _posY);
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", _width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", _height);

	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "TRANSPARENT=%s\n", _transparent ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PAUSE_MUSIC=%s\n", _pauseMusic ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "MENU=%s\n", _isMenu ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "IN_GAME=%s\n", _inGame ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CLIP_CONTENTS=%s\n", _clipContents ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (_fadeBackground) {
		Buffer->PutTextIndent(Indent + 2, "FADE_COLOR { %d, %d, %d }\n", D3DCOLGetR(_fadeColor), D3DCOLGetG(_fadeColor), D3DCOLGetB(_fadeColor));
		Buffer->PutTextIndent(Indent + 2, "FADE_ALPHA=%d\n", D3DCOLGetA(_fadeColor));
	}

	Buffer->PutTextIndent(Indent + 2, "ALPHA_COLOR { %d, %d, %d }\n", D3DCOLGetR(_alphaColor), D3DCOLGetG(_alphaColor), D3DCOLGetB(_alphaColor));
	Buffer->PutTextIndent(Indent + 2, "ALPHA=%d\n", D3DCOLGetA(_alphaColor));

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	// controls
	for (int i = 0; i < _widgets.GetSize(); i++)
		_widgets[i]->SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::EnableWidget(char *Name, bool Enable) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, Name) == 0) _widgets[i]->_disable = !Enable;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ShowWidget(char *Name, bool Visible) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, Name) == 0) _widgets[i]->_visible = Visible;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetWidget / GetControl
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetWidget") == 0 || strcmp(Name, "GetControl") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		if (val->GetType() == VAL_INT) {
			int widget = val->GetInt();
			if (widget < 0 || widget >= _widgets.GetSize()) Stack->PushNULL();
			else Stack->PushNative(_widgets[widget], true);
		} else {
			for (int i = 0; i < _widgets.GetSize(); i++) {
				if (scumm_stricmp(_widgets[i]->_name, val->GetString()) == 0) {
					Stack->PushNative(_widgets[i], true);
					return S_OK;
				}
			}
			Stack->PushNULL();
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetInactiveFont") == 0) {
		Stack->CorrectParams(1);

		if (_fontInactive) Game->_fontStorage->RemoveFont(_fontInactive);
		_fontInactive = Game->_fontStorage->AddFont(Stack->Pop()->GetString());
		Stack->PushBool(_fontInactive != NULL);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetInactiveImage") == 0) {
		Stack->CorrectParams(1);

		delete _imageInactive;
		_imageInactive = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!_imageInactive || FAILED(_imageInactive->LoadFile(Filename))) {
			delete _imageInactive;
			_imageInactive = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInactiveImage") == 0) {
		Stack->CorrectParams(0);
		if (!_imageInactive || !_imageInactive->_filename) Stack->PushNULL();
		else Stack->PushString(_imageInactive->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInactiveImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_imageInactive) Stack->PushNULL();
		else Stack->PushNative(_imageInactive, true);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Close") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(SUCCEEDED(Close()));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoExclusive") == 0) {
		Stack->CorrectParams(0);
		GoExclusive();
		Script->WaitFor(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoSystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoSystemExclusive") == 0) {
		Stack->CorrectParams(0);
		GoSystemExclusive();
		Script->WaitFor(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Center
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Center") == 0) {
		Stack->CorrectParams(0);
		_posX = (Game->_renderer->_width - _width) / 2;
		_posY = (Game->_renderer->_height - _height) / 2;
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadFromFile
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadFromFile") == 0) {
		Stack->CorrectParams(1);

		CScValue *Val = Stack->Pop();
		Cleanup();
		if (!Val->IsNULL()) {
			Stack->PushBool(SUCCEEDED(LoadFile(Val->GetString())));
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateButton") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIButton *Btn = new CUIButton(Game);
		if (!Val->IsNULL()) Btn->SetName(Val->GetString());
		Stack->PushNative(Btn, true);

		Btn->_parent = this;
		_widgets.Add(Btn);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateStatic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateStatic") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIText *Sta = new CUIText(Game);
		if (!Val->IsNULL()) Sta->SetName(Val->GetString());
		Stack->PushNative(Sta, true);

		Sta->_parent = this;
		_widgets.Add(Sta);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEditor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateEditor") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIEdit *Edi = new CUIEdit(Game);
		if (!Val->IsNULL()) Edi->SetName(Val->GetString());
		Stack->PushNative(Edi, true);

		Edi->_parent = this;
		_widgets.Add(Edi);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateWindow") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIWindow *Win = new CUIWindow(Game);
		if (!Val->IsNULL()) Win->SetName(Val->GetString());
		Stack->PushNative(Win, true);

		Win->_parent = this;
		_widgets.Add(Win);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteControl / DeleteButton / DeleteStatic / DeleteEditor / DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteControl") == 0 || strcmp(Name, "DeleteButton") == 0 || strcmp(Name, "DeleteStatic") == 0 || strcmp(Name, "DeleteEditor") == 0 || strcmp(Name, "DeleteWindow") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		CUIObject *obj = (CUIObject *)val->GetNative();

		for (int i = 0; i < _widgets.GetSize(); i++) {
			if (_widgets[i] == obj) {
				delete _widgets[i];
				_widgets.RemoveAt(i);
				if (val->GetType() == VAL_VARIABLE_REF) val->SetNULL();
			}
		}
		Stack->PushNULL();
		return S_OK;
	} else if SUCCEEDED(Game->WindowScriptMethodHook(this, Script, Stack, Name)) return S_OK;

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIWindow::ScGetProperty(char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("window");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWidgets / NumControls (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumWidgets") == 0 || strcmp(Name, "NumControls") == 0) {
		_scValue->SetInt(_widgets.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Exclusive") == 0) {
		_scValue->SetBool(_mode == WINDOW_EXCLUSIVE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SystemExclusive") == 0) {
		_scValue->SetBool(_mode == WINDOW_SYSTEM_EXCLUSIVE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Menu") == 0) {
		_scValue->SetBool(_isMenu);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InGame") == 0) {
		_scValue->SetBool(_inGame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseMusic") == 0) {
		_scValue->SetBool(_pauseMusic);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClipContents") == 0) {
		_scValue->SetBool(_clipContents);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Transparent") == 0) {
		_scValue->SetBool(_transparent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeColor") == 0) {
		_scValue->SetInt((int)_fadeColor);
		return _scValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Menu") == 0) {
		_isMenu = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InGame") == 0) {
		_inGame = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseMusic") == 0) {
		_pauseMusic = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClipContents") == 0) {
		_clipContents = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Transparent") == 0) {
		_transparent = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeColor") == 0) {
		_fadeColor = (uint32)Value->GetInt();
		_fadeBackground = (_fadeColor != 0);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Exclusive") == 0) {
		if (Value->GetBool())
			GoExclusive();
		else {
			Close();
			_visible = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SystemExclusive") == 0) {
		if (Value->GetBool())
			GoSystemExclusive();
		else {
			Close();
			_visible = true;
		}
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CUIWindow::ScToString() {
	return "[window]";
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::HandleKeypress(SDL_Event *event) {
//TODO
#if 0
	if (event->type == SDL_KEYDOWN && event->key.keysym.scancode == SDL_SCANCODE_TAB) {
		return SUCCEEDED(MoveFocus(!CBKeyboardState::IsShiftDown()));
	} else {
		if (_focusedWidget) return _focusedWidget->HandleKeypress(event);
		else return false;
	}
#endif
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::HandleMouseWheel(int Delta) {
	if (_focusedWidget) return _focusedWidget->HandleMouseWheel(Delta);
	else return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::HandleMouse(TMouseEvent Event, TMouseButton Button) {
	HRESULT res = CUIObject::HandleMouse(Event, Button);

	// handle window dragging
	if (!CBPlatform::IsRectEmpty(&_dragRect)) {
		// start drag
		if (Event == MOUSE_CLICK && Button == MOUSE_BUTTON_LEFT) {
			RECT DragRect = _dragRect;
			int OffsetX, OffsetY;
			GetTotalOffset(&OffsetX, &OffsetY);
			CBPlatform::OffsetRect(&DragRect, _posX + OffsetX, _posY + OffsetY);

			if (CBPlatform::PtInRect(&DragRect, Game->_mousePos)) {
				_dragFrom.x = Game->_mousePos.x;
				_dragFrom.y = Game->_mousePos.y;
				_dragging = true;
			}
		}
		// end drag
		else if (_dragging && Event == MOUSE_RELEASE && Button == MOUSE_BUTTON_LEFT) {
			_dragging = false;
		}
	}

	return res;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_backInactive));
	PersistMgr->Transfer(TMEMBER(_clipContents));
	PersistMgr->Transfer(TMEMBER(_dragFrom));
	PersistMgr->Transfer(TMEMBER(_dragging));
	PersistMgr->Transfer(TMEMBER(_dragRect));
	PersistMgr->Transfer(TMEMBER(_fadeBackground));
	PersistMgr->Transfer(TMEMBER(_fadeColor));
	PersistMgr->Transfer(TMEMBER(_fontInactive));
	PersistMgr->Transfer(TMEMBER(_imageInactive));
	PersistMgr->Transfer(TMEMBER(_inGame));
	PersistMgr->Transfer(TMEMBER(_isMenu));
	PersistMgr->Transfer(TMEMBER_INT(_mode));
	PersistMgr->Transfer(TMEMBER(_shieldButton));
	PersistMgr->Transfer(TMEMBER(_shieldWindow));
	PersistMgr->Transfer(TMEMBER_INT(_titleAlign));
	PersistMgr->Transfer(TMEMBER(_titleRect));
	PersistMgr->Transfer(TMEMBER(_transparent));
	PersistMgr->Transfer(TMEMBER(_viewport));
	PersistMgr->Transfer(TMEMBER(_pauseMusic));

	_widgets.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::MoveFocus(bool Forward) {
	int i;
	bool found = false;
	for (i = 0; i < _widgets.GetSize(); i++) {
		if (_widgets[i] == _focusedWidget) {
			found = true;
			break;
		}
	}
	if (!found) _focusedWidget = NULL;

	if (!_focusedWidget) {
		if (_widgets.GetSize() > 0) i = 0;
		else return S_OK;
	}

	int NumTries = 0;
	bool done = false;

	while (NumTries <= _widgets.GetSize()) {
		if (_widgets[i] != _focusedWidget && _widgets[i]->_canFocus && _widgets[i]->_visible && !_widgets[i]->_disable) {
			_focusedWidget = _widgets[i];
			done = true;
			break;
		}

		if (Forward) {
			i++;
			if (i >= _widgets.GetSize()) i = 0;
		} else {
			i--;
			if (i < 0) i = _widgets.GetSize() - 1;
		}
		NumTries++;
	}

	return done ? S_OK : E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GoExclusive() {
	if (_mode == WINDOW_EXCLUSIVE) return S_OK;

	if (_mode == WINDOW_NORMAL) {
		_ready = false;
		_mode = WINDOW_EXCLUSIVE;
		_visible = true;
		_disable = false;
		Game->FocusWindow(this);
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GoSystemExclusive() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) return S_OK;

	MakeFreezable(false);

	_mode = WINDOW_SYSTEM_EXCLUSIVE;
	_ready = false;
	_visible = true;
	_disable = false;
	Game->FocusWindow(this);

	Game->Freeze(_pauseMusic);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Close() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) {
		Game->Unfreeze();
	}

	_mode = WINDOW_NORMAL;
	_visible = false;
	_ready = true;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "close") == 0) Close();
		else return CBObject::Listen(param1, param2);
		break;
	default:
		return CBObject::Listen(param1, param2);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::MakeFreezable(bool Freezable) {
	for (int i = 0; i < _widgets.GetSize(); i++)
		_widgets[i]->MakeFreezable(Freezable);

	CBObject::MakeFreezable(Freezable);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GetWindowObjects(CBArray<CUIObject *, CUIObject *>& Objects, bool InteractiveOnly) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		CUIObject *Control = _widgets[i];
		if (Control->_disable && InteractiveOnly) continue;

		switch (Control->_type) {
		case UI_WINDOW:
			((CUIWindow *)Control)->GetWindowObjects(Objects, InteractiveOnly);
			break;

		case UI_BUTTON:
		case UI_EDIT:
			Objects.Add(Control);
			break;

		default:
			if (!InteractiveOnly) Objects.Add(Control);
		}
	}
	return S_OK;
}

} // end of namespace WinterMute
