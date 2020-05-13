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

#ifndef WINTERMUTE_UIOBJECT_H
#define WINTERMUTE_UIOBJECT_H


#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView

namespace Wintermute {

class UITiledImage;
class BaseFont;
class UIObject : public BaseObject {
public:

	bool getTotalOffset(int *offsetX, int *offsetY);
	bool focus();
	bool handleMouse(TMouseEvent event, TMouseButton button) override;
	bool isFocused();

	DECLARE_PERSISTENT(UIObject, BaseObject)
	UIObject *_parent;
	bool display() override { return display(0, 0); }
	virtual bool display(int offsetX) { return display(offsetX, 0); }
	virtual bool display(int offsetX, int offsetY);
	virtual void correctSize();
	void setText(const char *text);

	UIObject(BaseGame *inGame = nullptr);
	~UIObject() override;
	void setListener(BaseScriptHolder *object, BaseScriptHolder *listenerObject, uint32 listenerParam);
	BaseScriptHolder *getListener() const;

	UIObject *_focusedWidget;
	bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	// scripting interface
	ScValue *scGetProperty(const Common::String &name) override;
	bool scSetProperty(const char *name, ScValue *value) override;
	bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	const char *scToString() override;
	TUIObjectType _type;

	int32 getWidth() const;
	int32 getHeight() override;
	void setHeight(int32 height);
	void setWidth(int32 width);
	bool isDisabled() const;
	void setDisabled(bool disable);
	bool isVisible() const;
	void setVisible(bool visible);
	bool hasSharedFonts() const;
	void setSharedFonts(bool shared);
	bool hasSharedImages() const;
	void setSharedImages(bool shared);
	BaseSprite *getImage() const;
	void setImage(BaseSprite *image);
	void setFont(BaseFont *font);
	BaseFont *getFont();
	bool canFocus() const;

protected:
	BaseScriptHolder *_listenerParamObject;
	uint32 _listenerParamDWORD;
	BaseScriptHolder *_listenerObject;
	BaseSprite *_image;
	BaseFont *_font;
	bool _sharedFonts;
	bool _sharedImages;
	char *_text;
	bool _visible;
	bool _disable;
	int32 _width;
	int32 _height;
	bool _canFocus;
	bool _parentNotify;
	UITiledImage *_back;
};

} // End of namespace Wintermute

#endif
