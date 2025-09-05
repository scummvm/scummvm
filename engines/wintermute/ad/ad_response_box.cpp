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

#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_response.h"
#include "engines/wintermute/ad/ad_response_box.h"
#include "engines/wintermute/base/base_engine.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/base/font/base_font.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/ui/ui_button.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/wintermute.h"
#include "engines/wintermute/dcgf.h"

#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdResponseBox, false)

//////////////////////////////////////////////////////////////////////////
AdResponseBox::AdResponseBox(BaseGame *inGame) : BaseObject(inGame) {
	_font = _fontHover = nullptr;

	_window = nullptr;
	_shieldWindow = new UIWindow(_game);

	_horizontal = false;
	BasePlatform::setRectEmpty(&_responseArea);
	_scrollOffset = 0;
	_spacing = 0;

	_waitingScript = nullptr;
	_lastResponseText = nullptr;
	_lastResponseTextOrig = nullptr;

	_verticalAlign = VAL_BOTTOM;
	_align = TAL_LEFT;
}


//////////////////////////////////////////////////////////////////////////
AdResponseBox::~AdResponseBox() {
	SAFE_DELETE(_window);
	SAFE_DELETE(_shieldWindow);
	SAFE_DELETE_ARRAY(_lastResponseText);
	SAFE_DELETE_ARRAY(_lastResponseTextOrig);

	if (_font) {
		_game->_fontStorage->removeFont(_font);
	}
	if (_fontHover) {
		_game->_fontStorage->removeFont(_fontHover);
	}

	clearResponses();
	clearButtons();

	_waitingScript = nullptr;
}

//////////////////////////////////////////////////////////////////////////
void AdResponseBox::clearResponses() {
	for (int32 i = 0; i < _responses.getSize(); i++) {
		delete _responses[i];
	}
	_responses.removeAll();
}


