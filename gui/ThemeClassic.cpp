/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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
 * $URL$
 * $Id$
 */

#include "gui/theme.h"

namespace GUI {
ThemeClassic::ThemeClassic(OSystem *system) : Theme() {
	_system = system;
	_initOk = false;
	memset(&_screen, 0, sizeof(_screen));
#ifdef OLDGUI_TRANSPARENCY
	memset(&_dialog, 0, sizeof(_dialog));
#endif
	_font = 0;
	
	// Maybe change this filename
	_configFile.loadFromFile("classic.ini");
}

ThemeClassic::~ThemeClassic() {
	deinit();
}

bool ThemeClassic::init() {
	deinit();
	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	if (_screen.pixels) {
		_initOk = true;
		clearAll();
		_bgcolor = _system->RGBToColor(0, 0, 0);
		_color = _system->RGBToColor(104, 104, 104);
		_shadowcolor = _system->RGBToColor(64, 64, 64);
		_textcolor = _system->RGBToColor(32, 160, 32);
		_textcolorhi = _system->RGBToColor(0, 255, 0);
		if (_screen.w >= 400 && _screen.h >= 300) {
			_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		} else {
			_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		}
		resetDrawArea();
	}

	if (isThemeLoadingRequired())
		loadTheme(_defaultConfig);

	return true;
}

void ThemeClassic::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		_screen.free();
		_initOk = false;
	}
}

void ThemeClassic::refresh() {
	init();
	_bgcolor = _system->RGBToColor(0, 0, 0);
	_color = _system->RGBToColor(104, 104, 104);
	_shadowcolor = _system->RGBToColor(64, 64, 64);
	_textcolor = _system->RGBToColor(32, 160, 32);
	_textcolorhi = _system->RGBToColor(0, 255, 0);
	if (_enabled)
		_system->showOverlay();
}

void ThemeClassic::enable() {
	init();
	resetDrawArea();
	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeClassic::disable() {
	_system->hideOverlay();
	_enabled = false;
}

void ThemeClassic::openDialog(bool topDialog) {
#ifdef OLDGUI_TRANSPARENCY
	if (!_dialog) {
		_dialog = new DialogState;
		assert(_dialog);
		// first dialog
		_dialog->screen.create(_screen.w, _screen.h, sizeof(OverlayColor));
	}
	memcpy(_dialog->screen.pixels, _screen.pixels, _screen.pitch*_screen.h);
	blendScreenToDialog();
#endif
}

void ThemeClassic::closeDialog() {
#ifdef OLDGUI_TRANSPARENCY
	if (_dialog) {
		_dialog->screen.free();
		delete _dialog;
		_dialog = 0;
	}
	_forceRedraw = true;
#endif
}

void ThemeClassic::clearAll() {
	if (!_initOk)
		return;
	_system->clearOverlay();
	// FIXME: problem with the 'pitch'
	_system->grabOverlay((OverlayColor*)_screen.pixels, _screen.w);
}

void ThemeClassic::drawAll() {
	_forceRedraw = false;
}

void ThemeClassic::resetDrawArea() {
	if (_initOk) {
		_drawArea = Common::Rect(0, 0, _screen.w, _screen.h);
	}
}

void ThemeClassic::drawDialogBackground(const Common::Rect &r, uint16 hints, kState state) {
	if (!_initOk)
		return;
	
	restoreBackground(r);
	
	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		addDirtyRect(r);
		return;
	}

	box(r.left, r.top, r.width(), r.height(), _color, _shadowcolor);
	addDirtyRect(r, (hints & THEME_HINT_SAVE_BACKGROUND) != 0);
}

void ThemeClassic::drawText(const Common::Rect &r, const Common::String &str, kState state, kTextAlign align, bool inverted, int deltax, bool useEllipsis, kFontStyle font) {
	if (!_initOk)
		return;

	if (!inverted) {
		restoreBackground(r);
		_font->drawString(&_screen, str, r.left, r.top, r.width(), getColor(state), convertAligment(align), deltax, useEllipsis);
	} else {
		_screen.fillRect(r, getColor(state));
		_font->drawString(&_screen, str, r.left, r.top, r.width(), _bgcolor, convertAligment(align), deltax, useEllipsis);
	}

	addDirtyRect(r);
}

