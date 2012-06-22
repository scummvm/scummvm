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

#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/UI/UIObject.h"
#include "engines/wintermute/UI/UITiledImage.h"
#include "engines/wintermute/UI/UIWindow.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BFontStorage.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIObject, false)

//////////////////////////////////////////////////////////////////////////
CUIObject::CUIObject(CBGame *inGame): CBObject(inGame) {
	_back = NULL;
	_image = NULL;
	_font = NULL;
	_text = NULL;
	_sharedFonts = _sharedImages = false;

	_width = _height = 0;

	_listenerObject = NULL;
	_listenerParamObject = NULL;
	_listenerParamDWORD = 0;

	_disable = false;
	_visible = true;

	_type = UI_UNKNOWN;
	_parent = NULL;

	_parentNotify = false;

	_focusedWidget = NULL;

	_canFocus = false;
	_nonIntMouseEvents = true;
}


//////////////////////////////////////////////////////////////////////////
CUIObject::~CUIObject() {
	if (!Game->_loadInProgress) CSysClassRegistry::GetInstance()->EnumInstances(CBGame::InvalidateValues, "CScValue", (void *)this);

	if (_back) delete _back;
	if (_font && !_sharedFonts) Game->_fontStorage->RemoveFont(_font);

	if (_image && !_sharedImages) delete _image;

	if (_text) delete [] _text;

	_focusedWidget = NULL; // ref only
}


