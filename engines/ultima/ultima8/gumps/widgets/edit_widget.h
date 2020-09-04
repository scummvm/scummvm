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

#include "ultima/ultima8/gumps/gump.h"

#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

class RenderedText;

class EditWidget : public Gump {
public:
	ENABLE_RUNTIME_CLASSTYPE()

	EditWidget(int x, int y, Std::string txt, bool gamefont, int fontnum,
	           int width, int height, unsigned int maxlength = 0,
	           bool multiline = false);
	~EditWidget() override;

	void InitGump(Gump *newparent, bool take_focus = true) override;

	void PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;
	void PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy) override;

	Gump *onMouseMotion(int32 mx, int32 my) override;
	bool OnKeyDown(int key, int mod) override;
	bool OnKeyUp(int key) override;
	bool OnTextInput(int unicode) override;

	void OnFocus(bool gain) override;

	//! get the current text
	Std::string getText() const {
		return _text;
	}
	void setText(const Std::string &t);

	enum Message {
		EDIT_ENTER = 16,
		EDIT_ESCAPE = 17
	};


protected:
	Std::string _text;
	Std::string::size_type _cursor;
	bool _gameFont;
	int _fontNum;
	unsigned int _maxLength;
	bool _multiLine;

	uint32 _cursorChanged;
	bool _cursorVisible;

	void ensureCursorVisible();
	bool textFits(Std::string &t);
	void renderText();
	Font *getFont() const;

	RenderedText *_cachedText;

};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
