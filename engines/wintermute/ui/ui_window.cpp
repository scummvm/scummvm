/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_keyboard_state.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_edit.h"
#include "engines/wintermute/ui/ui_text.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/base/base_viewport.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIWindow, false)

//////////////////////////////////////////////////////////////////////////
UIWindow::UIWindow(BaseGame *inGame) : UIObject(inGame) {
	BasePlatform::setRectEmpty(&_titleRect);
	BasePlatform::setRectEmpty(&_dragRect);
	_titleAlign = TAL_LEFT;
	_transparent = false;

	_backInactive = nullptr;
	_fontInactive = nullptr;
	_imageInactive = nullptr;

	_type = UI_WINDOW;
	_canFocus = true;

	_dragging = false;
	_dragFrom.x = _dragFrom.y = 0;

	_mode = WINDOW_NORMAL;
	_shieldWindow = nullptr;
	_shieldButton = nullptr;

	_fadeColor = 0x00000000;
	_fadeBackground = false;

	_ready = true;
	_isMenu = false;
	_inGame = false;

	_clipContents = false;
	_viewport = nullptr;

	_pauseMusic = true;
}


//////////////////////////////////////////////////////////////////////////
UIWindow::~UIWindow() {
	close();
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void UIWindow::cleanup() {
	SAFE_DELETE(_shieldWindow);
	SAFE_DELETE(_shieldButton);
	SAFE_DELETE(_viewport);
	delete _backInactive;
	if (!_sharedFonts && _fontInactive) {
		_game->_fontStorage->removeFont(_fontInactive);
	}
	if (!_sharedImages && _imageInactive) {
		delete _imageInactive;
	}

	for (int32 i = 0; i < _widgets.getSize(); i++) {
		delete _widgets[i];
	}
	_widgets.removeAll();
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::display(int offsetX, int offsetY) {
	// go exclusive
	if (_mode == WINDOW_EXCLUSIVE || _mode == WINDOW_SYSTEM_EXCLUSIVE) {
		if (!_shieldWindow) {
			_shieldWindow = new UIWindow(_game);
		}
		if (_shieldWindow) {
			_shieldWindow->_posX = _shieldWindow->_posY = 0;
			_shieldWindow->_width = _game->_renderer->getWidth();
			_shieldWindow->_height = _game->_renderer->getHeight();

			_shieldWindow->display();
		}
	} else if (_isMenu) {
		if (!_shieldButton) {
			_shieldButton = new UIButton(_game);
			_shieldButton->setName("close");
			_shieldButton->setListener(this, _shieldButton, 0);
			_shieldButton->_parent = this;
		}
		if (_shieldButton) {
			_shieldButton->_posX = _shieldButton->_posY = 0;
			_shieldButton->_width = _game->_renderer->getWidth();
			_shieldButton->_height = _game->_renderer->getHeight();

			_shieldButton->display();
		}
	}

	if (!_visible) {
		return STATUS_OK;
	}

	if (_fadeBackground) {
		byte fadeR = RGBCOLGetR(_fadeColor);
		byte fadeG = RGBCOLGetG(_fadeColor);
		byte fadeB = RGBCOLGetB(_fadeColor);
		byte fadeA = RGBCOLGetA(_fadeColor);
		_game->_renderer->fadeToColor(fadeR, fadeG, fadeB, fadeA);
	}

	if (_dragging) {
		_posX += (_game->_mousePos.x - _dragFrom.x);
		_posY += (_game->_mousePos.y - _dragFrom.y);

		_dragFrom.x = _game->_mousePos.x;
		_dragFrom.y = _game->_mousePos.y;
	}

	if (!_focusedWidget || (!_focusedWidget->_canFocus || _focusedWidget->_disable || !_focusedWidget->_visible)) {
		moveFocus();
	}

	bool popViewport = false;
	if (_clipContents) {
		if (!_viewport) {
			_viewport = new BaseViewport(_game);
		}
		if (_viewport) {
			_viewport->setRect(_posX + offsetX, _posY + offsetY, _posX + _width + offsetX, _posY + _height + offsetY);
			_game->pushViewport(_viewport);
			popViewport = true;
		}
	}


	UITiledImage *back = _back;
	BaseSprite *image = _image;
	BaseFont *font = _font;

	if (!isFocused()) {
		if (_backInactive) {
			back = _backInactive;
		}
		if (_imageInactive) {
			image = _imageInactive;
		}
		if (_fontInactive) {
			font = _fontInactive;
		}
	}

	if (_alphaColor != 0) {
		_game->_renderer->_forceAlphaColor = _alphaColor;
	}
	if (back) {
		back->display(_posX + offsetX, _posY + offsetY, _width, _height);
	}
	if (image) {
		image->draw(_posX + offsetX, _posY + offsetY, _transparent ? nullptr : this);
	}

	if (!BasePlatform::isRectEmpty(&_titleRect) && font && _text) {
		font->drawText((byte *)_text, _posX + offsetX + _titleRect.left, _posY + offsetY + _titleRect.top, _titleRect.right - _titleRect.left, _titleAlign, _titleRect.bottom - _titleRect.top);
	}

	if (!_transparent && !image) {
		_game->_renderer->_rectList.add(new BaseActiveRect(_game, this, nullptr, _posX + offsetX, _posY + offsetY, _width, _height, 100, 100, false));
	}

	for (int32 i = 0; i < _widgets.getSize(); i++) {
		_widgets[i]->display(_posX + offsetX, _posY + offsetY);
	}

	if (_alphaColor != 0) {
		_game->_renderer->_forceAlphaColor = 0;
	}

	if (popViewport) {
		_game->popViewport();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::loadFile(const char *filename) {
	char *buffer = (char *)_game->_fileManager->readWholeFile(filename);
	if (buffer == nullptr) {
		_game->LOG(0, "UIWindow::loadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_game->LOG(0, "Error parsing WINDOW file '%s'", filename);
	}

	delete[] buffer;

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
bool UIWindow::loadBuffer(char *buffer, bool complete) {
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

	char *params;
	int cmd = 2;
	BaseParser parser(_game);

	int fadeR = 0, fadeG = 0, fadeB = 0, fadeA = 0;
	int ar = 0, ag = 0, ab = 0, alpha = 0;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_WINDOW) {
			_game->LOG(0, "'WINDOW' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd >= PARSERR_TOKENNOTFOUND && (cmd = parser.getCommand(&buffer, commands, &params)) >= PARSERR_TOKENNOTFOUND) {
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
			SAFE_DELETE(_back);
			_back = new UITiledImage(_game);
			if (!_back || DID_FAIL(_back->loadFile(params))) {
				SAFE_DELETE(_back);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_INACTIVE:
			SAFE_DELETE(_backInactive);
			_backInactive = new UITiledImage(_game);
			if (!_backInactive || DID_FAIL(_backInactive->loadFile(params))) {
				SAFE_DELETE(_backInactive);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			SAFE_DELETE(_image);
			_image = new BaseSprite(_game);
			if (!_image || DID_FAIL(_image->loadFile(params))) {
				SAFE_DELETE(_image);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_INACTIVE:
			SAFE_DELETE(_imageInactive);
			_imageInactive = new BaseSprite(_game);
			if (!_imageInactive || DID_FAIL(_imageInactive->loadFile(params))) {
				SAFE_DELETE(_imageInactive);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (_font) {
				_game->_fontStorage->removeFont(_font);
			}
			_font = _game->_fontStorage->addFont(params);
			if (!_font) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT_INACTIVE:
			if (_fontInactive) {
				_game->_fontStorage->removeFont(_fontInactive);
			}
			_fontInactive = _game->_fontStorage->addFont(params);
			if (!_fontInactive) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_TITLE:
			setText(params);
			_game->_stringTable->expand(&_text);
			break;

		case TOKEN_TITLE_ALIGN:
			if (scumm_stricmp(params, "left") == 0) {
				_titleAlign = TAL_LEFT;
			} else if (scumm_stricmp(params, "right") == 0) {
				_titleAlign = TAL_RIGHT;
			} else {
				_titleAlign = TAL_CENTER;
			}
			break;

		case TOKEN_TITLE_RECT:
			parser.scanStr(params, "%d,%d,%d,%d", &_titleRect.left, &_titleRect.top, &_titleRect.right, &_titleRect.bottom);
			break;

		case TOKEN_DRAG_RECT:
			parser.scanStr(params, "%d,%d,%d,%d", &_dragRect.left, &_dragRect.top, &_dragRect.right, &_dragRect.bottom);
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
			SAFE_DELETE(_cursor);
			_cursor = new BaseSprite(_game);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				SAFE_DELETE(_cursor);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BUTTON: {
			UIButton *btn = new UIButton(_game);
			if (!btn || DID_FAIL(btn->loadBuffer(params, false))) {
				SAFE_DELETE(btn);
				cmd = PARSERR_GENERIC;
			} else {
				btn->_parent = this;
				_widgets.add(btn);
			}
		}
		break;

		case TOKEN_STATIC: {
			UIText *text = new UIText(_game);
			if (!text || DID_FAIL(text->loadBuffer(params, false))) {
				SAFE_DELETE(text);
				cmd = PARSERR_GENERIC;
			} else {
				text->_parent = this;
				_widgets.add(text);
			}
		}
		break;

		case TOKEN_EDIT: {
			UIEdit *edit = new UIEdit(_game);
			if (!edit || DID_FAIL(edit->loadBuffer(params, false))) {
				SAFE_DELETE(edit);
				cmd = PARSERR_GENERIC;
			} else {
				edit->_parent = this;
				_widgets.add(edit);
			}
		}
		break;

		case TOKEN_WINDOW: {
			UIWindow *win = new UIWindow(_game);
			if (!win || DID_FAIL(win->loadBuffer(params, false))) {
				SAFE_DELETE(win);
				cmd = PARSERR_GENERIC;
			} else {
				win->_parent = this;
				_widgets.add(win);
			}
		}
		break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%b", &_transparent);
			break;

		case TOKEN_SCRIPT:
			addScript(params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.scanStr(params, "%b", &_parentNotify);
			break;

		case TOKEN_PAUSE_MUSIC:
			parser.scanStr(params, "%b", &_pauseMusic);
			break;

		case TOKEN_DISABLED:
			parser.scanStr(params, "%b", &_disable);
			break;

		case TOKEN_VISIBLE:
			parser.scanStr(params, "%b", &_visible);
			break;

		case TOKEN_MENU:
			parser.scanStr(params, "%b", &_isMenu);
			break;

		case TOKEN_IN_GAME:
			parser.scanStr(params, "%b", &_inGame);
			break;

		case TOKEN_CLIP_CONTENTS:
			parser.scanStr(params, "%b", &_clipContents);
			break;

		case TOKEN_FADE_COLOR:
			parser.scanStr(params, "%d,%d,%d", &fadeR, &fadeG, &fadeB);
			_fadeBackground = true;
			break;

		case TOKEN_FADE_ALPHA:
			parser.scanStr(params, "%d", &fadeA);
			_fadeBackground = true;
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr(params, "%d", &alpha);
			break;

		default:
			if (DID_FAIL(_game->windowLoadHook(this, &buffer, &params))) {
				cmd = PARSERR_GENERIC;
			}
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in WINDOW definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_game->LOG(0, "Error loading WINDOW definition");
		return STATUS_FAILED;
	}

	// HACK: Increase window title height by 1 for "5 Lethal Demons" game
	// For some reason getFontHeight() is off-by-one comparing to height set in TITLE_RECT,
	// Which made text being bigger then title rect and drawing was skipped.
	if (BaseEngine::instance().getGameId() == "5ld" && !BasePlatform::isRectEmpty(&_titleRect) && _text) {
		_titleRect.bottom ++;
	}

	correctSize();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = BYTETORGBA(ar, ag, ab, alpha);

	if (_fadeBackground) {
		_fadeColor = BYTETORGBA(fadeR, fadeG, fadeB, fadeA);
	}

	_focusedWidget = nullptr;

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIWindow::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "WINDOW\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "\n");

	if (_back && _back->_filename) {
		buffer->putTextIndent(indent + 2, "BACK=\"%s\"\n", _back->_filename);
	}
	if (_backInactive && _backInactive->_filename) {
		buffer->putTextIndent(indent + 2, "BACK_INACTIVE=\"%s\"\n", _backInactive->_filename);
	}

	if (_image && _image->_filename) {
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->_filename);
	}
	if (_imageInactive && _imageInactive->_filename) {
		buffer->putTextIndent(indent + 2, "IMAGE_INACTIVE=\"%s\"\n", _imageInactive->_filename);
	}

	if (_font && _font->_filename) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);
	}
	if (_fontInactive && _fontInactive->_filename) {
		buffer->putTextIndent(indent + 2, "FONT_INACTIVE=\"%s\"\n", _fontInactive->_filename);
	}

	if (_cursor && _cursor->_filename) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	if (_text) {
		buffer->putTextIndent(indent + 2, "TITLE=\"%s\"\n", _text);
	}

	switch (_titleAlign) {
	case TAL_LEFT:
		buffer->putTextIndent(indent + 2, "TITLE_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		buffer->putTextIndent(indent + 2, "TITLE_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		buffer->putTextIndent(indent + 2, "TITLE_ALIGN=\"%s\"\n", "center");
		break;
	default:
		break;
	}

	if (!BasePlatform::isRectEmpty(&_titleRect)) {
		buffer->putTextIndent(indent + 2, "TITLE_RECT { %d, %d, %d, %d }\n", _titleRect.left, _titleRect.top, _titleRect.right, _titleRect.bottom);
	}

	if (!BasePlatform::isRectEmpty(&_dragRect)) {
		buffer->putTextIndent(indent + 2, "DRAG_RECT { %d, %d, %d, %d }\n", _dragRect.left, _dragRect.top, _dragRect.right, _dragRect.bottom);
	}

	buffer->putTextIndent(indent + 2, "\n");

	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "WIDTH=%d\n", _width);
	buffer->putTextIndent(indent + 2, "HEIGHT=%d\n", _height);

	buffer->putTextIndent(indent + 2, "DISABLED=%s\n", _disable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PARENT_NOTIFY=%s\n", _parentNotify ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "TRANSPARENT=%s\n", _transparent ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PAUSE_MUSIC=%s\n", _pauseMusic ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "MENU=%s\n", _isMenu ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "IN_GAME=%s\n", _inGame ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "CLIP_CONTENTS=%s\n", _clipContents ? "TRUE" : "FALSE");

	buffer->putTextIndent(indent + 2, "\n");

	if (_fadeBackground) {
		buffer->putTextIndent(indent + 2, "FADE_COLOR { %d, %d, %d }\n", RGBCOLGetR(_fadeColor), RGBCOLGetG(_fadeColor), RGBCOLGetB(_fadeColor));
		buffer->putTextIndent(indent + 2, "FADE_ALPHA=%d\n", RGBCOLGetA(_fadeColor));
	}

	buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d, %d, %d }\n", RGBCOLGetR(_alphaColor), RGBCOLGetG(_alphaColor), RGBCOLGetB(_alphaColor));
	buffer->putTextIndent(indent + 2, "ALPHA=%d\n", RGBCOLGetA(_alphaColor));

	buffer->putTextIndent(indent + 2, "\n");

	// scripts
	for (int32 i = 0; i < _scripts.getSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	// controls
	for (int32 i = 0; i < _widgets.getSize(); i++) {
		_widgets[i]->saveAsText(buffer, indent + 2);
	}


	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UIWindow::enableWidget(const char *name, bool enable) {
	for (int32 i = 0; i < _widgets.getSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, name) == 0) {
			_widgets[i]->_disable = !enable;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::showWidget(const char *name, bool visible) {
	for (int32 i = 0; i < _widgets.getSize(); i++) {
		if (scumm_stricmp(_widgets[i]->_name, name) == 0) {
			_widgets[i]->_visible = visible;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIWindow::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetWidget / GetControl
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetWidget") == 0 || strcmp(name, "GetControl") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (val->getType() == VAL_INT) {
			int widget = val->getInt();
			if (widget < 0 || widget >= _widgets.getSize()) {
				stack->pushNULL();
			} else {
				stack->pushNative(_widgets[widget], true);
			}
		} else {
			for (int32 i = 0; i < _widgets.getSize(); i++) {
				if (scumm_stricmp(_widgets[i]->_name, val->getString()) == 0) {
					stack->pushNative(_widgets[i], true);
					return STATUS_OK;
				}
			}
			stack->pushNULL();
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInactiveFont") == 0) {
		stack->correctParams(1);

		if (_fontInactive) {
			_game->_fontStorage->removeFont(_fontInactive);
		}
		_fontInactive = _game->_fontStorage->addFont(stack->pop()->getString());
		stack->pushBool(_fontInactive != nullptr);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetInactiveImage") == 0) {
		stack->correctParams(1);

		SAFE_DELETE(_imageInactive);
		_imageInactive = new BaseSprite(_game);
		const char *filename = stack->pop()->getString();
		if (!_imageInactive || DID_FAIL(_imageInactive->loadFile(filename))) {
			SAFE_DELETE(_imageInactive);
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInactiveImage") == 0) {
		stack->correctParams(0);
		if (!_imageInactive || !_imageInactive->_filename) {
			stack->pushNULL();
		} else {
			stack->pushString(_imageInactive->_filename);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetInactiveImageObject") == 0) {
		stack->correctParams(0);
		if (!_imageInactive) {
			stack->pushNULL();
		} else {
			stack->pushNative(_imageInactive, true);
		}

		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Close") == 0) {
		stack->correctParams(0);
		stack->pushBool(DID_SUCCEED(close()));
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoExclusive") == 0) {
		stack->correctParams(0);
		goExclusive();
		script->waitFor(this);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoSystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoSystemExclusive") == 0) {
		stack->correctParams(0);
		goSystemExclusive();
		script->waitFor(this);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Center
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Center") == 0) {
		stack->correctParams(0);
		_posX = (_game->_renderer->getWidth() - _width) / 2;
		_posY = (_game->_renderer->getHeight() - _height) / 2;
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadFromFile
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadFromFile") == 0) {
		stack->correctParams(1);

		ScValue *val = stack->pop();
		cleanup();
		if (!val->isNULL()) {
			stack->pushBool(DID_SUCCEED(loadFile(val->getString())));
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateButton") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIButton *btn = new UIButton(_game);
		if (!val->isNULL()) {
			btn->setName(val->getString());
		}
		stack->pushNative(btn, true);

		btn->_parent = this;
		_widgets.add(btn);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateStatic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateStatic") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIText *sta = new UIText(_game);
		if (!val->isNULL()) {
			sta->setName(val->getString());
		}
		stack->pushNative(sta, true);

		sta->_parent = this;
		_widgets.add(sta);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEditor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateEditor") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIEdit *edi = new UIEdit(_game);
		if (!val->isNULL()) {
			edi->setName(val->getString());
		}
		stack->pushNative(edi, true);

		edi->_parent = this;
		_widgets.add(edi);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateWindow") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		UIWindow *win = new UIWindow(_game);
		if (!val->isNULL()) {
			win->setName(val->getString());
		}
		stack->pushNative(win, true);

		win->_parent = this;
		_widgets.add(win);

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteControl / DeleteButton / DeleteStatic / DeleteEditor / DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteControl") == 0 || strcmp(name, "DeleteButton") == 0 || strcmp(name, "DeleteStatic") == 0 || strcmp(name, "DeleteEditor") == 0 || strcmp(name, "DeleteWindow") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		UIObject *obj = (UIObject *)val->getNative();

		for (int32 i = 0; i < _widgets.getSize(); i++) {
			if (_widgets[i] == obj) {
				delete _widgets[i];
				_widgets.removeAt(i);
				if (val->getType() == VAL_VARIABLE_REF) {
					val->setNULL();
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	} else if DID_SUCCEED(_game->windowScriptMethodHook(this, script, stack, name)) {
		return STATUS_OK;
	} else {
		return UIObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIWindow::scGetProperty(const char *name) {
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
		_scValue->setInt(_widgets.getSize());
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
	} else {
		return UIObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Menu") == 0) {
		_isMenu = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InGame") == 0) {
		_inGame = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseMusic") == 0) {
		_pauseMusic = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ClipContents") == 0) {
		_clipContents = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Transparent") == 0) {
		_transparent = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "FadeColor") == 0) {
		_fadeColor = (uint32)value->getInt();
		_fadeBackground = (_fadeColor != 0);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Exclusive") == 0) {
		if (value->getBool()) {
			goExclusive();
		} else {
			close();
			_visible = true;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SystemExclusive") == 0) {
		if (value->getBool()) {
			goSystemExclusive();
		} else {
			close();
			_visible = true;
		}
		return STATUS_OK;
	} else {
		return UIObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIWindow::scToString() {
	return "[window]";
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::handleKeypress(Common::Event *event, bool printable) {
//TODO
	if (event->type == Common::EVENT_KEYDOWN && event->kbd.keycode == Common::KEYCODE_TAB) {
		return DID_SUCCEED(moveFocus(!BaseKeyboardState::isShiftDown()));
	} else {
		if (_focusedWidget) {
			return _focusedWidget->handleKeypress(event, printable);
		} else {
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::handleMouseWheel(int32 delta) {
	if (_focusedWidget) {
		return _focusedWidget->handleMouseWheel(delta);
	} else {
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::handleMouse(TMouseEvent event, TMouseButton button) {
	bool res = UIObject::handleMouse(event, button);

	// handle window dragging
	if (!BasePlatform::isRectEmpty(&_dragRect)) {
		// start drag
		if (event == MOUSE_CLICK && button == MOUSE_BUTTON_LEFT) {
			Common::Rect32 dragRect = _dragRect;
			int offsetX, offsetY;
			getTotalOffset(&offsetX, &offsetY);
			BasePlatform::offsetRect(&dragRect, _posX + offsetX, _posY + offsetY);

			if (BasePlatform::ptInRect(&dragRect, _game->_mousePos)) {
				_dragFrom.x = _game->_mousePos.x;
				_dragFrom.y = _game->_mousePos.y;
				_dragging = true;
			}
		}
		// end drag
		else if (_dragging && event == MOUSE_RELEASE && button == MOUSE_BUTTON_LEFT) {
			_dragging = false;
		}
	}

	return res;
}



//////////////////////////////////////////////////////////////////////////
bool UIWindow::persist(BasePersistenceManager *persistMgr) {

	UIObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_backInactive));
	persistMgr->transferBool(TMEMBER(_clipContents));
	persistMgr->transferPoint32(TMEMBER(_dragFrom));
	persistMgr->transferBool(TMEMBER(_dragging));
	persistMgr->transferRect32(TMEMBER(_dragRect));
	persistMgr->transferBool(TMEMBER(_fadeBackground));
	persistMgr->transferUint32(TMEMBER(_fadeColor));
	persistMgr->transferPtr(TMEMBER_PTR(_fontInactive));
	persistMgr->transferPtr(TMEMBER_PTR(_imageInactive));
	persistMgr->transferBool(TMEMBER(_inGame));
	persistMgr->transferBool(TMEMBER(_isMenu));
	persistMgr->transferSint32(TMEMBER_INT(_mode));
	persistMgr->transferPtr(TMEMBER_PTR(_shieldButton));
	persistMgr->transferPtr(TMEMBER_PTR(_shieldWindow));
	persistMgr->transferSint32(TMEMBER_INT(_titleAlign));
	persistMgr->transferRect32(TMEMBER(_titleRect));
	persistMgr->transferBool(TMEMBER(_transparent));
	persistMgr->transferPtr(TMEMBER_PTR(_viewport));
	persistMgr->transferBool(TMEMBER(_pauseMusic));

	_widgets.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::moveFocus(bool forward) {
	int i;
	bool found = false;
	for (i = 0; i < _widgets.getSize(); i++) {
		if (_widgets[i] == _focusedWidget) {
			found = true;
			break;
		}
	}
	if (!found) {
		_focusedWidget = nullptr;
	}

	if (!_focusedWidget) {
		if (_widgets.getSize() > 0) {
			i = 0;
		} else {
			return STATUS_OK;
		}
	}

	int numTries = 0;
	bool done = false;

	while (numTries <= _widgets.getSize()) {
		if (_widgets[i] != _focusedWidget && _widgets[i]->_canFocus && _widgets[i]->_visible && !_widgets[i]->_disable) {
			_focusedWidget = _widgets[i];
			done = true;
			break;
		}

		if (forward) {
			i++;
			if (i >= _widgets.getSize()) {
				i = 0;
			}
		} else {
			i--;
			if (i < 0) {
				i = _widgets.getSize() - 1;
			}
		}
		numTries++;
	}

	return done ? STATUS_OK : STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::goExclusive() {
	if (_mode == WINDOW_EXCLUSIVE) {
		return STATUS_OK;
	}

	if (_mode == WINDOW_NORMAL) {
		//Game->m_AccessMgr->SetActiveObject();
		_ready = false;
		_mode = WINDOW_EXCLUSIVE;
		_visible = true;
		_disable = false;
		_game->focusWindow(this);
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::goSystemExclusive() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) {
		return STATUS_OK;
	}

	makeFreezable(false);

	//Game->m_AccessMgr->SetActiveObject();
	_mode = WINDOW_SYSTEM_EXCLUSIVE;
	_ready = false;
	_visible = true;
	_disable = false;
	_game->focusWindow(this);

	_game->freeze(_pauseMusic);
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::close() {
	if (_mode == WINDOW_SYSTEM_EXCLUSIVE) {
		_game->unfreeze();
	}

	_mode = WINDOW_NORMAL;
	_visible = false;
	_ready = true;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIWindow::listen(BaseScriptHolder *param1, uint32 param2) {
	UIObject *obj = (UIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "close") == 0) {
			close();
		} else {
			return BaseObject::listen(param1, param2);
		}
		break;
	default:
		return BaseObject::listen(param1, param2);
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void UIWindow::makeFreezable(bool freezable) {
	for (int32 i = 0; i < _widgets.getSize(); i++) {
		_widgets[i]->makeFreezable(freezable);
	}

	BaseObject::makeFreezable(freezable);
}

//////////////////////////////////////////////////////////////////////////
/*CBObject *CUIWindow::GetNextAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetWindowObjects(Objects, true);

	if (Objects.GetSize() == 0)
		return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = 0; i < Objects.GetSize(); i++) {
				if (Objects[i] == CurrObject) {
					if (i < Objects.GetSize() - 1)
						return Objects[i + 1];
					else
						break;
				}
			}
		}
		return Objects[0];
	}
	return NULL;
}*/

//////////////////////////////////////////////////////////////////////////
/*CBObject *CUIWindow::GetPrevAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetWindowObjects(Objects, true);

	if (Objects.GetSize() == 0)
		return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = Objects.GetSize() - 1; i >= 0; i--) {
				if (Objects[i] == CurrObject) {
					if (i > 0)
						return Objects[i - 1];
					else
						break;
				}
			}
		}
		return Objects[Objects.GetSize() - 1];
	}
	return NULL;
}*/

//////////////////////////////////////////////////////////////////////////
bool UIWindow::getWindowObjects(BaseArray<UIObject *> &objects, bool interactiveOnly) {
	for (int32 i = 0; i < _widgets.getSize(); i++) {
		UIObject *control = _widgets[i];
		if (control->_disable && interactiveOnly) {
			continue;
		}

		switch (control->_type) {
		case UI_WINDOW:
			((UIWindow *)control)->getWindowObjects(objects, interactiveOnly);
			break;

		case UI_BUTTON:
		case UI_EDIT:
			objects.add(control);
			break;

		default:
			if (!interactiveOnly) {
				objects.add(control);
			}
		}
	}
	return STATUS_OK;
}

} // End of namespace Wintermute