void ThemeClassic::drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, kState state) {
	if (!_initOk)
		return;
	restoreBackground(r);
	font->drawChar(&_screen, ch, r.left, r.top, getColor(state));
	addDirtyRect(r);
}

void ThemeClassic::drawWidgetBackground(const Common::Rect &r, uint16 hints, kWidgetBackground background, kState state) {
	if (!_initOk || background == kWidgetBackgroundNo)
		return;

	restoreBackground(r);
	
	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		addDirtyRect(r);
		return;
	}

	switch (background) {
	case kWidgetBackgroundBorder:
		box(r.left, r.top, r.width(), r.height(), _color, _shadowcolor);
		break;

	//case kWidgetBackgroundPlain:
	//case kWidgetBackgroundBorderSmall:
	default:
		box(r.left, r.top, r.width(), r.height());
		break;
	};

	addDirtyRect(r, (hints & THEME_HINT_SAVE_BACKGROUND) != 0);
}

void ThemeClassic::drawButton(const Common::Rect &r, const Common::String &str, kState state) {
	if (!_initOk)
		return;
	restoreBackground(r);

	drawWidgetBackground(r, 0, kWidgetBackgroundBorder, state);

	const int off = (r.height() - _font->getFontHeight()) / 2;
	_font->drawString(&_screen, str, r.left, r.top+off, r.width(), getColor(state), Graphics::kTextAlignCenter, 0, false);

	addDirtyRect(r);
}

void ThemeClassic::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, kState state, bool transparency) {
	if (!_initOk)
		return;

	Common::Rect rect(r.left, r.top, r.left + surface.w, r.top + surface.h);
	rect.clip(_screen.w, _screen.h);

	if (!rect.isValidRect())
		return;
	
	assert(surface.bytesPerPixel == sizeof(OverlayColor));

	OverlayColor *src = (OverlayColor *)surface.pixels;
	OverlayColor *dst = (OverlayColor *)_screen.getBasePtr(rect.left, rect.top);

	int w = rect.width();
	int h = rect.height();

	while (h--) {
		memcpy(dst, src, surface.pitch);
		src += w;
		// FIXME: this should be pitch
		dst += _screen.w;
	}
	addDirtyRect(r);
}

void ThemeClassic::drawSlider(const Common::Rect &r, int width, kState state) {
	if (!_initOk)
		return;
	Common::Rect r2 = r;

	restoreBackground(r);

	box(r.left, r.top, r.width(), r.height(), _color, _shadowcolor);
	r2.left = r.left + 2;
	r2.top = r.top + 2;
	r2.bottom = r.bottom - 2;
	r2.right = r2.left + width;
	if (r2.right > r.right - 2) {
		r2.right = r.right - 2;
	}

	_screen.fillRect(r2, getColor(state));

	addDirtyRect(r);
}

void ThemeClassic::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, kState state, kTextAlign align) {
	if (!_initOk)
		return;

	restoreBackground(r);

	box(r.left, r.top, r.width(), r.height());

	Common::Point p0, p1;

	p0 = Common::Point(r.right + 1 - r.height() / 2, r.top + 4);
	p1 = Common::Point(r.right + 1 - r.height() / 2, r.bottom - 4);

	OverlayColor color = getColor(state);

	// Evil HACK to draw filled triangles
	// FIXME: The "big" version is pretty ugly.
	for (; p1.y - p0.y > 1; p0.y++, p0.x--, p1.y--, p1.x++) {
		_screen.drawLine(p0.x, p0.y, p1.x, p0.y, color);
		_screen.drawLine(p0.x, p1.y, p1.x, p1.y, color);
	}

	if (sel != "") {
		Common::Rect text(r.left + 2, r.top + 3, r.right - 4, r.top + 3 + _font->getFontHeight());
		_font->drawString(&_screen, sel, text.left, text.top, text.width(), color, convertAligment(align), deltax, false);
	}

	addDirtyRect(r);
}

