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
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/ui/ui_object.h"
#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/ui/ui_window.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/font/base_font_storage.h"
#include "engines/wintermute/dcgf.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UIObject, false)

//////////////////////////////////////////////////////////////////////////
UIObject::UIObject(BaseGame *inGame) : BaseObject(inGame) {
	_back = nullptr;
	_image = nullptr;
	_font = nullptr;
	_text = nullptr;
	_sharedFonts = _sharedImages = false;

	_width = _height = 0;

	_listenerObject = nullptr;
	_listenerParamObject = nullptr;
	_listenerParamDWORD = 0;

	_disable = false;
	_visible = true;

	_type = UI_UNKNOWN;
	_parent = nullptr;

	_parentNotify = false;

	_focusedWidget = nullptr;

	_canFocus = false;
	_nonIntMouseEvents = true;
}


//////////////////////////////////////////////////////////////////////////
UIObject::~UIObject() {
	if (!_game->_loadInProgress) {
		SystemClassRegistry::getInstance()->enumInstances(BaseGame::invalidateValues, "ScValue", (void *)this);
	}

	if (_back) {
		delete _back;
	}
	if (_font && !_sharedFonts) {
		_game->_fontStorage->removeFont(_font);
	}

	if (_image && !_sharedImages) {
		delete _image;
	}

	if (_text) {
		delete[] _text;
	}

	_focusedWidget = nullptr; // ref only
}


