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
#include "ultima/ultima8/gumps/widgets/text_widget.h"
#include "ultima/ultima8/graphics/fonts/shape_font.h"
#include "ultima/ultima8/graphics/fonts/rendered_text.h"
#include "ultima/ultima8/graphics/render_surface.h"
#include "ultima/ultima8/graphics/fonts/font_manager.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/odata_source.h"
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TextWidget, Gump)

TextWidget::TextWidget() : Gump(), _gameFont(false), _fontNum(0), _blendColour(0),
		_tx(0), _ty(0), _currentStart(0), _currentEnd(0), _targetWidth(0), _targetHeight(0),
		_cachedText(nullptr), _textAlign(Font::TEXT_LEFT) {
}

TextWidget::TextWidget(int x, int y, const Std::string &txt, bool gamefont_, int font,
                       int w, int h, Font::TextAlign align) :
	Gump(x, y, w, h), _text(txt), _gameFont(gamefont_), _fontNum(font),
	_blendColour(0), _currentStart(0), _currentEnd(0),
	_targetWidth(w), _targetHeight(h), _cachedText(nullptr), _textAlign(align) {
}

TextWidget::~TextWidget(void) {
	delete _cachedText;
}

// Init the gump, call after construction
void TextWidget::InitGump(Gump *newparent, bool take_focus) {
	Gump::InitGump(newparent, take_focus);

	Font *font = getFont();

	// Y offset is always baseline
	_dims.y = -font->getBaseline();

	// No X offset
	_dims.x = 0;

	if (_gameFont && getFont()->isHighRes()) {
		int32 w = 0;
		int32 x_ = 0, y_ = 0;
		ScreenSpaceToGumpRect(x_, y_, w, _dims.y, ROUND_OUTSIDE);

		int32 tx_ = _dims.x;
		int32 ty_ = _dims.y;

		// Note that GumpRectToScreenSpace is guaranteed to keep
		// _targetWidth/_targetHeight zero if they already were.
		GumpRectToScreenSpace(tx_, ty_, _targetWidth, _targetHeight, ROUND_OUTSIDE);

		_dims.w = _targetWidth;
		_dims.h = _targetHeight;
		x_ = 0;
		y_ = 0;
		ScreenSpaceToGumpRect(x_, y_, _dims.w, _dims.h, ROUND_OUTSIDE);
	}

	setupNextText();
}

int TextWidget::getVlead() {
	renderText();
	assert(_cachedText);

	int32 vlead = _cachedText->getVlead();

	if (_gameFont && getFont()->isHighRes()) {
		int32 xv = 0, yv = 0, w = 0;
		ScreenSpaceToGumpRect(xv, yv, w, vlead, ROUND_OUTSIDE);
	}

	return vlead;
}

Font *TextWidget::getFont() const {
	if (_gameFont)
		return FontManager::get_instance()->getGameFont(_fontNum, true);
	else
		return FontManager::get_instance()->getTTFont(_fontNum);
}

bool TextWidget::setupNextText() {
	_currentStart = _currentEnd;

	if (_currentStart >= _text.size()) return false;

	Font *font = getFont();

	unsigned int remaining;
	font->getTextSize(_text.substr(_currentStart), _tx, _ty, remaining,
	                  _targetWidth, _targetHeight, _textAlign, true);


	_dims.w = _tx;
	_dims.h = _ty;
	_dims.y = -font->getBaseline();
	_dims.x = 0;
	_currentEnd = _currentStart + remaining;

	delete _cachedText;
	_cachedText = nullptr;

	if (_gameFont) {
		Font *fontP = getFont();
		if (fontP->isHighRes()) {
			int32 x_ = 0, y_ = 0;
			ScreenSpaceToGumpRect(x_, y_, _dims.w, _dims.h, ROUND_OUTSIDE);

			int32 w = 0;
			x_ = 0;
			y_ = 0;
			ScreenSpaceToGumpRect(x_, y_, w, _dims.y, ROUND_OUTSIDE);
		}
	}

	return true;
}