void ThemeClassic::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, kState state) {
	if (!_initOk)
		return;

	Common::Rect r2 = r;
	int checkBoxSize = getFontHeight();
	if (checkBoxSize > r.height()) {
		checkBoxSize = r.height();
	}
	r2.bottom = r2.top + checkBoxSize;

	restoreBackground(r2);

	box(r.left, r.top, checkBoxSize, checkBoxSize, _color, _shadowcolor);

	if (checked) {
		r2.top += 3;
		r2.bottom = r.top + checkBoxSize - 4;
		r2.left += 3;
		r2.right = r.left + checkBoxSize - 4;

		OverlayColor c = getColor(state);

		// Draw a cross
		_screen.drawLine(r2.left, r2.top, r2.right, r2.bottom, c);
		_screen.drawLine(r2.left, r2.bottom, r2.right, r2.top, c);

		if (r2.height() > 5) {
			// Thicken the lines
			_screen.drawLine(r2.left, r2.top + 1, r2.right - 1, r2.bottom, c);
			_screen.drawLine(r2.left + 1, r2.top, r2.right, r2.bottom - 1, c);
			_screen.drawLine(r2.left, r2.bottom - 1, r2.right - 1, r2.top, c);
			_screen.drawLine(r2.left + 1, r2.bottom, r2.right, r2.top + 1, c);
		}

		r2 = r;
	}

	r2.left += checkBoxSize + 10;
	_font->drawString(&_screen, str, r2.left, r2.top, r2.width(), getColor(state), Graphics::kTextAlignLeft, 0, true);

	addDirtyRect(r);
}

void ThemeClassic::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, kState state) {
	if (!_initOk)
		return;
	restoreBackground(r);

	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;
		box(r.left + i * tabWidth, r.top+2, tabWidth, tabHeight-2, _color, _shadowcolor);
		_font->drawString(&_screen, tabs[i], r.left + i * tabWidth, r.top+4, tabWidth, getColor(state), Graphics::kTextAlignCenter, 0, true);
	}
	
	box(r.left + active * tabWidth, r.top, tabWidth, tabHeight, _color, _shadowcolor, true);
	_font->drawString(&_screen, tabs[active], r.left + active * tabWidth, r.top+2, tabWidth, getColor(kStateHighlight), Graphics::kTextAlignCenter, 0, true);

	_screen.hLine(r.left, r.top + tabHeight, r.left + active * tabWidth + 1, _color);
	_screen.hLine(r.left + active * tabWidth + tabWidth - 2, r.top + tabHeight, r.right, _color);
	_screen.hLine(r.left, r.bottom - 1, r.right - 1, _shadowcolor);
	_screen.vLine(r.left, r.top + tabHeight, r.bottom - 1, _color);
	_screen.vLine(r.right - 1, r.top + tabHeight, r.bottom - 1, _shadowcolor);
	
	addDirtyRect(r);
}