//////////////////////////////////////////////////////////////////////////
void UIObject::setText(const char *text) {
	if (_text) {
		delete[] _text;
	}
	size_t textSize = strlen(text) + 1;
	_text = new char [textSize];
	Common::strcpy_s(_text, textSize, text);
	for (uint32 i = 0; i < strlen(_text); i++) {
		if (_text[i] == '|') {
			_text[i] = '\n';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::display(int offsetX, int offsetY) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void UIObject::setListener(BaseScriptHolder *object, BaseScriptHolder *listenerObject, uint32 listenerParam) {
	_listenerObject = object;
	_listenerParamObject = listenerObject;
	_listenerParamDWORD = listenerParam;
}


//////////////////////////////////////////////////////////////////////////
void UIObject::correctSize() {
	Common::Rect32 rect;

	if (_width <= 0) {
		if (_image) {
			_image->getBoundingRect(&rect, 0, 0);
			_width = rect.right - rect.left;
		} else {
			_width = 100;
		}
	}

	if (_height <= 0) {
		if (_image) {
			_image->getBoundingRect(&rect, 0, 0);
			_height = rect.bottom - rect.top;
		}
	}

	if (_back) {
		_back->correctSize(&_width, &_height);
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool UIObject::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetFont") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		if (_font) {
			_game->_fontStorage->removeFont(_font);
		}
		if (val->isNULL()) {
			_font = nullptr;
			stack->pushBool(true);
		} else {
			_font = _game->_fontStorage->addFont(val->getString());
			stack->pushBool(_font != nullptr);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetImage") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();

		/* const char *filename = */ val->getString();

		SAFE_DELETE(_image);
		if (val->isNULL()) {
			stack->pushBool(true);
			return STATUS_OK;
		}

		_image = new BaseSprite(_game);
		if (!_image || DID_FAIL(_image->loadFile(val->getString()))) {
			SAFE_DELETE(_image);
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetImage") == 0) {
		stack->correctParams(0);
		if (!_image || !_image->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_image->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetImageObject") == 0) {
		stack->correctParams(0);
		if (!_image) {
			stack->pushNULL();
		} else {
			stack->pushNative(_image, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focus
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Focus") == 0) {
		stack->correctParams(0);
		focus();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveAfter / MoveBefore
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveAfter") == 0 || strcmp(name, "MoveBefore") == 0) {
		stack->correctParams(1);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;

			int32 i;
			bool found = false;
			ScValue *val = stack->pop();
			// find directly
			if (val->isNative()) {
				UIObject *widget = (UIObject *)val->getNative();
				for (i = 0; i < win->_widgets.getSize(); i++) {
					if (win->_widgets[i] == widget) {
						found = true;
						break;
					}
				}
			}
			// find by name
			else {
				const char *findName = val->getString();
				for (i = 0; i < win->_widgets.getSize(); i++) {
					if (scumm_stricmp(win->_widgets[i]->_name, findName) == 0) {
						found = true;
						break;
					}
				}
			}

			if (found) {
				bool done = false;
				for (int32 j = 0; j < win->_widgets.getSize(); j++) {
					if (win->_widgets[j] == this) {
						if (strcmp(name, "MoveAfter") == 0) {
							i++;
						}
						if (j >= i) {
							j++;
						}

						win->_widgets.insertAt(i, this);
						win->_widgets.removeAt(j);

						done = true;
						stack->pushBool(true);
						break;
					}
				}
				if (!done) {
					stack->pushBool(false);
				}
			} else {
				stack->pushBool(false);
			}

		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToBottom
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveToBottom") == 0) {
		stack->correctParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (int32 i = 0; i < win->_widgets.getSize(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.removeAt(i);
					win->_widgets.insertAt(0, this);
					break;
				}
			}
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToTop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveToTop") == 0) {
		stack->correctParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (int32 i = 0; i < win->_widgets.getSize(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.removeAt(i);
					win->_widgets.add(this);
					break;
				}
			}
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	} else {
		return BaseObject::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *UIObject::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("ui_object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Name") == 0) {
		_scValue->setString(_name);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Parent (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Parent") == 0) {
		_scValue->setNative(_parent, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ParentNotify") == 0) {
		_scValue->setBool(_parentNotify);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_scValue->setInt(_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_scValue->setInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Visible") == 0) {
		_scValue->setBool(_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Disabled") == 0) {
		_scValue->setBool(_disable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		_scValue->setString(_text);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NextSibling (RO) / PrevSibling (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NextSibling") == 0 || strcmp(name, "PrevSibling") == 0) {
		_scValue->setNULL();
		if (_parent && _parent->_type == UI_WINDOW) {
			UIWindow *win = (UIWindow *)_parent;
			for (int32 i = 0; i < win->_widgets.getSize(); i++) {
				if (win->_widgets[i] == this) {
					if (strcmp(name, "NextSibling") == 0) {
						if (i < win->_widgets.getSize() - 1) {
							_scValue->setNative(win->_widgets[i + 1], true);
						}
					} else {
						if (i > 0) {
							_scValue->setNative(win->_widgets[i - 1], true);
						}
					}
					break;
				}
			}
		}
		return _scValue;
	} else {
		return BaseObject::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Name") == 0) {
		setName(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ParentNotify") == 0) {
		_parentNotify = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Width") == 0) {
		_width = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_height = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Visible") == 0) {
		_visible = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Disabled") == 0) {
		_disable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Text") == 0) {
		setText(value->getString());
		return STATUS_OK;
	} else {
		return BaseObject::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *UIObject::scToString() {
	return "[ui_object]";
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::isFocused() {
	if (!_game->_focusedWindow) {
		return false;
	}
	if (_game->_focusedWindow == this) {
		return true;
	}

	UIObject *obj = _game->_focusedWindow;
	while (obj) {
		if (obj == this) {
			return true;
		} else {
			obj = obj->_focusedWidget;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::handleMouse(TMouseEvent event, TMouseButton button) {
	// handle focus change
	if (event == MOUSE_CLICK && button == MOUSE_BUTTON_LEFT) {
		focus();
	}
	return BaseObject::handleMouse(event, button);
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::focus() {
	UIObject *obj = this;
	bool disabled = false;
	while (obj) {
		if (obj->_disable && obj->_type == UI_WINDOW) {
			disabled = true;
			break;
		}
		obj = obj->_parent;
	}
	if (!disabled) {
		obj = this;
		while (obj) {
			if (obj->_parent) {
				if (!obj->_disable && obj->_canFocus) {
					obj->_parent->_focusedWidget = obj;
				}
			} else {
				if (obj->_type == UI_WINDOW) {
					_game->focusWindow((UIWindow *)obj);
				}
			}

			obj = obj->_parent;
		}
	}
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::getTotalOffset(int *offsetX, int *offsetY) {
	int offX = 0, offY = 0;

	UIObject *obj = _parent;
	while (obj) {
		offX += obj->_posX;
		offY += obj->_posY;

		obj = obj->_parent;
	}
	if (offsetX) {
		*offsetX = offX;
	}
	if (offsetY) {
		*offsetY = offY;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UIObject::persist(BasePersistenceManager *persistMgr) {

	BaseObject::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER_PTR(_back));
	persistMgr->transferBool(TMEMBER(_canFocus));
	persistMgr->transferBool(TMEMBER(_disable));
	persistMgr->transferPtr(TMEMBER_PTR(_focusedWidget));
	persistMgr->transferPtr(TMEMBER_PTR(_font));
	persistMgr->transferSint32(TMEMBER(_height));
	persistMgr->transferPtr(TMEMBER_PTR(_image));
	persistMgr->transferPtr(TMEMBER_PTR(_listenerObject));
	persistMgr->transferPtr(TMEMBER_PTR(_listenerParamObject));
	persistMgr->transferUint32(TMEMBER(_listenerParamDWORD));
	persistMgr->transferPtr(TMEMBER_PTR(_parent));
	persistMgr->transferBool(TMEMBER(_parentNotify));
	persistMgr->transferBool(TMEMBER(_sharedFonts));
	persistMgr->transferBool(TMEMBER(_sharedImages));
	persistMgr->transferCharPtr(TMEMBER(_text));
	persistMgr->transferSint32(TMEMBER_INT(_type));
	persistMgr->transferBool(TMEMBER(_visible));
	persistMgr->transferSint32(TMEMBER(_width));

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
/*char *CUIObject::GetAccessCaption() {
	if (m_AccessCaption)
		return m_AccessCaption;
	else {
		if (m_Text)
			return m_Text;
		else
			return GetCaption();
	}
}*/

//////////////////////////////////////////////////////////////////////////
bool UIObject::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	return STATUS_FAILED;
}

} // End of namespace Wintermute