//////////////////////////////////////////////////////////////////////////
void CUIObject::SetText(const char *Text) {
	if (_text) delete [] _text;
	_text = new char [strlen(Text) + 1];
	if (_text) {
		strcpy(_text, Text);
		for (int i = 0; i < strlen(_text); i++) {
			if (_text[i] == '|') _text[i] = '\n';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::Display(int OffsetX, int OffsetY) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIObject::SetListener(CBScriptHolder *Object, CBScriptHolder *ListenerObject, uint32 ListenerParam) {
	_listenerObject = Object;
	_listenerParamObject = ListenerObject;
	_listenerParamDWORD = ListenerParam;
}


//////////////////////////////////////////////////////////////////////////
void CUIObject::CorrectSize() {
	RECT rect;

	if (_width <= 0) {
		if (_image) {
			_image->GetBoundingRect(&rect, 0, 0);
			_width = rect.right - rect.left;
		} else _width = 100;
	}

	if (_height <= 0) {
		if (_image) {
			_image->GetBoundingRect(&rect, 0, 0);
			_height = rect.bottom - rect.top;
		}
	}

	if (_back) _back->CorrectSize(&_width, &_height);
}



//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (_font) Game->_fontStorage->RemoveFont(_font);
		if (Val->IsNULL()) {
			_font = NULL;
			Stack->PushBool(true);
		} else {
			_font = Game->_fontStorage->AddFont(Val->GetString());
			Stack->PushBool(_font != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetImage") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		/* const char *Filename = */ Val->GetString();

		delete _image;
		_image = NULL;
		if (Val->IsNULL()) {
			Stack->PushBool(true);
			return S_OK;
		}

		_image = new CBSprite(Game);
		if (!_image || FAILED(_image->LoadFile(Val->GetString()))) {
			delete _image;
			_image = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetImage") == 0) {
		Stack->CorrectParams(0);
		if (!_image || !_image->_filename) Stack->PushNULL();
		else Stack->PushString(_image->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!_image) Stack->PushNULL();
		else Stack->PushNative(_image, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focus
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Focus") == 0) {
		Stack->CorrectParams(0);
		Focus();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveAfter / MoveBefore
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveAfter") == 0 || strcmp(Name, "MoveBefore") == 0) {
		Stack->CorrectParams(1);

		if (_parent && _parent->_type == UI_WINDOW) {
			CUIWindow *win = (CUIWindow *)_parent;

			int i;
			bool found = false;
			CScValue *val = Stack->Pop();
			// find directly
			if (val->IsNative()) {
				CUIObject *widget = (CUIObject *)val->GetNative();
				for (i = 0; i < win->_widgets.GetSize(); i++) {
					if (win->_widgets[i] == widget) {
						found = true;
						break;
					}
				}
			}
			// find by name
			else {
				const char *name = val->GetString();
				for (i = 0; i < win->_widgets.GetSize(); i++) {
					if (scumm_stricmp(win->_widgets[i]->_name, name) == 0) {
						found = true;
						break;
					}
				}
			}

			if (found) {
				bool done = false;
				for (int j = 0; j < win->_widgets.GetSize(); j++) {
					if (win->_widgets[j] == this) {
						if (strcmp(Name, "MoveAfter") == 0) i++;
						if (j >= i) j++;

						win->_widgets.InsertAt(i, this);
						win->_widgets.RemoveAt(j);

						done = true;
						Stack->PushBool(true);
						break;
					}
				}
				if (!done) Stack->PushBool(false);
			} else Stack->PushBool(false);

		} else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToBottom
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveToBottom") == 0) {
		Stack->CorrectParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			CUIWindow *win = (CUIWindow *)_parent;
			for (int i = 0; i < win->_widgets.GetSize(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.RemoveAt(i);
					win->_widgets.InsertAt(0, this);
					break;
				}
			}
			Stack->PushBool(true);
		} else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveToTop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveToTop") == 0) {
		Stack->CorrectParams(0);

		if (_parent && _parent->_type == UI_WINDOW) {
			CUIWindow *win = (CUIWindow *)_parent;
			for (int i = 0; i < win->_widgets.GetSize(); i++) {
				if (win->_widgets[i] == this) {
					win->_widgets.RemoveAt(i);
					win->_widgets.Add(this);
					break;
				}
			}
			Stack->PushBool(true);
		} else Stack->PushBool(false);

		return S_OK;
	}

	else return CBObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIObject::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("ui_object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Name") == 0) {
		_scValue->SetString(_name);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Parent (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Parent") == 0) {
		_scValue->SetNative(_parent, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ParentNotify") == 0) {
		_scValue->SetBool(_parentNotify);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		_scValue->SetInt(_width);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		_scValue->SetInt(_height);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Visible") == 0) {
		_scValue->SetBool(_visible);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Disabled") == 0) {
		_scValue->SetBool(_disable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Text") == 0) {
		_scValue->SetString(_text);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NextSibling (RO) / PrevSibling (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NextSibling") == 0 || strcmp(Name, "PrevSibling") == 0) {
		_scValue->SetNULL();
		if (_parent && _parent->_type == UI_WINDOW) {
			CUIWindow *win = (CUIWindow *)_parent;
			for (int i = 0; i < win->_widgets.GetSize(); i++) {
				if (win->_widgets[i] == this) {
					if (strcmp(Name, "NextSibling") == 0) {
						if (i < win->_widgets.GetSize() - 1) _scValue->SetNative(win->_widgets[i + 1], true);
					} else {
						if (i > 0) _scValue->SetNative(win->_widgets[i - 1], true);
					}
					break;
				}
			}
		}
		return _scValue;
	}

	else return CBObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ParentNotify
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ParentNotify") == 0) {
		_parentNotify = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Width
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Width") == 0) {
		_width = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		_height = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Visible
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Visible") == 0) {
		_visible = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Disabled
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Disabled") == 0) {
		_disable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Text
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Text") == 0) {
		SetText(Value->GetString());
		return S_OK;
	}

	else return CBObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CUIObject::ScToString() {
	return "[ui_object]";
}


//////////////////////////////////////////////////////////////////////////
bool CUIObject::IsFocused() {
	if (!Game->_focusedWindow) return false;
	if (Game->_focusedWindow == this) return true;

	CUIObject *obj = Game->_focusedWindow;
	while (obj) {
		if (obj == this) return true;
		else obj = obj->_focusedWidget;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::HandleMouse(TMouseEvent Event, TMouseButton Button) {
	// handle focus change
	if (Event == MOUSE_CLICK && Button == MOUSE_BUTTON_LEFT) {
		Focus();
	}
	return CBObject::HandleMouse(Event, Button);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::Focus() {
	CUIObject *obj = this;
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
				if (!obj->_disable && obj->_canFocus) obj->_parent->_focusedWidget = obj;
			} else {
				if (obj->_type == UI_WINDOW) Game->FocusWindow((CUIWindow *)obj);
			}

			obj = obj->_parent;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::GetTotalOffset(int *OffsetX, int *OffsetY) {
	int OffX = 0, OffY = 0;

	CUIObject *obj = _parent;
	while (obj) {
		OffX += obj->_posX;
		OffY += obj->_posY;

		obj = obj->_parent;
	}
	if (OffsetX) *OffsetX = OffX;
	if (OffsetY) *OffsetY = OffY;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::Persist(CBPersistMgr *persistMgr) {

	CBObject::Persist(persistMgr);

	persistMgr->transfer(TMEMBER(_back));
	persistMgr->transfer(TMEMBER(_canFocus));
	persistMgr->transfer(TMEMBER(_disable));
	persistMgr->transfer(TMEMBER(_focusedWidget));
	persistMgr->transfer(TMEMBER(_font));
	persistMgr->transfer(TMEMBER(_height));
	persistMgr->transfer(TMEMBER(_image));
	persistMgr->transfer(TMEMBER(_listenerObject));
	persistMgr->transfer(TMEMBER(_listenerParamObject));
	persistMgr->transfer(TMEMBER(_listenerParamDWORD));
	persistMgr->transfer(TMEMBER(_parent));
	persistMgr->transfer(TMEMBER(_parentNotify));
	persistMgr->transfer(TMEMBER(_sharedFonts));
	persistMgr->transfer(TMEMBER(_sharedImages));
	persistMgr->transfer(TMEMBER(_text));
	persistMgr->transfer(TMEMBER_INT(_type));
	persistMgr->transfer(TMEMBER(_visible));
	persistMgr->transfer(TMEMBER(_width));

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIObject::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	return E_FAIL;
}

} // end of namespace WinterMute
