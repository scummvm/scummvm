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
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BActiveRect.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BKeyboardState.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/UI/UIButton.h"
#include "engines/wintermute/UI/UIEdit.h"
#include "engines/wintermute/UI/UIText.h"
#include "engines/wintermute/UI/UITiledImage.h"
#include "engines/wintermute/Base/BViewport.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
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
	close();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::cleanup() {
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
HRESULT CUIWindow::display(int OffsetX, int OffsetY) {
	// go exclusive
	if (_mode == WINDOW_EXCLUSIVE || _mode == WINDOW_SYSTEM_EXCLUSIVE) {
		if (!_shieldWindow) _shieldWindow = new CUIWindow(Game);
		if (_shieldWindow) {
			_shieldWindow->_posX = _shieldWindow->_posY = 0;
			_shieldWindow->_width = Game->_renderer->_width;
			_shieldWindow->_height = Game->_renderer->_height;

			_shieldWindow->display();
		}
	} else if (_isMenu) {
		if (!_shieldButton) {
			_shieldButton = new CUIButton(Game);
			_shieldButton->setName("close");
			_shieldButton->setListener(this, _shieldButton, 0);
			_shieldButton->_parent = this;
		}
		if (_shieldButton) {
			_shieldButton->_posX = _shieldButton->_posY = 0;
			_shieldButton->_width = Game->_renderer->_width;
			_shieldButton->_height = Game->_renderer->_height;

			_shieldButton->display();
		}
	}

	if (!_visible) return S_OK;

	if (_fadeBackground) Game->_renderer->fadeToColor(_fadeColor);

	if (_dragging) {
		_posX += (Game->_mousePos.x - _dragFrom.x);
		_posY += (Game->_mousePos.y - _dragFrom.y);

		_dragFrom.x = Game->_mousePos.x;
		_dragFrom.y = Game->_mousePos.y;
	}

	if (!_focusedWidget || (!_focusedWidget->_canFocus || _focusedWidget->_disable || !_focusedWidget->_visible)) {
		moveFocus();
	}

	bool PopViewport = false;
	if (_clipContents) {
		if (!_viewport) _viewport = new CBViewport(Game);
		if (_viewport) {
			_viewport->setRect(_posX + OffsetX, _posY + OffsetY, _posX + _width + OffsetX, _posY + _height + OffsetY);
			Game->PushViewport(_viewport);
			PopViewport = true;
		}
	}


	CUITiledImage *back = _back;
	CBSprite *image = _image;
	CBFont *font = _font;

	if (!isFocused()) {
		if (_backInactive) back = _backInactive;
		if (_imageInactive) image = _imageInactive;
		if (_fontInactive) font = _fontInactive;
	}

	if (_alphaColor != 0) Game->_renderer->_forceAlphaColor = _alphaColor;
	if (back) back->display(_posX + OffsetX, _posY + OffsetY, _width, _height);
	if (image) image->Draw(_posX + OffsetX, _posY + OffsetY, _transparent ? NULL : this);

	if (!CBPlatform::IsRectEmpty(&_titleRect) && font && _text) {
		font->drawText((byte *)_text, _posX + OffsetX + _titleRect.left, _posY + OffsetY + _titleRect.top, _titleRect.right - _titleRect.left, _titleAlign, _titleRect.bottom - _titleRect.top);
	}

	if (!_transparent && !image) Game->_renderer->_rectList.Add(new CBActiveRect(Game, this, NULL, _posX + OffsetX, _posY + OffsetY, _width, _height, 100, 100, false));

	for (int i = 0; i < _widgets.GetSize(); i++) {
		_widgets[i]->display(_posX + OffsetX, _posY + OffsetY);
	}

	if (_alphaColor != 0) Game->_renderer->_forceAlphaColor = 0;

	if (PopViewport) Game->PopViewport();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::loadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->readWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIWindow::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = loadBuffer(Buffer, true))) Game->LOG(0, "Error parsing WINDOW file '%s'", Filename);

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
HRESULT CUIWindow::loadBuffer(byte  *Buffer, bool Complete) {
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
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_BACK:
			delete _back;
			_back = new CUITiledImage(Game);
			if (!_back || FAILED(_back->loadFile((char *)params))) {
				delete _back;
				_back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_INACTIVE:
			delete _backInactive;
			_backInactive = new CUITiledImage(Game);
			if (!_backInactive || FAILED(_backInactive->loadFile((char *)params))) {
				delete _backInactive;
				_backInactive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSprite(Game);
			if (!_image || FAILED(_image->loadFile((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_INACTIVE:
			delete _imageInactive,
			       _imageInactive = new CBSprite(Game);
			if (!_imageInactive || FAILED(_imageInactive->loadFile((char *)params))) {
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
			setText((char *)params);
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
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BUTTON: {
			CUIButton *btn = new CUIButton(Game);
			if (!btn || FAILED(btn->loadBuffer(params, false))) {
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
			if (!text || FAILED(text->loadBuffer(params, false))) {
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
			if (!edit || FAILED(edit->loadBuffer(params, false))) {
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
			if (!win || FAILED(win->loadBuffer(params, false))) {
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
			addScript((char *)params);
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
			parseEditorProperty(params, false);
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

	correctSize();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = DRGBA(ar, ag, ab, alpha);

	if (_fadeBackground) _fadeColor = DRGBA(FadeR, FadeG, FadeB, FadeA);

	_focusedWidget = NULL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::saveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->putTextIndent(Indent, "WINDOW\n");
	Buffer->putTextIndent(Indent, "{\n");

	Buffer->putTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->putTextIndent(Indent + 2, "CAPTION=\"%s\"\n", getCaption());

	Buffer->putTextIndent(Indent + 2, "\n");

	if (_back && _back->_filename)
		Buffer->putTextIndent(Indent + 2, "BACK=\"%s\"\n", _back->_filename);
	if (_backInactive && _backInactive->_filename)
		Buffer->putTextIndent(Indent + 2, "BACK_INACTIVE=\"%s\"\n", _backInactive->_filename);

	if (_image && _image->_filename)
		Buffer->putTextIndent(Indent + 2, "IMAGE=\"%s\"\n", _image->_filename);
	if (_imageInactive && _imageInactive->_filename)
		Buffer->putTextIndent(Indent + 2, "IMAGE_INACTIVE=\"%s\"\n", _imageInactive->_filename);

	if (_font && _font->_filename)
		Buffer->putTextIndent(Indent + 2, "FONT=\"%s\"\n", _font->_filename);
	if (_fontInactive && _fontInactive->_filename)
		Buffer->putTextIndent(Indent + 2, "FONT_INACTIVE=\"%s\"\n", _fontInactive->_filename);

	if (_cursor && _cursor->_filename)
		Buffer->putTextIndent(Indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	Buffer->putTextIndent(Indent + 2, "\n");

	if (_text)
		Buffer->putTextIndent(Indent + 2, "TITLE=\"%s\"\n", _text);

	switch (_titleAlign) {
	case TAL_LEFT:
		Buffer->putTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->putTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->putTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("UIWindow::SaveAsText - Unhandled enum-value NUM_TEXT_ALIGN");
	}

	if (!CBPlatform::IsRectEmpty(&_titleRect)) {
		Buffer->putTextIndent(Indent + 2, "TITLE_RECT { %d, %d, %d, %d }\n", _titleRect.left, _titleRect.top, _titleRect.right, _titleRect.bottom);
	}

	if (!CBPlatform::IsRectEmpty(&_dragRect)) {
		Buffer->putTextIndent(Indent + 2, "DRAG_RECT { %d, %d, %d, %d }\n", _dragRect.left, _dragRect.top, _dragRect.right, _dragRect.bottom);
	}

	Buffer->putTextIndent(Indent + 2, "\n");

	Buffer->putTextIndent(Indent + 2, "X=%d\n", _posX);
	Buffer->putTextIndent(Indent + 2, "Y=%d\n", _posY);
	Buffer->putTextIndent(Indent + 2, "WIDTH=%d\n", _width);
	Buffer->putTextIndent(Indent + 2, "HEIGHT=%d\n", _height);

	Buffer->putTextIndent(Indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	Buffer->putTextIndent(Indent + 2, "TRANSPARENT=%s\n", _transparent ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "PAUSE_MUSIC=%s\n", _pauseMusic ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "MENU=%s\n", _isMenu ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "IN_GAME=%s\n", _inGame ? "TRUE" : "FALSE");
	Buffer->putTextIndent(Indent + 2, "CLIP_CONTENTS=%s\n", _clipContents ? "TRUE" : "FALSE");

	Buffer->putTextIndent(Indent + 2, "\n");

	if (_fadeBackground) {
		Buffer->putTextIndent(Indent + 2, "FADE_COLOR { %d, %d, %d }\n", D3DCOLGetR(_fadeColor), D3DCOLGetG(_fadeColor), D3DCOLGetB(_fadeColor));
		Buffer->putTextIndent(Indent + 2, "FADE_ALPHA=%d\n", D3DCOLGetA(_fadeColor));
	}

	Buffer->putTextIndent(Indent + 2, "ALPHA_COLOR { %d, %d, %d }\n", D3DCOLGetR(_alphaColor), D3DCOLGetG(_alphaColor), D3DCOLGetB(_alphaColor));
	Buffer->putTextIndent(Indent + 2, "ALPHA=%d\n", D3DCOLGetA(_alphaColor));

	Buffer->putTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < _scripts.GetSize(); i++) {
		Buffer->putTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	Buffer->putTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(Buffer, Indent + 2);

	// controls
	for (int i = 0; i < _widgets.GetSize(); i++)
		_widgets[i]->saveAsText(Buffer, Indent + 2);


	Buffer->putTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::enableWidget(const char *name, bool Enable) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, name) == 0) _widgets[i]->_disable = !Enable;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::showWidget(const char *name, bool Visible) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, name) == 0) _widgets[i]->_visible = Visible;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetWidget / GetControl
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetWidget") == 0 || strcmp(name, "GetControl") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		if (val->getType() == VAL_INT) {
			int widget = val->getInt();
			if (widget < 0 || widget >= _widgets.GetSize()) stack->pushNULL();
			else stack->pushNative(_widgets[widget], true);
		} else {
			for (int i = 0; i < _widgets.GetSize(); i++) {
				if (scumm_stricmp(_widgets[i]->_name, val->getString()) == 0) {
					stack->pushNative(_widgets[i], true);
					return S_OK;
				}
			}
			stack->pushNULL();
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInactiveFont") == 0) {
		stack->correctParams(1);

		if (_fontInactive) Game->_fontStorage->RemoveFont(_fontInactive);
		_fontInactive = Game->_fontStorage->AddFont(stack->pop()->getString());
		stack->pushBool(_fontInactive != NULL);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInactiveImage") == 0) {
		stack->correctParams(1);

		delete _imageInactive;
		_imageInactive = new CBSprite(Game);
		const char *Filename = stack->pop()->getString();
		if (!_imageInactive || FAILED(_imageInactive->loadFile(Filename))) {
			delete _imageInactive;
			_imageInactive = NULL;
			stack->pushBool(false);
		} else stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInactiveImage") == 0) {
		stack->correctParams(0);
		if (!_imageInactive || !_imageInactive->_filename) stack->pushNULL();
		else stack->pushString(_imageInactive->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInactiveImageObject") == 0) {
		stack->correctParams(0);
		if (!_imageInactive) stack->pushNULL();
		else stack->pushNative(_imageInactive, true);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Close") == 0) {
		stack->correctParams(0);
		stack->pushBool(SUCCEEDED(close()));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoExclusive") == 0) {
		stack->correctParams(0);
		goExclusive();
		script->WaitFor(this);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoSystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoSystemExclusive") == 0) {
		stack->correctParams(0);
		goSystemExclusive();
		script->WaitFor(this);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Center
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Center") == 0) {
		stack->correctParams(0);
		_posX = (Game->_renderer->_width - _width) / 2;
		_posY = (Game->_renderer->_height - _height) / 2;
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadFromFile
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadFromFile") == 0) {
		stack->correctParams(1);

		CScValue *Val = stack->pop();
		cleanup();
		if (!Val->isNULL()) {
			stack->pushBool(SUCCEEDED(loadFile(Val->getString())));
		} else stack->pushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateButton") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CUIButton *Btn = new CUIButton(Game);
		if (!Val->isNULL()) Btn->setName(Val->getString());
		stack->pushNative(Btn, true);

		Btn->_parent = this;
		_widgets.Add(Btn);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateStatic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateStatic") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CUIText *Sta = new CUIText(Game);
		if (!Val->isNULL()) Sta->setName(Val->getString());
		stack->pushNative(Sta, true);

		Sta->_parent = this;
		_widgets.Add(Sta);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEditor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEditor") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CUIEdit *Edi = new CUIEdit(Game);
		if (!Val->isNULL()) Edi->setName(Val->getString());
		stack->pushNative(Edi, true);

		Edi->_parent = this;
		_widgets.Add(Edi);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateWindow") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();

		CUIWindow *Win = new CUIWindow(Game);
		if (!Val->isNULL()) Win->setName(Val->getString());
		stack->pushNative(Win, true);

		Win->_parent = this;
		_widgets.Add(Win);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteControl / DeleteButton / DeleteStatic / DeleteEditor / DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteControl") == 0 || strcmp(name, "DeleteButton") == 0 || strcmp(name, "DeleteStatic") == 0 || strcmp(name, "DeleteEditor") == 0 || strcmp(name, "DeleteWindow") == 0) {
		stack->correctParams(1);
		CScValue *val = stack->pop();
		CUIObject *obj = (CUIObject *)val->getNative();

		for (int i = 0; i < _widgets.GetSize(); i++) {
			if (_widgets[i] == obj) {
				delete _widgets[i];
				_widgets.RemoveAt(i);
				if (val->getType() == VAL_VARIABLE_REF) val->setNULL();
			}
		}
		stack->pushNULL();
		return S_OK;
	} else if SUCCEEDED(Game->WindowScriptMethodHook(this, script, stack, name)) return S_OK;

	else return CUIObject::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIWindow::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("window");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWidgets / NumControls (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NumWidgets") == 0 || strcmp(name, "NumControls") == 0) {
		_scValue->setInt(_widgets.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Exclusive") == 0) {
		_scValue->setBool(_mode == WINDOW_EXCLUSIVE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SystemExclusive") == 0) {
		_scValue->setBool(_mode == WINDOW_SYSTEM_EXCLUSIVE);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Menu") == 0) {
		_scValue->setBool(_isMenu);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InGame") == 0) {
		_scValue->setBool(_inGame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseMusic") == 0) {
		_scValue->setBool(_pauseMusic);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClipContents") == 0) {
		_scValue->setBool(_clipContents);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Transparent") == 0) {
		_scValue->setBool(_transparent);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeColor") == 0) {
		_scValue->setInt((int)_fadeColor);
		return _scValue;
	}

	else return CUIObject::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Menu") == 0) {
		_isMenu = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InGame") == 0) {
		_inGame = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseMusic") == 0) {
		_pauseMusic = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClipContents") == 0) {
		_clipContents = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Transparent") == 0) {
		_transparent = value->getBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeColor") == 0) {
		_fadeColor = (uint32)value->getInt();
		_fadeBackground = (_fadeColor != 0);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Exclusive") == 0) {
		if (value->getBool())
			goExclusive();
		else {
			close();
			_visible = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SystemExclusive") == 0) {
		if (value->getBool())
			goSystemExclusive();
		else {
			close();
			_visible = true;
		}
		return S_OK;
	}

	else return CUIObject::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIWindow::scToString() {
	return "[window]";
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::handleKeypress(Common::Event *event, bool printable) {
//TODO
	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_TAB) {
		return SUCCEEDED(moveFocus(!CBKeyboardState::IsShiftDown()));
	} else {
		if (_focusedWidget) return _focusedWidget->handleKeypress(event, printable);
		else return false;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::handleMouseWheel(int Delta) {
	if (_focusedWidget) return _focusedWidget->handleMouseWheel(Delta);
	else return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::handleMouse(TMouseEvent Event, TMouseButton Button) {
	HRESULT res = CUIObject::handleMouse(Event, Button);

	// handle window dragging
	if (!CBPlatform::IsRectEmpty(&_dragRect)) {
		// start drag
		if (Event == MOUSE_CLICK && Button == MOUSE_BUTTON_LEFT) {
			RECT DragRect = _dragRect;
			int OffsetX, OffsetY;
			getTotalOffset(&OffsetX, &OffsetY);
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
HRESULT CUIWindow::persist(CBPersistMgr *persistMgr) {

	CUIObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_backInactive));
	persistMgr->transfer(TMEMBER(_clipContents));
	persistMgr->transfer(TMEMBER(_dragFrom));
	persistMgr->transfer(TMEMBER(_dragging));
	persistMgr->transfer(TMEMBER(_dragRect));
	persistMgr->transfer(TMEMBER(_fadeBackground));
	persistMgr->transfer(TMEMBER(_fadeColor));
	persistMgr->transfer(TMEMBER(_fontInactive));
	persistMgr->transfer(TMEMBER(_imageInactive));
	persistMgr->transfer(TMEMBER(_inGame));
	persistMgr->transfer(TMEMBER(_isMenu));
	persistMgr->transfer(TMEMBER_INT(_mode));
	persistMgr->transfer(TMEMBER(_shieldButton));
	persistMgr->transfer(TMEMBER(_shieldWindow));
	persistMgr->transfer(TMEMBER_INT(_titleAlign));
	persistMgr->transfer(TMEMBER(_titleRect));
	persistMgr->transfer(TMEMBER(_transparent));
	persistMgr->transfer(TMEMBER(_viewport));
	persistMgr->transfer(TMEMBER(_pauseMusic));

	_widgets.persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::moveFocus(bool Forward) {
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
HRESULT CUIWindow::goExclusive() {
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
HRESULT CUIWindow::goSystemExclusive() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) return S_OK;

	makeFreezable(false);

	_mode = WINDOW_SYSTEM_EXCLUSIVE;
	_ready = false;
	_visible = true;
	_disable = false;
	Game->FocusWindow(this);

	Game->Freeze(_pauseMusic);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::close() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) {
		Game->Unfreeze();
	}

	_mode = WINDOW_NORMAL;
	_visible = false;
	_ready = true;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "close") == 0) close();
		else return CBObject::listen(param1, param2);
		break;
	default:
		return CBObject::listen(param1, param2);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::makeFreezable(bool Freezable) {
	for (int i = 0; i < _widgets.GetSize(); i++)
		_widgets[i]->makeFreezable(Freezable);

	CBObject::makeFreezable(Freezable);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::getWindowObjects(CBArray<CUIObject *, CUIObject *> &Objects, bool InteractiveOnly) {
	for (int i = 0; i < _widgets.GetSize(); i++) {
		CUIObject *Control = _widgets[i];
		if (Control->_disable && InteractiveOnly) continue;

		switch (Control->_type) {
		case UI_WINDOW:
			((CUIWindow *)Control)->getWindowObjects(Objects, InteractiveOnly);
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
