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

#ifndef ULTIMA8_GUMPS_WIDGETS_EDITWIDGET_H
#define ULTIMA8_GUMPS_WIDGETS_EDITWIDGET_H

//
// EditWidget. Widget for text input (single or multi-line)
//

#include "ultima8/gumps/gump.h"

#include "ultima8/graphics/fonts/font.h"
#include "ultima8/misc/p_dynamic_cast.h"

namespace Ultima8 {

using Pentagram::Font;

class RenderedText;

class EditWidget : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE();

	EditWidget(int X, int Y, std::string txt, bool gamefont, int fontnum,
	           int width, int height, unsigned int maxlength = 0,
	           bool multiline = false);
	virtual ~EditWidget(void);

	virtual void InitGump(Gump *newparent, bool take_focus = true);

	virtual void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled);
	virtual void PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy);

	virtual Gump *OnMouseMotion(int mx, int my);
	virtual bool OnKeyDown(int key, int mod);
	virtual bool OnKeyUp(int key);
	virtual bool OnTextInput(int unicode);

	//! get the current text
	std::string getText() const {
		return text;
	}
	void setText(const std::string &t);

	enum Message {
		EDIT_ENTER = 16,
		EDIT_ESCAPE = 17
	};


protected:
	std::string text;
	std::string::size_type cursor;
	bool gamefont;
	int fontnum;
	unsigned int maxlength;
	bool multiline;

	uint32 cursor_changed;
	bool cursor_visible;

	void ensureCursorVisible();
	bool textFits(std::string &t);
	void renderText();
	Pentagram::Font *getFont() const;

	RenderedText *cached_text;

};

} // End of namespace Ultima8

#endif
