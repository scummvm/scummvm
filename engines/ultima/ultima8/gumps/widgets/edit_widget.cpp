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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/gumps/widgets/edit_widget.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/misc/encoding.h"
#include "common/system.h"
#include "common/events.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(EditWidget, Gump)

EditWidget::EditWidget(int X, int Y, Std::string txt, bool gamefont_, int font,
                       int w, int h, unsigned int maxlength_, bool multiline_)
	: Gump(X, Y, w, h), text(txt), gamefont(gamefont_), fontnum(font),
	  maxlength(maxlength_), multiline(multiline_),
	  cursor_changed(0), cursor_visible(true), cached_text(0) {
	cursor = text.size();
}

EditWidget::~EditWidget(void) {
	delete cached_text;
	cached_text = 0;
}

// Init the gump, call after construction
void EditWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	Pentagram::Font *font = getFont();

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;

	if (gamefont && getFont()->isHighRes()) {
		int32 x_ = 0, y_ = 0, w = 0;
		ScreenSpaceToGumpRect(x_, y_, w, dims.y, ROUND_OUTSIDE);
	}
}

Pentagram::Font *EditWidget::getFont() const {
	if (gamefont)
		return FontManager::get_instance()->getGameFont(fontnum, true);
	else
		return FontManager::get_instance()->getTTFont(fontnum);
}

void EditWidget::setText(const Std::string &t) {
	text = t;
	cursor = text.size();
	FORGET_OBJECT(cached_text);
}

void EditWidget::ensureCursorVisible() {
	cursor_visible = true;
	cursor_changed = g_system->getMillis();
}

bool EditWidget::textFits(Std::string &t) {
	Pentagram::Font *font = getFont();

	unsigned int remaining;
	int32 width, height;

	int32 max_width = multiline ? dims.w : 0;
	int32 max_height = dims.h;
	if (gamefont && font->isHighRes()) {
		int32 x_ = 0, y_ = 0;
		GumpRectToScreenSpace(x_, y_, max_width, max_height, ROUND_INSIDE);
	}

	font->getTextSize(t, width, height, remaining,
	                  max_width, max_height,
	                  Pentagram::Font::TEXT_LEFT, false);

	if (gamefont && font->isHighRes()) {
		int32 x_ = 0, y_ = 0;
		ScreenSpaceToGumpRect(x_, y_, width, height, ROUND_OUTSIDE);
	}

	if (multiline)
		return (remaining >= t.size());
	else
		return (width <= dims.w);
}

void EditWidget::renderText() {
	bool cv = cursor_visible;
	if (!IsFocus()) {
		cv = false;
	} else {
		uint32 now = g_system->getMillis();
		if (now > cursor_changed + 750) {
			cv = !cursor_visible;
			cursor_changed = now;
		}
	}

	if (cv != cursor_visible) {
		FORGET_OBJECT(cached_text);
		cursor_visible = cv;
	}

	if (!cached_text) {
		Pentagram::Font *font = getFont();

		int32 max_width = multiline ? dims.w : 0;
		int32 max_height = dims.h;
		if (gamefont && font->isHighRes()) {
			int32 x_ = 0, y_ = 0;
			GumpRectToScreenSpace(x_, y_, max_width, max_height, ROUND_INSIDE);
		}

		unsigned int remaining;
		cached_text = font->renderText(text, remaining,
		                               max_width, max_height,
		                               Pentagram::Font::TEXT_LEFT,
		                               false, cv ? cursor : Std::string::npos);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void EditWidget::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	renderText();

	if (scaled && gamefont && getFont()->isHighRes()) {
		surf->FillAlpha(0xFF, dims.x, dims.y, dims.w, dims.h);
		return;
	}

	cached_text->draw(surf, 0, 0);
}

// Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
void EditWidget::PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy) {
	Pentagram::Font *font = getFont();

	if (!gamefont || !font->isHighRes()) return;

	int32 x_ = 0, y_ = 0;
	GumpToScreenSpace(x_, y_, ROUND_BOTTOMRIGHT);

	cached_text->draw(surf, x_, y_, true);

	x_ = dims.x;
	y_ = dims.y;
	int32 w = dims.w, h = dims.h;
	GumpRectToScreenSpace(x_, y_, w, h, ROUND_OUTSIDE);
	surf->FillAlpha(0x00, x_, y_, w, h);
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump *EditWidget::OnMouseMotion(int32 mx, int32 my) {
	return 0;
}

bool EditWidget::OnKeyDown(int key, int mod) {
	switch (key) {
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		parent->ChildNotify(this, EDIT_ENTER);
		break;
	case Common::KEYCODE_ESCAPE:
		parent->ChildNotify(this, EDIT_ESCAPE);
		break;
	case Common::KEYCODE_BACKSPACE:
		if (cursor > 0) {
			text.erase(--cursor, 1);
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case Common::KEYCODE_DELETE:
		if (cursor != text.size()) {
			text.erase(cursor, 1);
			FORGET_OBJECT(cached_text);
		}
		break;
	case Common::KEYCODE_LEFT:
		if (cursor > 0) {
			cursor--;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	case Common::KEYCODE_RIGHT:
		if (cursor < text.size()) {
			cursor++;
			FORGET_OBJECT(cached_text);
			ensureCursorVisible();
		}
		break;
	default:
		break;
	}

	return true;
}

bool EditWidget::OnKeyUp(int key) {
	return true;
}


bool EditWidget::OnTextInput(int unicode) {
	if (maxlength > 0 && text.size() >= maxlength)
		return true;

	char c = 0;
	if (unicode >= 0 && unicode < 256)
		c = Pentagram::reverse_encoding[unicode];
	if (!c) return true;

	Std::string newtext = text;
	newtext.insert(cursor, 1, c);

	if (textFits(newtext)) {
		text = newtext;
		cursor++;
		FORGET_OBJECT(cached_text);
	}

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
