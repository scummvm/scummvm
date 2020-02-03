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

#ifndef ULTIMA8_GUMPS_WIDGETS_TEXTWIDGET_H
#define ULTIMA8_GUMPS_WIDGETS_TEXTWIDGET_H

//
// TextWidget. Displays text in either a fixed-size or a fit-to-text rectangle.
//

#include "ultima/ultima8/gumps/gump.h"
#include "ultima/ultima8/graphics/fonts/font.h"
#include "ultima/ultima8/misc/p_dynamic_cast.h"

namespace Ultima {
namespace Ultima8 {

using Pentagram::Font;

class RenderedText;

class TextWidget : public Gump {
protected:
	Std::string     text;
	bool            gamefont;
	int             fontnum;
	uint32          blendColour;
	int32           tx, ty;

	unsigned int    current_start; //!< start of currently displaying text
	unsigned int    current_end;   //!< start of remaining text

	int32 targetwidth, targetheight;

	RenderedText *cached_text;
	Font::TextAlign textalign;
public:
	ENABLE_RUNTIME_CLASSTYPE()

	TextWidget();
	TextWidget(int X, int Y, Std::string txt, bool gamefont, int fontnum,
	           int width = 0, int height = 0,
	           Font::TextAlign align = Font::TEXT_LEFT);
	virtual ~TextWidget(void);

	// Init the gump, call after construction
	virtual void            InitGump(Gump *newparent, bool take_focus = true) override;

	// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
	virtual void            PaintThis(RenderSurface *, int32 lerp_factor, bool scaled) override;

	virtual void            PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 scalex, int32 scaley) override;

	virtual Gump *OnMouseMotion(int32 mx, int32 my) override;

	//! display the next part of the text
	//! \return false if there is no more text to display
	bool setupNextText();

	//! reset the widget to the start of the text
	void rewind();

	//! get the text that's currently being displayed
	//! \param start Returns the start of the text
	//! \param end Returns the start of the remaining text
	void getCurrentText(unsigned int &start, unsigned int &end) const {
		start = current_start;
		end = current_end;
	}

	//! set the colour to blend the rendered text with. (0 to disable)
	void setBlendColour(uint32 col) {
		blendColour = col;
	}

	//! get the RenderedText's vlead
	int getVlead();

protected:
	void renderText();

	Pentagram::Font *getFont() const;

public:
	bool loadData(IDataSource *ids, uint32 version);

protected:
	virtual void saveData(ODataSource *ods) override;
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