void TextWidget::rewind() {
	_currentStart = 0;
	_currentEnd = 0;
	setupNextText();
}

void TextWidget::renderText() {
	if (!_cachedText) {
		Font *font = getFont();

		unsigned int remaining;
		_cachedText = font->renderText(_text.substr(_currentStart,
		                               _currentEnd - _currentStart),
		                               remaining, _targetWidth, _targetHeight,
		                               _textAlign, true);
	}
}

// Overloadable method to Paint just this Gump (RenderSurface is relative to this)
void TextWidget::PaintThis(RenderSurface *surf, int32 lerp_factor, bool scaled) {
	Gump::PaintThis(surf, lerp_factor, scaled);

	renderText();

	if (scaled && _gameFont && getFont()->isHighRes()) {
		surf->FillAlpha(0xFF, _dims.x, _dims.y, _dims.w, _dims.h);
		return;
	}

	if (!_blendColour)
		_cachedText->draw(surf, 0, 0);
	else
		_cachedText->drawBlended(surf, 0, 0, _blendColour);
}

// Overloadable method to Paint just this gumps unscaled components that require compositing (RenderSurface is relative to parent).
void TextWidget::PaintComposited(RenderSurface *surf, int32 lerp_factor, int32 sx, int32 sy) {
	Font *font = getFont();

	if (!_gameFont || !font->isHighRes()) return;

	int32 x = 0, y = 0;
	GumpToScreenSpace(x, y, ROUND_BOTTOMRIGHT);

	if (!_blendColour)
		_cachedText->draw(surf, x, y, true);
	else
		_cachedText->drawBlended(surf, x, y, _blendColour, true);

	if (_parent->IsOfType<BarkGump>())
		return;

	if (_parent->IsOfType<ButtonWidget>() && _parent->GetParent()->IsOfType<AskGump>())
		return;

	x = _dims.x;
	y = _dims.y;
	int32 w = _dims.w, h = _dims.h;
	GumpRectToScreenSpace(x, y, w, h, ROUND_OUTSIDE);
	surf->FillAlpha(0x00, x, y, w, h);
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump *TextWidget::OnMouseMotion(int32 mx, int32 my) {
	return nullptr;
}


void TextWidget::saveData(ODataSource *ods) {
	Gump::saveData(ods);

	ods->write1(_gameFont ? 1 : 0);
	ods->write4(static_cast<uint32>(_fontNum));
	ods->write4(_blendColour);
	ods->write4(static_cast<uint32>(_currentStart));
	ods->write4(static_cast<uint32>(_currentEnd));
	ods->write4(static_cast<uint32>(_targetWidth));
	ods->write4(static_cast<uint32>(_targetHeight));
	ods->write2(static_cast<uint16>(_textAlign));
	ods->write4(_text.size());
	ods->write(_text.c_str(), _text.size());
}

bool TextWidget::loadData(IDataSource *ids, uint32 version) {
	if (!Gump::loadData(ids, version)) return false;

	_gameFont = (ids->read1() != 0);
	_fontNum = static_cast<int>(ids->read4());
	_blendColour = ids->read4();
	_currentStart = static_cast<int>(ids->read4());
	_currentEnd = static_cast<int>(ids->read4());
	_targetWidth = static_cast<int>(ids->read4());
	_targetHeight = static_cast<int>(ids->read4());
	_textAlign = static_cast<Font::TextAlign>(ids->read2());

	uint32 slen = ids->read4();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		ids->read(buf, slen);
		buf[slen] = 0;
		_text = buf;
		delete[] buf;
	} else {
		_text = "";
	}

	// HACK ALERT: this is to deal with possibly changing font sizes
	// after loading.
	Font *font = getFont();

	int32 tx_, ty_;
	unsigned int remaining;
	font->getTextSize(_text.substr(_currentStart), tx_, ty_, remaining,
	                  _targetWidth, _targetHeight, _textAlign, true);

	// Y offset is always baseline
	_dims.y = -font->getBaseline();
	_dims.w = tx_;
	_dims.h = ty_;
	_currentEnd = _currentStart + remaining;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