void ThemeClassic::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, kScrollbarState scroll, kState state) {
	if (!_initOk)
		return;
	restoreBackground(r);
	Common::Rect r2 = r;
	box(r.left, r.top, r.width(), r.height(), _color, _shadowcolor);

	const int UP_DOWN_BOX_HEIGHT = r.width() + 1;
	const int B = 3;
	const int arrowSize = (r.width() / 2 - B + 1);

	OverlayColor color = 0;
	if (scroll == kScrollbarStateSinglePage) {
		color = _color;
	} else if (scroll == kScrollbarStateUp && state == kStateHighlight) {
		color = _textcolorhi;
	} else {
		color = _textcolor;
	}

	// draws the 'up' button
	box(r.left, r.top, r.width(), UP_DOWN_BOX_HEIGHT, _color, _shadowcolor);
	Common::Point p0 = Common::Point(r.left + r.width() / 2, r.top + (UP_DOWN_BOX_HEIGHT - arrowSize - 1) / 2);
	Common::Point p1 = Common::Point(p0.x - arrowSize, p0.y + arrowSize);
	Common::Point p2 = Common::Point(p0.x + arrowSize, p0.y + arrowSize);
	for (; p1.x <= p2.x; ++p1.x)
		_screen.drawLine(p0.x, p0.y, p1.x, p1.y, color);

	if (scroll != kScrollbarStateSinglePage) {
		r2.top += sliderY;
		r2.left += 2;
		r2.right -= 2;
		r2.bottom = r2.top + sliderHeight;
		_screen.fillRect(r2, (state == kStateHighlight && scroll == kScrollbarStateSlider) ? _textcolorhi : _textcolor);
		box(r2.left, r2.top, r2.width(), r2.height());
		int y = r2.top + sliderHeight / 2;
		color = (state == kStateHighlight && scroll == kScrollbarStateSlider) ? _color : _bgcolor;
		_screen.hLine(r2.left + 1, y - 2, r2.right - 2, color);
		_screen.hLine(r2.left + 1, y, r2.right - 2, color);
		_screen.hLine(r2.left + 1, y + 2, r2.right - 2, color);
		r2 = r;
	}

	r2.top = r2.bottom - UP_DOWN_BOX_HEIGHT;
	if (scroll == kScrollbarStateSinglePage) {
		color = _color;
	} else if (scroll == kScrollbarStateDown && state == kStateHighlight) {
		color = _textcolorhi;
	} else {
		color = _textcolor;
	}

	// draws the 'down' button
	box(r2.left, r2.top, r2.width(), UP_DOWN_BOX_HEIGHT, _color, _shadowcolor);
	p0 = Common::Point(r2.left + r2.width() / 2, r2.top + (UP_DOWN_BOX_HEIGHT + arrowSize + 1) / 2);
	p1 = Common::Point(p0.x - arrowSize, p0.y - arrowSize);
	p2 = Common::Point(p0.x + arrowSize, p0.y - arrowSize);
	for (; p1.x <= p2.x; ++p1.x)
		_screen.drawLine(p0.x, p0.y, p1.x, p1.y, color);

	addDirtyRect(r);
}

void ThemeClassic::drawCaret(const Common::Rect &r, bool erase, kState state) {
	if (!_initOk)
		return;

	OverlayColor color = 0;
	if (erase) {
		color = _bgcolor;
	} else {
		color = getColor(state);
	}

	_screen.vLine(r.left, r.top, r.bottom - 2, color);
	addDirtyRect(r);
}

void ThemeClassic::drawLineSeparator(const Common::Rect &r, kState state) {
	if (!_initOk)
		return;
	_screen.hLine(r.left - 1, r.top + r.height() / 2, r.right, _shadowcolor);
	_screen.hLine(r.left, r.top + 1 + r.height() / 2, r.right, _color);
	addDirtyRect(r);
}

// intern drawing

void ThemeClassic::restoreBackground(Common::Rect r, bool special) {
	r.clip(_screen.w, _screen.h);
	r.clip(_drawArea);
#ifndef OLDGUI_TRANSPARENCY
	_screen.fillRect(r, _bgcolor);
#else
	if (_dialog) {
		if (!_dialog->screen.pixels) {
			_screen.fillRect(r, _bgcolor);
			return;
		}
		const OverlayColor *src = (const OverlayColor*)_dialog->screen.getBasePtr(r.left, r.top);
		OverlayColor *dst = (OverlayColor*)_screen.getBasePtr(r.left, r.top);

		int h = r.height();
		int w = r.width();
		while (h--) {
			memcpy(dst, src, w*sizeof(OverlayColor));
			src += _dialog->screen.w;
			dst += _screen.w;
		}
	} else {
		_screen.fillRect(r, _bgcolor);
	}
#endif
}

bool ThemeClassic::addDirtyRect(Common::Rect r, bool save, bool special) {
	// TODO: implement proper dirty rect handling
	// FIXME: problem with the 'pitch'
	r.clip(_screen.w, _screen.h);
	r.clip(_drawArea);
	_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(r.left, r.top), _screen.w, r.left, r.top, r.width(), r.height());