//////////////////////////////////////////////////////////////////////////
void AdResponseBox::clearButtons() {
	for (int32 i = 0; i < _respButtons.getSize(); i++) {
		delete _respButtons[i];
	}
	_respButtons.removeAll();
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::invalidateButtons() {
	for (int32 i = 0; i < _respButtons.getSize(); i++) {
		_respButtons[i]->_image = nullptr;
		_respButtons[i]->_cursor = nullptr;
		_respButtons[i]->_font = nullptr;
		_respButtons[i]->_fontHover = nullptr;
		_respButtons[i]->_fontPress = nullptr;
		_respButtons[i]->setText("");
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::createButtons() {
	clearButtons();

	_scrollOffset = 0;
	for (int32 i = 0; i < _responses.getSize(); i++) {
		UIButton *btn = new UIButton(_game);
		if (btn) {
			btn->_parent = _window;
			btn->_sharedFonts = btn->_sharedImages = true;
			btn->_sharedCursors = true;
			// iconic
			if (_responses[i]->_icon) {
				btn->_image = _responses[i]->_icon;
				if (_responses[i]->_iconHover) {
					btn->_imageHover = _responses[i]->_iconHover;
				}
				if (_responses[i]->_iconPressed) {
					btn->_imagePress = _responses[i]->_iconPressed;
				}

				btn->setCaption(_responses[i]->_text);
				if (_cursor) {
					btn->_cursor = _cursor;
				} else if (_game->_activeCursor) {
					btn->_cursor = _game->_activeCursor;
				}
			}
			// textual
			else {
				btn->setText(_responses[i]->_text);
				btn->_font = (_font == nullptr) ? _game->_systemFont : _font;
				btn->_fontHover = (_fontHover == nullptr) ? _game->_systemFont : _fontHover;
				btn->_fontPress = btn->_fontHover;
				btn->_align = _align;

				if (_responses[i]->_font) {
					btn->_font = _responses[i]->_font;
				}

				btn->_width = _responseArea.right - _responseArea.left;
				if (btn->_width <= 0) {
					btn->_width = _game->_renderer->getWidth();
				}
			}

#ifdef ENABLE_FOXTAIL
			if (BaseEngine::instance().isFoxTail()) {
				btn->addScript("interface/scripts/dialogue_button.script");
				btn->_width = 120;
				if (_fontHover == nullptr) {
					btn->_fontHover = btn->_font;
					btn->_fontPress = btn->_fontHover;
				}
			}
#endif

			btn->setName("response");
			btn->correctSize();
			//btn->SetListener(this, btn, _responses[i]->_iD);
			btn->setListener(this, btn, i);
			btn->_visible = false;
			_respButtons.add(btn);

			if (_responseArea.bottom - _responseArea.top < btn->_height) {
				_game->LOG(0, "Warning: Response '%s' is too high to be displayed within response box. Correcting.", _responses[i]->_text);
				_responseArea.bottom += (btn->_height - (_responseArea.bottom - _responseArea.top));
			}
		}
	}
	_ready = false;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_game->LOG(0, "AdResponseBox::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_game->LOG(0, "Error parsing RESPONSE_BOX file '%s'", filename);
	}


	delete[] buffer;

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
bool AdResponseBox::loadBuffer(char *buffer, bool complete) {
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


	char *params;
	int cmd;
	BaseParser parser(_game);

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_RESPONSE_BOX) {
			_game->LOG(0, "'RESPONSE_BOX' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_WINDOW:
			SAFE_DELETE(_window);
			_window = new UIWindow(_game);
			if (!_window || DID_FAIL(_window->loadBuffer(params, false))) {
				SAFE_DELETE(_window);
				cmd = PARSERR_GENERIC;
			} else if (_shieldWindow) {
				_shieldWindow->_parent = _window;
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

		case TOKEN_FONT_HOVER:
			if (_fontHover) {
				_game->_fontStorage->removeFont(_fontHover);
			}
			_fontHover = _game->_fontStorage->addFont(params);
			if (!_fontHover) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_AREA:
			parser.scanStr(params, "%d,%d,%d,%d", &_responseArea.left, &_responseArea.top, &_responseArea.right, &_responseArea.bottom);
			break;

		case TOKEN_HORIZONTAL:
			parser.scanStr(params, "%b", &_horizontal);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp(params, "center") == 0) {
				_align = TAL_CENTER;
			} else if (scumm_stricmp(params, "right") == 0) {
				_align = TAL_RIGHT;
			} else {
				_align = TAL_LEFT;
			}
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp(params, "top") == 0) {
				_verticalAlign = VAL_TOP;
			} else if (scumm_stricmp(params, "center") == 0) {
				_verticalAlign = VAL_CENTER;
			} else {
				_verticalAlign = VAL_BOTTOM;
			}
			break;

		case TOKEN_SPACING:
			parser.scanStr(params, "%d", &_spacing);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_CURSOR:
			SAFE_DELETE(_cursor);
			_cursor = new BaseSprite(_game);
			if (!_cursor || DID_FAIL(_cursor->loadFile(params))) {
				SAFE_DELETE(_cursor);
				cmd = PARSERR_GENERIC;
			}
			break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_game->LOG(0, "Syntax error in RESPONSE_BOX definition");
		return STATUS_FAILED;
	}

	if (_window) {
		for (int32 i = 0; i < _window->_widgets.getSize(); i++) {
			if (!_window->_widgets[i]->_listenerObject) {
				_window->_widgets[i]->setListener(this, _window->_widgets[i], 0);
			}
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "RESPONSE_BOX\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "AREA { %d, %d, %d, %d }\n", _responseArea.left, _responseArea.top, _responseArea.right, _responseArea.bottom);

	if (_font && _font->_filename) {
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);
	}
	if (_fontHover && _fontHover->_filename) {
		buffer->putTextIndent(indent + 2, "FONT_HOVER=\"%s\"\n", _fontHover->_filename);
	}

	if (_cursor && _cursor->_filename) {
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);
	}

	buffer->putTextIndent(indent + 2, "HORIZONTAL=%s\n", _horizontal ? "TRUE" : "FALSE");

	switch (_align) {
	case TAL_LEFT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		buffer->putTextIndent(indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	default:
		error("AdResponseBox::SaveAsText - Unhandled enum");
		break;
	}

	switch (_verticalAlign) {
	case VAL_TOP:
	default:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		buffer->putTextIndent(indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
		break;
	}

	buffer->putTextIndent(indent + 2, "SPACING=%d\n", _spacing);

	buffer->putTextIndent(indent + 2, "\n");

	// window
	if (_window) {
		_window->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::display() {
	Common::Rect32 rect = _responseArea;
	if (_window) {
		BasePlatform::offsetRect(&rect, _window->_posX, _window->_posY);
		//_window->display();
	}

	int xxx, yyy;
	int32 i;

	xxx = rect.left;
	yyy = rect.top;

	// shift down if needed
	if (!_horizontal) {
		int32 totalHeight = 0;
		for (i = 0; i < _respButtons.getSize(); i++) {
			totalHeight += (_respButtons[i]->_height + _spacing);
		}
		totalHeight -= _spacing;

		switch (_verticalAlign) {
		case VAL_BOTTOM:
			if (yyy + totalHeight < rect.bottom) {
				yyy = rect.bottom - totalHeight;
			}
			break;

		case VAL_CENTER:
			if (yyy + totalHeight < rect.bottom) {
				yyy += ((rect.bottom - rect.top) - totalHeight) / 2;
			}
			break;

		case VAL_TOP:
		default:
			// do nothing
			break;
		}
	}

	// prepare response buttons
	bool scrollNeeded = false;
	for (i = _scrollOffset; i < _respButtons.getSize(); i++) {

#ifdef ENABLE_FOXTAIL
		// FoxTail's "HORIZONTAL=TRUE" display boxes are actual 2x3 display boxes
		// Tests show that this hack was removed in FOXTAIL_1_2_362
		if (_horizontal && BaseEngine::instance().isFoxTail(FOXTAIL_OLDEST_VERSION, FOXTAIL_1_2_304)) {
			if (i >= _scrollOffset + 6) {
				scrollNeeded = true;
				break;
			}
			_respButtons[i]->_visible = true;
			_respButtons[i]->_posX = 55 + 120 * (i / 3);
			_respButtons[i]->_posY = 100 + 10 * (i % 3);
			continue;
		}
#endif

		if ((_horizontal && xxx + _respButtons[i]->_width > rect.right) || (!_horizontal && yyy + _respButtons[i]->_height > rect.bottom)) {

			scrollNeeded = true;
			_respButtons[i]->_visible = false;
			break;
		}

		_respButtons[i]->_visible = true;
		_respButtons[i]->_posX = xxx;
		_respButtons[i]->_posY = yyy;

		if (_horizontal) {
			xxx += (_respButtons[i]->_width + _spacing);
		} else {
			yyy += (_respButtons[i]->_height+ _spacing);
		}
	}

	// show appropriate scroll buttons
	if (_window) {
		_window->showWidget("prev", _scrollOffset > 0);
		_window->showWidget("next", scrollNeeded);
	}

	// go exclusive
	if (_shieldWindow) {
		_shieldWindow->_posX = _shieldWindow->_posY = 0;
		_shieldWindow->_width = _game->_renderer->getWidth();
		_shieldWindow->_height = _game->_renderer->getHeight();

		_shieldWindow->display();
	}

	// display window
	if (_window) {
		_window->display();
	}


	// display response buttons
	for (i = _scrollOffset; i < _respButtons.getSize(); i++) {
		_respButtons[i]->display();
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::listen(BaseScriptHolder *param1, uint32 param2) {
	UIObject *obj = (UIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset--;
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset++;
		} else if (scumm_stricmp(obj->_name, "response") == 0) {
			if (_waitingScript) {
				_waitingScript->_stack->pushInt(_responses[param2]->_iD);
			}
			handleResponse(_responses[param2]);
			_waitingScript = nullptr;
			_game->_state = GAME_RUNNING;
			((AdGame *)_game)->_stateEx = GAME_NORMAL;
			_ready = true;
			invalidateButtons();
			clearResponses();
		} else {
			return BaseObject::listen(param1, param2);
		}
		break;
	default:
		error("AdResponseBox::Listen - Unhandled enum");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_font));
	persistMgr->transferPtr(TMEMBER_PTR(_fontHover));
	persistMgr->transferBool(TMEMBER(_horizontal));
	persistMgr->transferCharPtr(TMEMBER(_lastResponseText));
	persistMgr->transferCharPtr(TMEMBER(_lastResponseTextOrig));
	_respButtons.persist(persistMgr);
	persistMgr->transferRect32(TMEMBER(_responseArea));
	_responses.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_scrollOffset));
	persistMgr->transferPtr(TMEMBER_PTR(_shieldWindow));
	persistMgr->transferSint32(TMEMBER(_spacing));
	persistMgr->transferPtr(TMEMBER_PTR(_waitingScript));
	persistMgr->transferPtr(TMEMBER_PTR(_window));

	persistMgr->transferSint32(TMEMBER_INT(_verticalAlign));
	persistMgr->transferSint32(TMEMBER_INT(_align));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::weedResponses() {
	AdGame *adGame = (AdGame *)_game;

	for (int32 i = 0; i < _responses.getSize(); i++) {
		switch (_responses[i]->_responseType) {
		case RESPONSE_ONCE:
			if (adGame->branchResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.removeAt(i);
				i--;
			}
			break;

		case RESPONSE_ONCE_GAME:
			if (adGame->gameResponseUsed(_responses[i]->_iD)) {
				delete _responses[i];
				_responses.removeAt(i);
				i--;
			}
			break;
		default:
			debugC(kWintermuteDebugGeneral, "AdResponseBox::WeedResponses - Unhandled enum");
			break;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void AdResponseBox::setLastResponseText(const char *text, const char *textOrig) {
	BaseUtils::setString(&_lastResponseText, text);
	BaseUtils::setString(&_lastResponseTextOrig, textOrig);
}

//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::handleResponse(const AdResponse *response) {
	setLastResponseText(response->_text, response->_textOrig);

	AdGame *adGame = (AdGame *)_game;

	switch (response->_responseType) {
	case RESPONSE_ONCE:
		adGame->addBranchResponse(response->_iD);
		break;

	case RESPONSE_ONCE_GAME:
		adGame->addGameResponse(response->_iD);
		break;
	default:
		debugC(kWintermuteDebugGeneral, "AdResponseBox::HandleResponse - Unhandled enum");
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
BaseObject *AdResponseBox::getNextAccessObject(BaseObject *currObject) {
	BaseArray<UIObject *> objects;
	getObjects(objects, true);

	if (objects.getSize() == 0) {
		return nullptr;
	} else {
		if (currObject != nullptr) {
			for (int32 i = 0; i < objects.getSize(); i++) {
				if (objects[i] == currObject) {
					if (i < objects.getSize() - 1) {
						return objects[i + 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[0];
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
BaseObject *AdResponseBox::getPrevAccessObject(BaseObject *currObject) {
	BaseArray<UIObject *> objects;
	getObjects(objects, true);

	if (objects.getSize() == 0) {
		return nullptr;
	} else {
		if (currObject != nullptr) {
			for (int32 i = objects.getSize() - 1; i >= 0; i--) {
				if (objects[i] == currObject) {
					if (i > 0) {
						return objects[i - 1];
					} else {
						break;
					}
				}
			}
		}
		return objects[objects.getSize() - 1];
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool AdResponseBox::getObjects(BaseArray<UIObject *> &objects, bool interactiveOnly) {
	for (int32 i = 0; i < _respButtons.getSize(); i++) {
		objects.add(_respButtons[i]);
	}
	if (_window) {
		_window->getWindowObjects(objects, interactiveOnly);
	}

	return STATUS_OK;
}

} // End of namespace Wintermute
