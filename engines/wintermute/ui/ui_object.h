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

#ifndef WINTERMUTE_UIOBJECT_H
#define WINTERMUTE_UIOBJECT_H


#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView

namespace Wintermute {

class UITiledImage;
class BaseFont;
class UIObject : public BaseObject {
public:
	//virtual char *GetAccessCaption();

	bool getTotalOffset(int *offsetX, int *offsetY);
	bool _canFocus;
	bool focus();
	bool handleMouse(TMouseEvent event, TMouseButton button) override;
	bool isFocused();
	bool _parentNotify;
	DECLARE_PERSISTENT(UIObject, BaseObject)
	UIObject *_parent;
	bool display() override { return display(0, 0); }
	virtual bool display(int offsetX) { return display(offsetX, 0); }
	virtual bool display(int offsetX, int offsetY);
	virtual void correctSize();
	bool _sharedFonts;
	bool _sharedImages;
	void setText(const char *text);
	char *_text;
	BaseFont *_font;
	bool _visible;
	UITiledImage *_back;
	bool _disable;
	UIObject(BaseGame *inGame = nullptr);
	~UIObject() override;
	int32 _width;
	int32 _height;
	TUIObjectType _type;
	BaseSprite *_image;
	void setListener(BaseScriptHolder *object, BaseScriptHolder *listenerObject, uint32 listenerParam);
	BaseScriptHolder *_listenerParamObject;
	uint32 _listenerParamDWORD;
	BaseScriptHolder *_listenerObject;
	UIObject *_focusedWidget;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	// scripting interface
	ScValue *scGetProperty(const char *name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
};

} // End of namespace Wintermute

#endif
