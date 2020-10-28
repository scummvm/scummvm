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
#include "ultima/ultima8/graphics/fonts/tt_font.h"
#include "ultima/ultima8/gumps/bark_gump.h"
#include "ultima/ultima8/gumps/ask_gump.h"
#include "ultima/ultima8/gumps/widgets/button_widget.h"

namespace Ultima {
namespace Ultima8 {

DEFINE_RUNTIME_CLASSTYPE_CODE(TextWidget)

TextWidget::TextWidget() : Gump(), _gameFont(false), _fontNum(0), _blendColour(0),
		_tx(0), _ty(0), _currentStart(0), _currentEnd(0), _targetWidth(0), _targetHeight(0),
		_cachedText(nullptr), _textAlign(Font::TEXT_LEFT) {
}

TextWidget::TextWidget(int x, int y, const Std::string &txt, bool gamefont, int font,
                       int w, int h, Font::TextAlign align) :
	Gump(x, y, w, h), _text(txt), _gameFont(gamefont), _fontNum(font),
	_blendColour(0), _currentStart(0), _currentEnd(0), _tx(0), _ty(0),
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
	_dims.moveTo(0, -font->getBaseline());

	if (_gameFont && getFont()->isHighRes()) {
		Rect rect(_dims);
		ScreenSpaceToGumpRect(rect, ROUND_OUTSIDE);
		_dims.moveTo(0, rect.top);

		// Note that GumpRectToScreenSpace is guaranteed to keep
		// _targetWidth/_targetHeight zero if they already were.
		Rect target(_dims);
		GumpRectToScreenSpace(target, ROUND_OUTSIDE);

		_targetWidth = target.width();
		_targetHeight = target.height();

		Rect sr(0, 0, _targetWidth, _targetHeight);
		ScreenSpaceToGumpRect(sr, ROUND_OUTSIDE);
		_dims.setWidth(sr.width());
		_dims.setHeight(sr.height());
	}

	setupNextText();
}

int TextWidget::getVlead() {
	renderText();
	assert(_cachedText);

	int32 vlead = _cachedText->getVlead();

	if (_gameFont && getFont()->isHighRes()) {
		Rect rect(0, 0, 0, vlead);
		ScreenSpaceToGumpRect(rect, ROUND_OUTSIDE);
		vlead = rect.height();
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


	_dims.top = -font->getBaseline();
	_dims.left = 0;
	_dims.setWidth(_tx);
	_dims.setHeight(_ty);
	_currentEnd = _currentStart + remaining;

	delete _cachedText;
	_cachedText = nullptr;

	if (_gameFont) {
		Font *fontP = getFont();
		if (fontP->isHighRes()) {
			Rect sr(0, 0, _dims.width(), _dims.height());
			ScreenSpaceToGumpRect(sr, ROUND_OUTSIDE);
			_dims.setWidth(sr.width());
			_dims.setHeight(sr.height());

			sr = Rect(0, 0, 0, _dims.top);
			ScreenSpaceToGumpRect(sr, ROUND_OUTSIDE);
			_dims.moveTo(_dims.left, sr.height());
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
		surf->FillAlpha(0xFF, _dims.left, _dims.top, _dims.width(), _dims.height());
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

	if (dynamic_cast<BarkGump *>(_parent))
		return;

	if (dynamic_cast<ButtonWidget *>(_parent) && dynamic_cast<AskGump *>(_parent->GetParent()))
		return;

	Rect rect(_dims);
	GumpRectToScreenSpace(rect, ROUND_OUTSIDE);
	surf->FillAlpha(0x00, rect.left, rect.top, rect.width(), rect.height());
}

// don't handle any mouse motion events, so let parent handle them for us.
Gump *TextWidget::onMouseMotion(int32 mx, int32 my) {
	return nullptr;
}


void TextWidget::saveData(Common::WriteStream *ws) {
	Gump::saveData(ws);

	ws->writeByte(_gameFont ? 1 : 0);
	ws->writeUint32LE(static_cast<uint32>(_fontNum));
	ws->writeUint32LE(_blendColour);
	ws->writeUint32LE(static_cast<uint32>(_currentStart));
	ws->writeUint32LE(static_cast<uint32>(_currentEnd));
	ws->writeUint32LE(static_cast<uint32>(_targetWidth));
	ws->writeUint32LE(static_cast<uint32>(_targetHeight));
	ws->writeUint16LE(static_cast<uint16>(_textAlign));
	ws->writeUint32LE(_text.size());
	ws->write(_text.c_str(), _text.size());
}

bool TextWidget::loadData(Common::ReadStream *rs, uint32 version) {
	if (!Gump::loadData(rs, version)) return false;

	_gameFont = (rs->readByte() != 0);
	_fontNum = static_cast<int>(rs->readUint32LE());
	_blendColour = rs->readUint32LE();
	_currentStart = static_cast<int>(rs->readUint32LE());
	_currentEnd = static_cast<int>(rs->readUint32LE());
	_targetWidth = static_cast<int>(rs->readUint32LE());
	_targetHeight = static_cast<int>(rs->readUint32LE());
	_textAlign = static_cast<Font::TextAlign>(rs->readUint16LE());

	uint32 slen = rs->readUint32LE();
	if (slen > 0) {
		char *buf = new char[slen + 1];
		rs->read(buf, slen);
		buf[slen] = 0;
		_text = buf;
		delete[] buf;
	} else {
		_text = "";
	}

	// HACK ALERT: this is to deal with possibly changing font sizes
	// after loading.
	Font *font = getFont();

	int32 tx, ty;
	unsigned int remaining;
	font->getTextSize(_text.substr(_currentStart), tx, ty, remaining,
	                  _targetWidth, _targetHeight, _textAlign, true);

	// Y offset is always baseline
	_dims.top = -font->getBaseline();
	_dims.setWidth(tx);
	_dims.setHeight(ty);
	_currentEnd = _currentStart + remaining;

	return true;
}

} // End of namespace Ultima8
} // End of namespace Ultima
