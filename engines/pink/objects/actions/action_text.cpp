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

#include "common/debug.h"
#include "common/substream.h"
#include "common/unicode-bidi.h"

#include "graphics/paletteman.h"

#include "pink/archive.h"
#include "pink/screen.h"
#include "pink/pink.h"
#include "pink/objects/actors/actor.h"
#include "pink/objects/actions/action_text.h"
#include "pink/objects/pages/page.h"

namespace Pink {

ActionText::ActionText() {
	_txtWnd = nullptr;
	_macText = nullptr;

	_xLeft = _xRight = 0;
	_yTop = _yBottom = 0;

	_centered = 0;
	_scrollBar = 0;

	_textRGB = 0;
	_backgroundRGB = 0;

	_textColorIndex = 0;
	_backgroundColorIndex = 0;
}

ActionText::~ActionText() {
	end();
}

void ActionText::deserialize(Archive &archive) {
	Action::deserialize(archive);
	_fileName = archive.readString();

	_xLeft = archive.readDWORD();
	_yTop = archive.readDWORD();
	_xRight = archive.readDWORD();
	_yBottom = archive.readDWORD();

	_centered = archive.readDWORD();
	_scrollBar = archive.readDWORD();

	byte r = archive.readByte();
	byte g = archive.readByte();
	byte b = archive.readByte();
	(void)archive.readByte(); // skip Alpha
	_textRGB = r << 16 | g << 8 | b;

	r = archive.readByte();
	g = archive.readByte();
	b = archive.readByte();
	(void)archive.readByte(); // skip Alpha
	_backgroundRGB = r << 16 | g << 8 | b;
}

void ActionText::toConsole() const {
	debugC(6, kPinkDebugLoadingObjects, "\tActionText: _name = %s, _fileName = %s, "
				  "_xLeft = %u, _yTop = %u, _xRight = %u, _yBottom = %u _centered = %u, _scrollBar = %u, _textColor = %u _backgroundColor = %u",
		  _name.c_str(), _fileName.c_str(), _xLeft, _yTop, _xRight, _yBottom, _centered, _scrollBar, _textRGB, _backgroundRGB);
}

void ActionText::start() {
	findColorsInPalette();
	Screen *screen = _actor->getPage()->getGame()->getScreen();
	Graphics::TextAlign align = _centered ? Graphics::kTextAlignCenter : Graphics::kTextAlignLeft;
	Common::SeekableReadStream *stream = _actor->getPage()->getResourceStream(_fileName);

	char *str = new char[stream->size()];
	stream->read(str, stream->size());
	delete stream;

	Common::Language language = _actor->getPage()->getGame()->getLanguage();
	screen->getWndManager()._language = language;
	switch(language) {
	case Common::DA_DNK:
		// fall through
	case Common::ES_ESP:
		// fall through
	case Common::FR_FRA:
		// fall through
	case Common::PT_BRA:
		// fall through
	case Common::DE_DEU:
		// fall through
	case Common::IT_ITA:
		// fall through
	case Common::NL_NLD:
		_text = Common::String(str).decode(Common::kWindows1252);
		break;

	case Common::FI_FIN:
		// fall through
	case Common::SE_SWE:
		_text = Common::String(str).decode(Common::kWindows1257);
		break;

	case Common::HE_ISR:
		_text = Common::String(str).decode(Common::kWindows1255);
		if (!_centered) {
			align = Graphics::kTextAlignRight;
		}
		break;

	case Common::PL_POL:
		_text = Common::String(str).decode(Common::kWindows1250);
		break;

	case Common::RU_RUS:
		_text = Common::String(str).decode(Common::kWindows1251);
		break;

	case Common::EN_GRB:
		// fall through
	case Common::EN_ANY:
		// fall through
	default:
		_text = Common::String(str);
		break;
	}

	_text.trim();
	delete[] str;

	while ( _text.size() > 0 && (_text[ _text.size() - 1 ] == '\n' || _text[ _text.size() - 1 ] == '\r') )
		_text.deleteLastChar();

	if (_scrollBar) {
		_txtWnd = screen->getWndManager().addTextWindow(screen->getTextFont(), _textColorIndex, _backgroundColorIndex,
														  _xRight - _xLeft, align, nullptr, false);
		_txtWnd->setTextColorRGB(_textRGB);
		_txtWnd->enableScrollbar(true);
		// it will hide the scrollbar when the text height is smaller than the window height
		_txtWnd->setMode(Graphics::kWindowModeDynamicScrollbar);
		_txtWnd->move(_xLeft, _yTop);
		_txtWnd->resize(_xRight - _xLeft, _yBottom - _yTop);
		_txtWnd->setEditable(false);
		_txtWnd->setSelectable(false);

		_txtWnd->appendText(_text);
		screen->addTextWindow(_txtWnd);

	} else {
		screen->addTextAction(this);

		_macText = new Graphics::MacText(_text, &screen->getWndManager(), screen->getTextFont(), _textColorIndex, _backgroundColorIndex, _xRight - _xLeft, align);
	}
}

Common::Rect ActionText::getBound() {
	return Common::Rect(_xLeft, _yTop, _xRight, _yBottom);
}

void ActionText::end() {
	Screen *screen = _actor->getPage()->getGame()->getScreen();
	screen->addDirtyRect(this->getBound());
	if (_scrollBar && _txtWnd) {
		screen->getWndManager().removeWindow(_txtWnd);
		screen->removeTextWindow(_txtWnd);
		_txtWnd = nullptr;
	} else {
		screen->removeTextAction(this);
		delete _macText;
	}
}

void ActionText::draw(Graphics::ManagedSurface *surface) {
	int yOffset = 0;
	// we need to first fill this area with backgroundColor, in order to wash away the previous text
	surface->fillRect(Common::Rect(_xLeft, _yTop, _xRight, _yBottom), _backgroundColorIndex);

	if (_centered) {
		yOffset = (_yBottom - _yTop) / 2 - _macText->getTextHeight() / 2;
	}
	_macText->drawToPoint(surface, Common::Rect(0, 0, _xRight - _xLeft, _yBottom - _yTop), Common::Point(_xLeft, _yTop + yOffset));
}

#define BLUE(rgb) ((rgb) & 0xFF)
#define GREEN(rgb) (((rgb) >> 8) & 0xFF)
#define RED(rgb) (((rgb) >> 16) & 0xFF)

void ActionText::findColorsInPalette() {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	g_paletteLookup->setPalette(palette, 256);

	debug(2, "textcolorindex: %06x", _textRGB);
	_textColorIndex = g_paletteLookup->findBestColor(RED(_textRGB), GREEN(_textRGB), BLUE(_textRGB));
	debug(2, "backgroundColorIndex: %06x", _backgroundRGB);
	_backgroundColorIndex = g_paletteLookup->findBestColor(RED(_backgroundRGB), GREEN(_backgroundRGB), BLUE(_backgroundRGB));
}

} // End of namespace Pink
