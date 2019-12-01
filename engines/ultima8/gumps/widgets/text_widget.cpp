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

#include "ultima8/misc/pent_include.h"
#include "text_widget.h"
#include "ShapeFont.h"
#include "RenderedText.h"
#include "ultima8/graphics/render_surface.h"
#include "ultima8/graphics/fonts/font_manager.h"
#include "ultima8/filesys/idata_source.h"
#include "ultima8/filesys/odata_source.h"
#include "TTFont.h"
#include "bark_gump.h"
#include "ask_gump.h"
#include "button_widget.h"

namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TextWidget, Gump);

TextWidget::TextWidget()
	: Gump(), cached_text(0) {

}

TextWidget::TextWidget(int X, int Y, std::string txt, bool gamefont_, int font,
                       int w, int h, Font::TextAlign align) :
	Gump(X, Y, w, h), text(txt), gamefont(gamefont_), fontnum(font),
	blendColour(0), current_start(0), current_end(0),
	targetwidth(w), targetheight(h), cached_text(0), textalign(align) {

}

TextWidget::~TextWidget(void) {
	delete cached_text;
	cached_text = 0;
}

// Init the gump, call after construction
void TextWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	Pentagram::Font *font = getFont();

	// Y offset is always baseline
	dims.y = -font->getBaseline();

	// No X offset
	dims.x = 0;

	if (gamefont && getFont()->isHighRes()) {
		int w = 0;
		int x = 0, y = 0;
		ScreenSpaceToGumpRect(x, y, w, dims.y, ROUND_OUTSIDE);

		int tx = dims.x;
		int ty = dims.y;

		// Note that GumpRectToScreenSpace is guaranteed to keep
		// targetwidth/targetheight zero if they already were.
		GumpRectToScreenSpace(tx, ty, targetwidth, targetheight, ROUND_OUTSIDE);

		dims.w = targetwidth;
		dims.h = targetheight;
		x = 0;
		y = 0;
		ScreenSpaceToGumpRect(x, y, dims.w, dims.h, ROUND_OUTSIDE);
	}

	setupNextText();
}

int TextWidget::getVlead() {
	renderText();
	assert(cached_text);

	int vlead = cached_text->getVlead();

	if (gamefont && getFont()->isHighRes()) {
		int x = 0, y = 0, w = 0;
		ScreenSpaceToGumpRect(x, y, w, vlead, ROUND_OUTSIDE);
	}

	return vlead;
}

Pentagram::Font *TextWidget::getFont() const {
	if (gamefont)
		return FontManager::get_instance()->getGameFont(fontnum, true);
	else
		return FontManager::get_instance()->getTTFont(fontnum);
}

bool TextWidget::setupNextText() {
	current_start = current_end;

	if (current_start >= text.size()) return false;

	Pentagram::Font *font = getFont();

	unsigned int remaining;
	font->getTextSize(text.substr(current_start), tx, ty, remaining,
	                  targetwidth, targetheight, textalign, true);


	dims.w = tx;
	dims.h = ty;
	dims.y = -font->getBaseline();
	dims.x = 0;
	current_end = current_start + remaining;

	delete cached_text;
	cached_text = 0;

	if (gamefont) {
		Pentagram::Font *font = getFont();
		if (font->isHighRes()) {
			int x = 0, y = 0;
			ScreenSpaceToGumpRect(x, y, dims.w, dims.h, ROUND_OUTSIDE);

			int w = 0;
			x = 0;
			y = 0;
			ScreenSpaceToGumpRect(x, y, w, dims.y, ROUND_OUTSIDE);
		}
	}

	return true;
}

void TextWidget::rewind() {
	current_start = 0;
	current_end = 0;
	setupNextText();
}

void TextWidget::renderText() {
	if (!cached_text) {
		Pentagram::Font *font = getFont();

		unsigned int remaining;
		cached_text = font->renderText(text.substr(current_start,
		                               current_end - current_start),
		                               remaining, targetwidth, targetheight,
		                               textalign, true);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void TextWidget::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	renderText();

	if (scaled && gamefont && getFont()->isHighRes()) {
		surf->FillAlpha(0xFF, dims.x, dims.y, dims.w, dims.h);
		return;
	}

	if (!blendColour)
		cached_text->draw(surf, 0, 0);
	else
		cached_text->drawBlended(surf, 0, 0, blendColour);
}

// Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
void TextWidget::PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy) {
	Pentagram::Font *font = getFont();

	if (!gamefont || !font->isHighRes()) return;

	int x = 0, y = 0;
	GumpToScreenSpace(x, y, ROUND_BOTTOMRIGHT);

	if (!blendColour)
		cached_text->draw(surf, x, y, true);
	else
		cached_text->drawBlended(surf, x, y, blendColour, true);

	if (parent->IsOfType<BarkGump>()) return;

	if (parent->IsOfType<ButtonWidget>() && parent->GetParent()->IsOfType<AskGump>()) return;

	x = dims.x;
	y = dims.y;
	int w = dims.w, h = dims.h;
	GumpRectToScreenSpace(x, y, w, h, ROUND_OUTSIDE);
	surf->FillAlpha(0x00, x, y, w, h);
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump *TextWidget::OnMouseMotion(int mx, int my) {
	return 0;
}


void TextWidget::saveData(ODataSource *ods) {
	Gump::saveData(ods);

	ods->write1(gamefont ? 1 : 0);
	ods->write4(static_cast<uint32>(fontnum));
	ods->write4(blendColour);
	ods->write4(static_cast<uint32>(current_start));
	ods->write4(static_cast<uint32>(current_end));
	ods->write4(static_cast<uint32>(targetwidth));
	ods->write4(static_cast<uint32>(targetheight));
	ods->write2(static_cast<uint16>(textalign));
	ods->write4(text.size());
	ods->write(text.c_str(), text.size());
}

bool TextWidget::loadData(IDataSource *ids, uint32 version) {
	if (!Gump::loadData(ids, version)) return false;

	gamefont = (ids->read1() != 0);
	fontnum = static_cast<int>(ids->read4());
	blendColour = ids->read4();
	current_start = static_cast<int>(ids->read4());
	current_end = static_cast<int>(ids->read4());
	targetwidth = static_cast<int>(ids->read4());
	targetheight = static_cast<int>(ids->read4());
	textalign = static_cast<Font::TextAlign>(ids->read2());

	uint32 slen = ids->read4();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		ids->read(buf, slen);
		buf[slen] = 0;
		text = buf;
		delete[] buf;
	} else {
		text = "";
	}

	// HACK ALERT: this is to deal with possibly changing font sizes
	// after loading.
	Pentagram::Font *font = getFont();

	int tx, ty;
	unsigned int remaining;
	font->getTextSize(text.substr(current_start), tx, ty, remaining,
	                  targetwidth, targetheight, textalign, true);

	// Y offset is always baseline
	dims.y = -font->getBaseline();
	dims.w = tx;
	dims.h = ty;
	current_end = current_start + remaining;

	return true;
}


// COLOURLESS PROTECTION

} // End of namespace Ultima8