#ifdef OLDGUI_TRANSPARENCY
	if (_dialog && save) {
		if (_dialog->screen.pixels) {
			OverlayColor *dst = (OverlayColor*)_dialog->screen.getBasePtr(r.left, r.top);
			const OverlayColor *src = (const OverlayColor*)_screen.getBasePtr(r.left, r.top);
			int h = r.height();
			while (h--) {
				memcpy(dst, src, r.width()*sizeof(OverlayColor));
				dst += _dialog->screen.w;
				src += _screen.w;
			}
		}
	}
#endif
	return true;
}

void ThemeClassic::box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB, bool skipLastRow) {
	if (y >= 0) {
		_screen.hLine(x + 1, y, x + width - 2, colorA);
		_screen.hLine(x, y + 1, x + width - 1, colorA);
	}
	int drawY = y;
	if (drawY < 0) {
		height += drawY;
		drawY = 0;
	}
	_screen.vLine(x, drawY + 1, drawY + height - 2, colorA);
	_screen.vLine(x + 1, drawY, drawY + height - 1, colorA);
	_screen.vLine(x + width - 1, drawY + 1, drawY + height - 2, colorB);
	_screen.vLine(x + width - 2, drawY + 1, drawY + height - 1, colorB);

	if (y + height >= 0 && !skipLastRow) {
		_screen.hLine(x + 1, drawY + height - 2, x + width - 1, colorB);
		_screen.hLine(x + 1, drawY + height - 1, x + width - 2, colorB);
	}
}

void ThemeClassic::box(int x, int y, int w, int h) {
	_screen.hLine(x, y, x + w - 1, _color);
	_screen.hLine(x, y + h - 1, x +w - 1, _shadowcolor);
	_screen.vLine(x, y, y + h - 1, _color);
	_screen.vLine(x + w - 1, y, y + h - 1, _shadowcolor);
}

OverlayColor ThemeClassic::getColor(kState state) {
	OverlayColor usedColor = _color;
	switch (state) {
	case kStateEnabled:
		usedColor = _textcolor;
		break;

	case kStateHighlight:
		usedColor = _textcolorhi;
		break;

	default:
		break;
	}
	return usedColor;
}

#ifdef OLDGUI_TRANSPARENCY
void ThemeClassic::blendScreenToDialog() {
	Common::Rect rect(0, 0, _screen.w, _screen.h);

	if (!rect.isValidRect())
		return;

	if (_system->hasFeature(OSystem::kFeatureOverlaySupportsAlpha)) {
		int a, r, g, b;
		uint8 aa, ar, ag, ab;
		_system->colorToARGB(_bgcolor, aa, ar, ag, ab);
		a = aa*3/(3+1);
		if (a < 1)
			return;
		r = ar * a;
		g = ag * a;
		b = ab * a;

		OverlayColor *ptr = (OverlayColor*)_dialog->screen.getBasePtr(rect.left, rect.top);
		
		int h = rect.height();
		int w = rect.width();
		while (h--) {
			for (int i = 0; i < w; i++) {
				_system->colorToARGB(ptr[i], aa, ar, ag, ab);
				int a2 = aa + a - (a*aa)/255;
				ptr[i] = _system->ARGBToColor(a2,
							      ((255-a)*aa*ar/255+r)/a2,
							      ((255-a)*aa*ag/255+g)/a2,
							      ((255-a)*aa*ab/255+b)/a2);
			}
			ptr += _screen.w;
		}
	} else {
		int r, g, b;
		uint8 ar, ag, ab;
		_system->colorToRGB(_bgcolor, ar, ag, ab);
		r = ar * 3;
		g = ag * 3;
		b = ab * 3;

		OverlayColor *ptr = (OverlayColor*)_dialog->screen.getBasePtr(rect.left, rect.top);
		
		int h = rect.height();
		int w = rect.width();

		while (h--) {
			for (int i = 0; i < w; i++) {
				_system->colorToRGB(ptr[i], ar, ag, ab);
				ptr[i] = _system->RGBToColor((ar + r) / (3+1),
							     (ag + g) / (3+1),
							     (ab + b) / (3+1));
			}
			ptr += _screen.w;
		}
	}
}
#endif
} // end of namespace GUI

