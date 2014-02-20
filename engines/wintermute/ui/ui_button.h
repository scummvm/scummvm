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

#ifndef WINTERMUTE_UIBUTTON_H
#define WINTERMUTE_UIBUTTON_H


#include "engines/wintermute/ui/ui_object.h"
#include "engines/wintermute/dctypes.h"    // Added by ClassView

namespace Wintermute {

class UIButton : public UIObject {
public:

	DECLARE_PERSISTENT(UIButton, UIObject)
	void press();
	virtual bool display() { return display(0, 0); }
	virtual bool display(int offsetX, int offsetY);

	void correctSize();

	UIButton(BaseGame *inGame = nullptr);
	virtual ~UIButton();
	bool loadFile(const char *filename);
	bool loadBuffer(char *buffer, bool complete = true);
	virtual bool saveAsText(BaseDynamicBuffer *buffer, int indent) override;

	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name) override;
	virtual bool scSetProperty(const char *name, ScValue *value) override;
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) override;
	virtual const char *scToString() override;


	void setFontHover(BaseFont *font);
	BaseFont *getFontHover();
	void setFontPress(BaseFont *font);

	void setTextAlign(TTextAlign align);

	void setImageHover(BaseSprite *sprite);
	void setImagePress(BaseSprite *sprite);

private:
	bool _pixelPerfect;
	bool _stayPressed;
	bool _centerImage;
	bool _oneTimePress;
	UITiledImage *_backPress;
	UITiledImage *_backHover;
	UITiledImage *_backDisable;
	UITiledImage *_backFocus;
	bool _press;
	bool _hover;
	BaseFont *_fontDisable;
	BaseFont *_fontPress;
	BaseFont *_fontHover;
	BaseFont *_fontFocus;
	BaseSprite *_imageHover;
	BaseSprite *_imagePress;
	BaseSprite *_imageDisable;
	BaseSprite *_imageFocus;
	uint32 _oneTimePressTime;
	TTextAlign _align;

};

} // End of namespace Wintermute

#endif
