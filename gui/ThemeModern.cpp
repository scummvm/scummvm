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
 * $URL$
 * $Id$
 */

#ifndef DISABLE_FANCY_THEMES

#include "gui/ThemeModern.h"
#include "gui/eval.h"

#include "graphics/imageman.h"
#include "graphics/imagedec.h"
#include "graphics/colormasks.h"
#include "graphics/cursorman.h"

#include "common/config-manager.h"
#include "common/file.h"

#define kShadowTr0 8
#define kShadowTr1 16
#define kShadowTr2 32
#define kShadowTr3 64
#define kShadowTr35 96
#define kShadowTr4 128
#define kShadowTr5 192

using Graphics::Surface;

/** Specifies the currently active 16bit pixel format, 555 or 565. */
extern int gBitFormat;

namespace GUI {

// TODO: This should be moved to ThemeModern
OverlayColor getColorAlpha(OverlayColor col1, OverlayColor col2, int alpha);
OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos, int max, uint factor);
void getStateColor(OverlayColor &s, OverlayColor &e, OverlayColor enabledS, OverlayColor enabledE, OverlayColor highlightS, OverlayColor highlightE, Theme::WidgetStateInfo state);

#pragma mark -

ThemeModern::ThemeModern(OSystem *system, const Common::String &stylefile, const Common::ConfigFile *cfg) : Theme(), _system(system), _screen(), _initOk(false),
_forceRedraw(false), _lastUsedBitMask(0), _fonts(), _cursor(0), _imageHandles(), _images(0), _colors(), _gradientFactors() {
	_stylefile = stylefile;
	_initOk = false;
	_enabled = false;
	_useCursor = false;
	memset(&_screen, 0, sizeof(_screen));
	memset(&_dialog, 0, sizeof(_dialog));
	memset(&_colors, 0, sizeof(_colors));
	memset(&_gradientFactors, 0, sizeof(_gradientFactors));

	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	if (_screen.pixels) {
		_initOk = true;
		clearAll();
	}

	if (cfg) {
		_configFile = *cfg;
	} else {
		if (!loadConfigFile(stylefile)) {
			warning("Can not find theme config file '%s'", (stylefile + ".ini").c_str());
			return;
		}
	}

	ImageMan.addArchive(stylefile + ".zip");

	Common::String temp;
	_configFile.getKey("version", "theme", temp);
	if (atoi(temp.c_str()) != THEME_VERSION) {
		// TODO: improve this detection and handle it nicer
		warning("Theme config uses a different version (you have: '%s', needed is: '%d')", temp.c_str(), THEME_VERSION);
		return;
	}

	temp.clear();
	_configFile.getKey("type", "theme", temp);
	if (0 != temp.compareToIgnoreCase("modern")) {
		warning("Theme config is not for the modern style theme");
		return;
	}

	if (_configFile.hasKey("name", "theme"))
		_configFile.getKey("name", "theme", _stylename);
	else
		_stylename = _stylefile;

	_images = new const Graphics::Surface*[kImageHandlesMax];
	assert(_images);

	_lastUsedBitMask = gBitFormat;
}

ThemeModern::~ThemeModern() {
	deleteFonts();
	deinit();
	delete[] _images;
	delete[] _cursor;
	_images = 0;
	for (int i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.unregisterSurface(_imageHandles[i]);
	}
	ImageMan.remArchive(_stylefile + ".zip");
}

bool ThemeModern::init() {
	if (!_images)
		return false;

	deinit();
	_screen.create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(OverlayColor));
	if (_screen.pixels) {
		_initOk = true;
		clearAll();
		resetDrawArea();
	}

	if (isThemeLoadingRequired()) {
		loadTheme(_defaultConfig);
		loadTheme(_configFile, false, true); // Don't reset

		processExtraValues();
	}

	for (int i = 0; i < kImageHandlesMax; ++i) {
		if (!_images[i]) {
			return false;
		}
	}

	return true;
}

void ThemeModern::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		_screen.free();
		_initOk = false;
	}
}

void ThemeModern::refresh() {
	init();
	resetupGuiRenderer();
	if (_enabled) {
		_system->showOverlay();
		CursorMan.replaceCursorPalette(_cursorPal, 0, MAX_CURS_COLORS);
		CursorMan.replaceCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, 255, _cursorTargetScale);
	}
}

void ThemeModern::enable() {
	init();
	resetupGuiRenderer();
	resetDrawArea();
	if (_useCursor)
		setUpCursor();
	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeModern::disable() {
	_system->hideOverlay();
	if (_useCursor) {
		CursorMan.popCursorPalette();
		CursorMan.popCursor();
	}
	_enabled = false;
}

void ThemeModern::openDialog(bool topDialog) {
	if (!_dialog) {
		_dialog = new DialogState;
		assert(_dialog);
		// first dialog
		_dialog->screen.create(_screen.w, _screen.h, sizeof(OverlayColor));
	}

	if (_dialogShadingCallback && topDialog) {
		OverlayColor *col = (OverlayColor*)_screen.pixels;
		for (int y = 0; y < _screen.h; ++y) {
			for (int x = 0; x < _screen.w; ++x) {
				col[x] = (this->*(_dialogShadingCallback))(col[x]);
			}
			col += _screen.w;
		}
	}

	memcpy(_dialog->screen.pixels, _screen.pixels, _screen.pitch*_screen.h);

	if ((_dialogShadingCallback) && topDialog)
		addDirtyRect(Common::Rect(0, 0, _screen.w, _screen.h), false, false);
}

void ThemeModern::closeAllDialogs() {
	if (_dialog) {
		_dialog->screen.free();
		delete _dialog;
		_dialog = 0;
	}
	_forceRedraw = true;
}

void ThemeModern::clearAll() {
	if (!_initOk)
		return;
	_system->clearOverlay();
	// FIXME: problem with the 'pitch'
	_system->grabOverlay((OverlayColor*)_screen.pixels, _screen.w);
}

void ThemeModern::updateScreen() {
	// TODO: see ThemeModern::addDirtyRect
	_forceRedraw = false;
}

void ThemeModern::setDrawArea(const Common::Rect &r) {
	if (_initOk) {
		_drawArea = r;
		_shadowDrawArea = Common::Rect(r.left-_shadowLeftWidth, r.top-_shadowTopHeight, r.right+_shadowRightWidth, r.bottom+_shadowBottomHeight);
		_drawArea.clip(_screen.w, _screen.h);
		_shadowDrawArea.clip(_screen.w, _screen.h);
	}
}

void ThemeModern::resetDrawArea() {
	if (_initOk) {
		_drawArea = Common::Rect(0, 0, _screen.w, _screen.h);
		_shadowDrawArea = _drawArea;
	}
}

#define surface(x) (_images[x])

void ThemeModern::drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state) {
	if (!_initOk)
		return;

	Common::Rect r2 = shadowRect(r, kShadowFull);

	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		restoreBackground(r2, true);
		addDirtyRect(r2);
		return;
	}

	if (hints & THEME_HINT_MAIN_DIALOG) {
		colorFade(r, _colors[kMainDialogStart], _colors[kMainDialogEnd], _gradientFactors[kMainDialogFactor]);
	} else if (hints & THEME_HINT_SPECIAL_COLOR) {
		drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowFull);

		drawRectMasked(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd),
			256, _colors[kMainDialogStart], _colors[kMainDialogEnd], _gradientFactors[kDialogSpecialFactor]);
	} else if (hints & THEME_HINT_PLAIN_COLOR) {
		drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowFull);

		drawRectMasked(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd),
			256, _colors[kDialogEnd], _colors[kDialogEnd], _gradientFactors[kDialogFactor]);
	} else {
		drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowFull);

		drawRectMasked(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd),
				256, _colors[kDialogStart], _colors[kDialogEnd], _gradientFactors[kDialogFactor]);
	}

	addDirtyRect(r2, (hints & THEME_HINT_SAVE_BACKGROUND) != 0, true);
}

void ThemeModern::drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font) {
	if (!_initOk)
		return;

	uint32 color;

	restoreBackground(r);

	if (inverted) {
		_screen.fillRect(r, _colors[kTextInvertedBackground]);
		color = _colors[kTextInvertedColor];
	} else {
		color = getColor(state);
	}

	getFont(font)->drawString(&_screen, str, r.left, r.top, r.width(), color, convertAligment(align), deltax, useEllipsis);
	addDirtyRect(r);
}

void ThemeModern::drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state) {
	if (!_initOk)
		return;
	restoreBackground(r);
	font->drawChar(&_screen, ch, r.left, r.top, getColor(state));
	addDirtyRect(r);
}

void ThemeModern::drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state) {
	if (!_initOk)
		return;

	Common::Rect r2;

	ImageHandles corner, top, left, bkgd;
	ShadowStyles shadow;
	ColorHandles start, end;
	GradientFactors factor;

	switch (background) {
	case kWidgetBackgroundBorderSmall:
		corner = kWidgetSmallBkgdCorner;
		top = kWidgetSmallBkgdTop;
		left = kWidgetSmallBkgdLeft;
		bkgd = kWidgetSmallBkgd;
		shadow = kShadowSmall;
		start = kWidgetBackgroundSmallStart;
		end = kWidgetBackgroundSmallEnd;
		factor = kWidgetSmallFactor;
		break;
	case kWidgetBackgroundEditText:
		corner = kEditTextBkgdCorner;
		top = kEditTextBkgdTop;
		left = kEditTextBkgdLeft;
		bkgd = kEditTextBkgd;
		shadow = kShadowEmboss;
		start = kEditTextBackgroundStart;
		end = kEditTextBackgroundEnd;
		factor = kEditTextFactor;
		break;
	case kWidgetBackgroundSlider:
		corner = kSliderBkgdCorner;
		top = kSliderBkgdTop;
		left = kSliderBkgdLeft;
		bkgd = kSliderBkgd;
		shadow = kShadowEmboss;
		start = kSliderBackgroundStart;
		end = kSliderBackgroundEnd;
		factor = kSliderBackground;
		break;
	default:
		corner = kWidgetBkgdCorner;
		top = kWidgetBkgdTop;
		left = kWidgetBkgdLeft;
		bkgd = kWidgetBkgd;
		shadow = kShadowFull;
		if (hints & THEME_HINT_PLAIN_COLOR)
			start = kWidgetBackgroundEnd;
		else
			start = kWidgetBackgroundStart;
		end = kWidgetBackgroundEnd;
		factor = kWidgetFactor;
		break;
	}

	if ((hints & THEME_HINT_SAVE_BACKGROUND) && !(hints & THEME_HINT_FIRST_DRAW) && !_forceRedraw) {
		restoreBackground((hints & THEME_HINT_USE_SHADOW) ? r2 : r);
		addDirtyRect((hints & THEME_HINT_USE_SHADOW) ? r2 : r);
		return;
	}

	if ((hints & THEME_HINT_USE_SHADOW)) {
		r2 = shadowRect(r, shadow);
		restoreBackground(r2);
		// shadow
		drawShadow(r, surface(corner), surface(top), surface(left), surface(bkgd), shadow);
	}

	drawRectMasked(r, surface(corner), surface(top), surface(left), surface(bkgd),
				   (state == kStateDisabled) ? -30 : 256, _colors[start], _colors[end],
				   _gradientFactors[factor]);

	addDirtyRect((hints & THEME_HINT_USE_SHADOW) ? r2 : r, (hints & THEME_HINT_SAVE_BACKGROUND) != 0);
}

void ThemeModern::drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints) {
	if (!_initOk)
		return;

	Common::Rect r2 = shadowRect(r, kShadowButton);

	if (!(hints & THEME_HINT_NO_BACKGROUND_RESTORE) || state == kStateDisabled)
		restoreBackground(r2);

	// shadow
	drawShadow(r, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd), kShadowButton);

	OverlayColor start, end;
	int alpha = 256;

	getStateColor(start, end, _colors[kButtonBackgroundStart], _colors[kButtonBackgroundEnd],
				_colors[kButtonBackgroundHighlightStart], _colors[kButtonBackgroundHighlightEnd], state);

	if (state != kStateHighlight)
		alpha = (state == kStateDisabled) ? -30 : 256;

	drawRectMasked(r, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd),
					alpha, start, end, _gradientFactors[kButtonFactor]);

	const int off = (r.height() - getFontHeight()) / 2;

	OverlayColor col = 0;
	switch (state) {
	case kStateEnabled:
		col = _colors[kButtonTextEnabled];
		break;

	case kStateHighlight:
		col = _colors[kButtonTextHighlight];
		break;

	default:
		col = _colors[kButtonTextDisabled];
		break;
	};

	getFont()->drawString(&_screen, str, r.left, r.top + off, r.width(), col, Graphics::kTextAlignCenter, 0, true);

	addDirtyRect(r2);
}

void ThemeModern::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans) {
	if (!_initOk)
		return;

	Common::Rect rect(r.left, r.top, r.left + surface.w, r.top + surface.h);
	rect.clip(_screen.w, _screen.h);

	if (!rect.isValidRect())
		return;

	assert(surface.bytesPerPixel == sizeof(OverlayColor));

	if (alpha != 256)
		restoreBackground(rect);

	if (themeTrans)
		drawSurface(rect, &surface, false, false, alpha);
	else {
		OverlayColor *dst = (OverlayColor*)_screen.getBasePtr(rect.left, rect.top);
		const OverlayColor *src = (OverlayColor*)surface.pixels;

		int h = rect.height();
		if (alpha == 256) {
			while (h--) {
				memcpy(dst, src, surface.pitch);
				dst += _screen.w;
				src += surface.w;
			}
		} else {
			int w = rect.width();
			while (h--) {
				for (int i = 0; i < w; ++i) {
					*dst = getColorAlpha(*src, *dst, alpha);
				}
				dst += _screen.w;
				src += surface.w;
			}
		}
	}

	addDirtyRect(rect);
}

void ThemeModern::drawSlider(const Common::Rect &rr, int width, WidgetStateInfo state) {
	if (!_initOk)
		return;

	Common::Rect r = rr;

	r.left++;
	r.right++;

	drawWidgetBackground(r, THEME_HINT_USE_SHADOW, kWidgetBackgroundSlider, kStateEnabled);

	Common::Rect r2 = r;
	r2.left = r.left;
	r2.top = r.top;
	r2.bottom = r.bottom-1;
	r2.right = r2.left + width-1;
	if (r2.right > r.right) {
		r2.right = r.right;
	}

	drawShadow(r2, surface(kButtonBkgdCorner), surface(kButtonBkgdTop), surface(kButtonBkgdLeft), surface(kButtonBkgd), kShadowButton);

	OverlayColor start, end;
	int alpha = 256;

	getStateColor(start, end, _colors[kSliderStart], _colors[kSliderEnd], _colors[kSliderHighStart], _colors[kSliderHighEnd], state);

	if (state != kStateHighlight)
		alpha = (state == kStateDisabled) ? -30 : 256;

	drawRectMasked(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd),
				alpha, start, end, _gradientFactors[kSliderFactor]);

	addDirtyRect(r);
}

void ThemeModern::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align) {
	if (!_initOk)
		return;

	Common::Rect r2 = shadowRect(r, kShadowSmall);
	restoreBackground(r2);

	OverlayColor start = _colors[kPopUpWidgetStart], end = _colors[kPopUpWidgetEnd];
	if (state == kStateHighlight) {
		start = _colors[kPopUpWidgetHighlightStart];
		end = _colors[kPopUpWidgetHighlightEnd];
	}

	drawShadow(r, surface(kDialogBkgdCorner), surface(kDialogBkgdTop), surface(kDialogBkgdLeft), surface(kDialogBkgd), kShadowPopUp);

	drawRectMasked(r, surface(kPopUpWidgetBkgdCorner), surface(kPopUpWidgetBkgdTop), surface(kPopUpWidgetBkgdLeft), surface(kPopUpWidgetBkgd),
						(state == kStateDisabled) ? -30 : 256, start, end, _gradientFactors[kPopUpWidgetFactor]);

	const Graphics::Surface *arrow = surface(kWidgetArrow);

	int yOff = r.height() / 2 - arrow->h;
	if (yOff < 0)
		yOff = 0;

	Common::Rect arrowRect(r.right - 4 - arrow->w, r.top + yOff, r.right - 4, r.top + yOff + arrow->h);
	arrowRect.clip(r);

	drawSurface(arrowRect, arrow, false, false, (state == kStateDisabled) ? -30 : 256);

	arrowRect.top += arrow->h;
	arrowRect.bottom += arrow->h;
	arrowRect.clip(r);
	drawSurface(arrowRect, arrow, true, false, (state == kStateDisabled) ? -30 : 256);

	if (!sel.empty()) {
		Common::Rect text(r.left + 2, r.top + 3, r.right - 4, r.top + 3 + getFont()->getFontHeight());
		getFont()->drawString(&_screen, sel, text.left, text.top, text.width(), getColor(state), convertAligment(align), deltax, false);
	}

	addDirtyRect(r2);
}

void ThemeModern::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!_initOk)
		return;
	Common::Rect r2 = r;

	const Graphics::Surface *checkBox = surface(checked ? kCheckboxChecked : kCheckboxEmpty);
	int checkBoxSize = checkBox->w;

	restoreBackground(Common::Rect(r.left, r.top, r.left+checkBox->w, r.top+checkBox->h));

	drawSurface(Common::Rect(r.left, r.top, r.left+checkBox->w, r.top+checkBox->h), checkBox, false, false, (state == kStateDisabled) ? 128 : 256);

	r2.left += checkBoxSize + 5;
	getFont()->drawString(&_screen, str, r2.left, r2.top, r2.width(), getColor(state), Graphics::kTextAlignLeft, 0, false);

	addDirtyRect(r);
}

void ThemeModern::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state) {
	if (!_initOk)
		return;

	restoreBackground(r);

	// whole tab background
	drawRectMasked(r, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
						/*(state == kStateDisabled) ? -30 : */256, _colors[kTabBackgroundStart], _colors[kTabBackgroundEnd],
						_gradientFactors[kTabFactor]);

	OverlayColor tabEnd = calcGradient(_colors[kTabActiveStart], _colors[kTabActiveEnd], tabHeight, r.height()-1, _gradientFactors[kTabFactor]);

	const int tabOffset = 1;

	// tab shadows
	for (int i = 0; i < (int)tabs.size(); ++i) {
		Common::Rect tabRect(r.left + i * (tabWidth + tabOffset), r.top, r.left + i * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		drawShadow(tabRect, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
					kShadowSmall, true);
	}

	// inactive tabs
	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;

		Common::Rect tabRect(r.left + i * (tabWidth + tabOffset), r.top, r.left + i * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight + 5);
		drawRectMasked(tabRect, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
					256, _colors[kTabInactiveStart], _colors[kTabInactiveEnd], _gradientFactors[kTabFactor], true);

		getFont()->drawString(&_screen, tabs[i], tabRect.left, tabRect.top+titleVPad, tabRect.width(), getColor(kStateEnabled), Graphics::kTextAlignCenter, 0, true);
	}

	// area shadow
	Common::Rect widgetBackground = Common::Rect(r.left, r.top + tabHeight - 1, r.right, r.top
                 + tabHeight + 20);
	drawShadow(widgetBackground, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
					kShadowSmall);

	// area itself
	widgetBackground = Common::Rect(r.left, r.top + tabHeight, r.right, r.bottom);
	drawRectMasked(widgetBackground, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
						/*(state == kStateDisabled) ? -30 : */256, tabEnd, _colors[kTabActiveEnd],
						_gradientFactors[kTabFactor]);
	addDirtyRect(widgetBackground, true);

	// active tab
	if (active >= 0) {
		Common::Rect tabRect(r.left + active * (tabWidth + tabOffset), r.top, r.left + active * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight + 5);

		Common::Rect shadowRect2(tabRect.left, r.top, tabRect.right, tabRect.bottom - 6);
		drawShadow(shadowRect2, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd), kShadowSmall, false, true);

		drawRectMasked(tabRect, surface(kTabBkgdCorner), surface(kTabBkgdTop), surface(kTabBkgdLeft), surface(kTabBkgd),
				256, _colors[kTabActiveStart], tabEnd, _gradientFactors[kTabFactor], true);


		getFont()->drawString(&_screen, tabs[active], tabRect.left, tabRect.top+titleVPad, tabRect.width(), getColor(kStateEnabled), Graphics::kTextAlignCenter, 0, true);
	}

	addDirtyRect(Common::Rect(r.left, r.top-2, r.right, r.bottom));
}

void ThemeModern::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState scrollState, WidgetStateInfo state) {
	if (!_initOk)
		return;
	const int UP_DOWN_BOX_HEIGHT = r.width() + 1;
	Common::Rect r2 = r;

	// draws the scrollbar background
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					_colors[kScrollbarBackgroundStart], _colors[kScrollbarBackgroundEnd], _gradientFactors[kScrollbarBkgdFactor]);

	// draws the 'up' button
	OverlayColor buttonStart = _colors[kScrollbarButtonStart];
	OverlayColor buttonEnd = _colors[kScrollbarButtonEnd];

	if (scrollState == kScrollbarStateUp)
		getStateColor(buttonStart, buttonEnd, buttonStart, buttonEnd, _colors[kScrollbarButtonHighlightStart], _colors[kScrollbarButtonHighlightEnd], state);

	r2.bottom = r2.top + UP_DOWN_BOX_HEIGHT;
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					buttonStart, buttonEnd, _gradientFactors[kScrollbarBkgdFactor]);

	const Graphics::Surface *arrow = surface(kWidgetArrow);
	r2.left += 1 + (r2.width() - arrow->w) / 2;
	r2.right = r2.left + arrow->w;
	r2.top += (r2.height() - arrow->h) / 2;
	r2.bottom = r2.top + arrow->h;
	drawSurface(r2, arrow, false, false, 256);

	// draws the slider
	OverlayColor sliderStart = _colors[kScrollbarSliderStart];
	OverlayColor sliderEnd = _colors[kScrollbarSliderEnd];

	if (scrollState == kScrollbarStateSlider)
		getStateColor(sliderStart, sliderEnd, sliderStart, sliderEnd, _colors[kScrollbarSliderHighlightStart], _colors[kScrollbarSliderHighlightEnd], state);

	r2 = r;
	r2.left += 1;
	r2.right -= 1;
	r2.top += sliderY;
	r2.bottom = r2.top + sliderHeight - 1;

	drawShadow(r2, surface(kSliderCorner), surface(kSliderTop), surface(kSliderLeft), surface(kSliderBkgd), kShadowSmall);

	r2.left += 1;
	r2.right -= 1;

	r2.bottom = r2.top + sliderHeight / 2 + surface(kScrollbarCorner)->h + 4;
	drawRectMasked(r2, surface(kScrollbarCorner), surface(kScrollbarTop), surface(kScrollbarLeft), surface(kScrollbarBkgd), 256,
					sliderStart, sliderEnd, _gradientFactors[kScrollbarFactor]);
	r2.top += sliderHeight / 2;
	r2.bottom += sliderHeight / 2 - surface(kScrollbarCorner)->h - 4;
	drawRectMasked(r2, surface(kScrollbarCorner), surface(kScrollbarTop), surface(kScrollbarLeft), surface(kScrollbarBkgd), 256,
					sliderEnd, sliderStart, _gradientFactors[kScrollbarFactor]);

	// draws the 'down' button
	buttonStart = _colors[kScrollbarButtonStart];
	buttonEnd = _colors[kScrollbarButtonEnd];

	if (scrollState == kScrollbarStateDown)
		getStateColor(buttonStart, buttonEnd, buttonStart, buttonEnd, _colors[kScrollbarButtonHighlightStart], _colors[kScrollbarButtonHighlightEnd], state);

	r2 = r;
	r2.top = r2.bottom - UP_DOWN_BOX_HEIGHT;
	drawRectMasked(r2, surface(kScrollbarBkgdCorner), surface(kScrollbarBkgdTop), surface(kScrollbarBkgdLeft), surface(kScrollbarBkgd), 256,
					buttonStart, buttonEnd, _gradientFactors[kScrollbarBkgdFactor]);

	r2.left += 1 + (r2.width() - arrow->w) / 2;
	r2.right = r2.left + arrow->w;
	r2.top += (r2.height() - arrow->h) / 2;
	r2.bottom = r2.top + arrow->h;
	drawSurface(r2, arrow, true, false, 256);

	addDirtyRect(r);
}

void ThemeModern::drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state) {
	if (!_initOk)
		return;

	restoreBackground(r);
	if (!erase) {
		_screen.vLine(r.left, r.top, r.bottom-1, _colors[kCaretColor]);
	} else if (r.top >= 0) {
		// FIXME: hack to restore the caret background correctly
		const OverlayColor search = _colors[kTextInvertedBackground];
		const OverlayColor *src = (const OverlayColor*)_screen.getBasePtr(r.left-1, r.top-1);
		int height = r.height() + 2;
		if (r.top + height > _screen.h) {
			height = _screen.h - r.top;
		}
		bool drawInvBackground = false;
		while (height--) {
			if (src[0] == search || src[1] == search || src[2] == search) {
				drawInvBackground = true;
			}
			src += _screen.w;
		}
		if (drawInvBackground) {
			_screen.vLine(r.left, r.top, r.bottom-1, search);
		}
	}
	addDirtyRect(r);
}

void ThemeModern::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!_initOk)
		return;
	_screen.hLine(r.left - 1, r.top + r.height() / 2, r.right, _system->RGBToColor(0, 0, 0));
	addDirtyRect(r);
}

int ThemeModern::getTabSpacing() const {
	return 0;
}
int ThemeModern::getTabPadding() const {
	return 3;
}

#pragma mark - intern drawing

void ThemeModern::restoreBackground(Common::Rect r, bool special) {
	r.clip(_screen.w, _screen.h);
	if (special) {
		r.clip(_shadowDrawArea);
	} else {
		r.clip(_drawArea);
	}
	if (_dialog) {
		if (!_dialog->screen.pixels) {
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
	}
}

bool ThemeModern::addDirtyRect(Common::Rect r, bool backup, bool special) {
	// TODO: implement proper dirty rect handling
	// FIXME: problem with the 'pitch'
	r.clip(_screen.w, _screen.h);
	if (special) {
		r.clip(_shadowDrawArea);
	} else {
		r.clip(_drawArea);
	}
	_system->copyRectToOverlay((OverlayColor*)_screen.getBasePtr(r.left, r.top), _screen.w, r.left, r.top, r.width(), r.height());
	if (_dialog && backup) {
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
	return true;
}

void ThemeModern::colorFade(const Common::Rect &r, OverlayColor start, OverlayColor end, uint factor) {
	OverlayColor *ptr = (OverlayColor*)_screen.getBasePtr(r.left, r.top);
	const int h = r.height();
	const int w = r.width();
	const int lastRow = r.height() - 1;
	const int ptrAdd = _screen.w - r.width();
	for (int l = 0; l < h; ++l) {
		OverlayColor col = calcGradient(start, end, l, lastRow, factor);
		for (int i = 0; i < w; ++i) {
			*ptr++ = col;
		}
		ptr += ptrAdd;
	}
}

void ThemeModern::drawRect(const Common::Rect &r, const Surface *corner, const Surface *top, const Surface *left, const Surface *fill, int alpha, bool skipLastRow) {
	drawRectMasked(r, corner, top, left, fill, alpha, _system->RGBToColor(255, 255, 255), _system->RGBToColor(255, 255, 255), 1, skipLastRow);
}

void ThemeModern::drawRectMasked(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
							const Graphics::Surface *left, const Graphics::Surface *fill, int alpha,
							OverlayColor start, OverlayColor end, uint factor, bool skipLastRow, bool skipTopRow) {
	int drawWidth = MIN(corner->w, MIN(top->w, MIN(left->w, fill->w)));
	int drawHeight = MIN(corner->h, MIN(top->h, MIN(left->h, fill->h)));
	int partsH = r.height() / drawHeight;
	int partsW = r.width() / drawWidth;
	int yPos = r.top;

	int specialHeight = 0;
	int specialWidth = 0;

	if (drawHeight*2 > r.height()) {
		drawHeight = r.height() / 2;
		partsH = 2;
	} else {
		specialHeight = r.height() % drawHeight;
		if (specialHeight != 0)
			++partsH;
	}

	if (drawWidth*2 > r.width()) {
		drawWidth = r.width() / 2;
		partsW = 2;
	} else {
		specialWidth = r.width() % drawWidth;
		if (specialWidth != 0)
			++partsW;
	}

	for (int y = 0; y < partsH; ++y) {
		int xPos = r.left;
		bool upDown = (y == partsH - 1);

		// calculate the correct drawing height
		int usedHeight = drawHeight;
		if (specialHeight && y == 1) {
			usedHeight = specialHeight;
		}

		OverlayColor startCol = calcGradient(start, end, yPos-r.top, r.height()-1, factor);
		OverlayColor endCol = calcGradient(start, end, yPos-r.top+usedHeight, r.height()-1, factor);

		for (int i = 0; i < partsW; ++i) {
			// calculate the correct drawing width
			int usedWidth = drawWidth;
			if (specialWidth && i == 1) {
				usedWidth = specialWidth;
			}

			// draw the right surface
			if (!i || i == partsW - 1) {
				if ((!y && !skipTopRow) || (y == partsH - 1 && !skipLastRow)) {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), corner, upDown, (i == partsW - 1), alpha, startCol, endCol);
				} else {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), left, upDown, (i == partsW - 1), alpha, startCol, endCol);
				}
			} else if (!y || (y == partsH - 1 && !skipLastRow)) {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), top, upDown, false, alpha, startCol, endCol);
			} else {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), fill, false, false, alpha, startCol, endCol);
			}
			xPos += usedWidth;
		}

		yPos += usedHeight;
	}
}

Common::Rect ThemeModern::shadowRect(const Common::Rect &r, uint32 shadowStyle) {
	switch (shadowStyle) {
	case kShadowButton:
		return Common::Rect(r.left - 1, r.top - 1, r.right + 1, r.bottom + 1);

	case kShadowEmboss:
		return Common::Rect(r.left - 1, r.top - 1, r.right + 1, r.bottom + 1);

	case kShadowPopUp:
		return Common::Rect(r.left - 1, r.top - 1, r.right + 3, r.bottom + 3);

	case kShadowFull:
		return Common::Rect(r.left - 2, r.top - 2, r.right + 4, r.bottom + 4);

	default:
		return Common::Rect(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2 + 1, r.bottom + _shadowBottomHeight/2 + 1);
	}

	return Common::Rect();
}

void ThemeModern::drawShadow(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
						const Graphics::Surface *left, const Graphics::Surface *fill, uint32 shadowStyle, bool skipLastRow, bool skipTopRow) {
	switch (shadowStyle) {
	case kShadowFull: {
		Common::Rect r2(r.left-1, r.top-1, r.right + 4, r.bottom + 4);
		Common::Rect r3(r.left, r.top+1, r.right + 3, r.bottom + 3);
		Common::Rect r4(r.left, r.top+1, r.right + 2, r.bottom + 2);
		Common::Rect r5(r.left, r.top, r.right + 1, r.bottom + 1);

		drawShadowRect(r2, r, corner, top, left, fill, kShadowTr0, skipLastRow, skipTopRow);
		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr1, skipLastRow, skipTopRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr2, skipLastRow, skipTopRow);
		drawShadowRect(r5, r, corner, top, left, fill, kShadowTr3, skipLastRow, skipTopRow);
		//drawShadowRect(r5, r, corner, top, left, fill, kShadowTr35, skipLastRow);
		} break;

	case kShadowSmall: {
		Common::Rect r3(r.left - _shadowLeftWidth/2, r.top - _shadowTopHeight/2, r.right + _shadowRightWidth/2, r.bottom + _shadowBottomHeight/2);
		Common::Rect r4(r.left - _shadowLeftWidth/2 + 1, r.top - _shadowTopHeight/2 + 1, r.right + _shadowRightWidth/2-1, r.bottom + _shadowBottomHeight/2-1);

		drawShadowRect(r3, r, corner, top, left, fill, kShadowTr1, skipLastRow, skipTopRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr2, skipLastRow, skipTopRow);
		} break;

	case kShadowButton: {
		Common::Rect r2(r.left-1, r.top - 1, r.right, r.bottom);
		Common::Rect r4(r.left, r.top, r.right + 1, r.bottom + 1);

		drawShadowRect(r2, r, corner, top, left, fill, -kShadowTr35-256, skipLastRow, skipTopRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr4, skipLastRow, skipTopRow);
		} break;

	case kShadowEmboss: {
		Common::Rect r2(r.left - 1, r.top - 1, r.right, r.bottom);
		Common::Rect r4(r.left + 1, r.top + 1, r.right + 1, r.bottom + 1);

		drawShadowRect(r2, r, corner, top, left, fill, kShadowTr5, skipLastRow, skipTopRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr1, skipLastRow, skipTopRow);
		} break;

	case kShadowPopUp: {
		Common::Rect r2(r.left, r.top, r.right + 3, r.bottom + 3);
		Common::Rect r25(r.left-1, r.top-1, r.right + 2, r.bottom + 2);
		Common::Rect r3(r.left - 1, r.top-1, r.right, r.bottom);
		Common::Rect r4(r.left, r.top, r.right + 1, r.bottom + 1);

		drawShadowRect(r2, r, corner, top, left, fill, kShadowTr1, skipLastRow, skipTopRow);
		drawShadowRect(r25, r, corner, top, left, fill, kShadowTr2, skipLastRow, skipTopRow);
		drawShadowRect(r4, r, corner, top, left, fill, kShadowTr3, skipLastRow, skipTopRow);
		drawShadowRect(r3, r, corner, top, left, fill, -kShadowTr35-256, skipLastRow, skipTopRow);
		} break;

	default:
		break;
	}
}

void ThemeModern::drawShadowRect(const Common::Rect &r, const Common::Rect &area, const Graphics::Surface *corner,
							const Graphics::Surface *top, const Graphics::Surface *left, const Graphics::Surface *fill,
							int alpha, bool skipLastRow, bool skipTopRow) {
	int drawWidth = MIN(corner->w, MIN(top->w, MIN(left->w, fill->w)));
	int drawHeight = MIN(corner->h, MIN(top->h, MIN(left->h, fill->h)));
	int partsH = r.height() / drawHeight;
	int partsW = r.width() / drawWidth;
	int yPos = r.top;

	int yDrawTilesTop = 1 + (ABS(area.top - r.top) % drawHeight);
	int xDrawTilesLeft = 1 + (ABS(area.left - r.left) % drawWidth);
	int yDrawTilesBottom = 1 + (ABS(area.bottom - r.bottom) % drawHeight);
	int xDrawTilesRight = 1 + (ABS(area.right - r.right) % drawWidth);

	int specialHeight = 0;
	int specialWidth = 0;

	if (drawHeight*2 > r.height()) {
		drawHeight = r.height() / 2;
		partsH = 2;
	} else {
		specialHeight = r.height() % drawHeight;
		if (specialHeight != 0)
			++partsH;
	}

	if (drawWidth*2 > r.width()) {
		drawWidth = r.width() / 2;
		partsW = 2;
	} else {
		specialWidth = r.width() % drawWidth;
		if (specialWidth != 0)
			++partsW;
	}

	OverlayColor startCol = g_system->RGBToColor(0, 0, 0);
	OverlayColor endCol = g_system->RGBToColor(0, 0, 0);

	for (int y = 0; y < partsH; ++y) {
		// calculate the correct drawing height
		int usedHeight = drawHeight;
		if (specialHeight && y == 1) {
			usedHeight = specialHeight;
		}

		int xPos = r.left;
		bool upDown = (y == partsH - 1);

		for (int i = 0; i < partsW; ++i) {
			// calculate the correct drawing width
			int usedWidth = drawWidth;
			if (specialWidth && i == 1) {
				usedWidth = specialWidth;
			}

			if (i >= xDrawTilesLeft && i <= partsW - xDrawTilesRight && y >= yDrawTilesTop && y <= partsH - yDrawTilesBottom) {
				xPos += usedWidth;
				continue;
			}

			// draw the right surface
			if (!i || i == partsW - 1) {
				if ((!y && !skipTopRow) || (y == partsH - 1 && !skipLastRow)) {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), corner, upDown, (i == partsW - 1), alpha, startCol, endCol);
				} else {
					drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), left, upDown, (i == partsW - 1), alpha, startCol, endCol);
				}
			} else if (!y || (y == partsH - 1 && !skipLastRow)) {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), top, upDown, false, alpha, startCol, endCol);
			} else {
				drawSurfaceMasked(Common::Rect(xPos, yPos, xPos+usedWidth, yPos+usedHeight), fill, upDown, false, alpha, startCol, endCol);
			}
			xPos += usedWidth;
		}
		yPos += usedHeight;
	}
}

void ThemeModern::drawSurface(const Common::Rect &r, const Surface *surf, bool upDown, bool leftRight, int alpha) {
	drawSurfaceMasked(r, surf, upDown, leftRight, alpha, _system->RGBToColor(255, 255, 255), _system->RGBToColor(255, 255, 255));
}

void ThemeModern::drawSurfaceMasked(const Common::Rect &r, const Graphics::Surface *surf, bool upDown, bool leftRight,
								int alpha, OverlayColor start, OverlayColor end, uint factor) {
	OverlayColor *dst = (OverlayColor*)_screen.getBasePtr(r.left, r.top);
	const OverlayColor *src = 0;

	const OverlayColor transparency = _colors[kColorTransparency];
	int drawWidth = (r.width() < surf->w) ? r.width() : surf->w;
	if (r.left + drawWidth > _screen.w)
		drawWidth = _screen.w - r.left;

	int srcAdd = 0;
	if (upDown) {
		src = (const OverlayColor*)surf->pixels + (surf->h - 1) * surf->w;
		srcAdd = -surf->w;
	} else {
		src = (const OverlayColor*)surf->pixels;
		srcAdd = surf->w;
	}

	int h = r.height();
	if (r.top + h > _screen.h)
		h = _screen.h - r.top;

	while (dst < _screen.pixels) {
		dst += _screen.w;
		src += srcAdd;
		--h;
	}

	if (h <= 0)
		return;

#define NO_EFFECT(x) ((x) & rowColor)
#define ALPHA_EFFECT(x) (getColorAlpha((x) & rowColor, *dst, alpha))
#define DARKEN_EFFECT(x) (calcDimColor((x) & rowColor))

#define LEFT_RIGHT_OFFSET(x) (drawWidth-x-1)
#define NORMAL_OFFSET(x) (x)

#define blitSurface(a, b) \
		const int dstAdd = _screen.w - drawWidth; \
		const int lastRow = r.height() - 1; \
		for (int i = 0; i < h; ++i) { \
			OverlayColor rowColor = calcGradient(start, end, i, lastRow, factor); \
			for (int x = 0; x < drawWidth; ++x, ++dst) { \
				OverlayColor col = src[a(x)]; \
				if (col != transparency) { \
					col = b(col); \
					*dst = col; \
				} \
			} \
			dst += dstAdd; \
			src += srcAdd; \
		}
	if (alpha >= 256) {
		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, NO_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, NO_EFFECT);
		}
	} else if (alpha < 0 && alpha >= -256) {
		int backUp = _dimPercentValue;
		_dimPercentValue = 256 * (100 - (-alpha)) / 100;

		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, DARKEN_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, DARKEN_EFFECT);
		}

		_dimPercentValue = backUp;
	} else {
		if (leftRight) {
			blitSurface(LEFT_RIGHT_OFFSET, ALPHA_EFFECT);
		} else {
			blitSurface(NORMAL_OFFSET, ALPHA_EFFECT);
		}
	}
#undef blitSurface

#undef NORMAL_OFFSET
#undef LEFT_RIGHT_OFFSET

#undef DARKEN_EFFECT
#undef ALPHA_EFFECT
#undef NO_EFFECT
}

OverlayColor ThemeModern::getColor(State state) {
	switch (state) {
	case kStateDisabled:
		return _colors[kColorStateDisabled];

	case kStateHighlight:
		return _colors[kColorStateHighlight];

	default:
		break;
	};
	return _colors[kColorStateEnabled];
}

const Graphics::Surface *ThemeModern::getImageSurface(const kThemeImages n) const {
	if (n == kImageLogo)
		return _images[kThemeLogo];
	else if (n == kImageLogoSmall)
		return _images[kThemeLogoSmall];
	else
		return 0;
}

void ThemeModern::resetupGuiRenderer() {
	if (_lastUsedBitMask == gBitFormat || !_initOk) {
		// ok same format no need to reload
		return;
	}

	_lastUsedBitMask = gBitFormat;

	int i;
	for (i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.unregisterSurface(_imageHandles[i]);
	}

	for (i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.registerSurface(_imageHandles[i], 0);
		_images[i] = ImageMan.getSurface(_imageHandles[i]);
	}

	setupColors();
}

void ThemeModern::setupColors() {
	// load the colors from the config file
	getColorFromConfig("main_dialog_start", _colors[kMainDialogStart]);
	getColorFromConfig("main_dialog_end", _colors[kMainDialogEnd]);

	getColorFromConfig("dialog_start", _colors[kDialogStart]);
	getColorFromConfig("dialog_end", _colors[kDialogEnd]);

	getColorFromConfig("color_state_disabled", _colors[kColorStateDisabled]);
	getColorFromConfig("color_state_highlight", _colors[kColorStateHighlight]);
	getColorFromConfig("color_state_enabled", _colors[kColorStateEnabled]);
	getColorFromConfig("color_transparency", _colors[kColorTransparency]);

	getColorFromConfig("text_inverted_background", _colors[kTextInvertedBackground]);
	getColorFromConfig("text_inverted_color", _colors[kTextInvertedColor]);

	getColorFromConfig("widget_bkgd_start", _colors[kWidgetBackgroundStart]);
	getColorFromConfig("widget_bkgd_end", _colors[kWidgetBackgroundEnd]);
	getColorFromConfig("widget_bkgd_small_start", _colors[kWidgetBackgroundSmallStart]);
	getColorFromConfig("widget_bkgd_small_end", _colors[kWidgetBackgroundSmallEnd]);

	getColorFromConfig("button_bkgd_start", _colors[kButtonBackgroundStart]);
	getColorFromConfig("button_bkgd_end", _colors[kButtonBackgroundEnd]);
	getColorFromConfig("button_bkgd_highlight_start", _colors[kButtonBackgroundHighlightStart]);
	getColorFromConfig("button_bkgd_highlight_end", _colors[kButtonBackgroundHighlightEnd]);
	getColorFromConfig("button_text_enabled", _colors[kButtonTextEnabled]);
	getColorFromConfig("button_text_disabled", _colors[kButtonTextDisabled]);
	getColorFromConfig("button_text_highlight", _colors[kButtonTextHighlight]);

	getColorFromConfig("slider_background_start", _colors[kSliderBackgroundStart]);
	getColorFromConfig("slider_background_end", _colors[kSliderBackgroundEnd]);
	getColorFromConfig("slider_start", _colors[kSliderStart]);
	getColorFromConfig("slider_end", _colors[kSliderEnd]);
	getColorFromConfig("slider_highlight_start", _colors[kSliderHighStart]);
	getColorFromConfig("slider_highlight_end", _colors[kSliderHighEnd]);

	getColorFromConfig("tab_background_start", _colors[kTabBackgroundStart]);
	getColorFromConfig("tab_background_end", _colors[kTabBackgroundEnd]);

	getColorFromConfig("tab_active_start", _colors[kTabActiveStart]);
	getColorFromConfig("tab_active_end", _colors[kTabActiveEnd]);
	getColorFromConfig("tab_inactive_start", _colors[kTabInactiveStart]);
	getColorFromConfig("tab_inactive_end", _colors[kTabInactiveEnd]);

	getColorFromConfig("scrollbar_background_start", _colors[kScrollbarBackgroundStart]);
	getColorFromConfig("scrollbar_background_end", _colors[kScrollbarBackgroundEnd]);
	getColorFromConfig("scrollbar_button_start", _colors[kScrollbarButtonStart]);
	getColorFromConfig("scrollbar_button_end", _colors[kScrollbarButtonEnd]);
	getColorFromConfig("scrollbar_slider_start", _colors[kScrollbarSliderStart]);
	getColorFromConfig("scrollbar_slider_end", _colors[kScrollbarSliderEnd]);
	getColorFromConfig("scrollbar_button_highlight_start", _colors[kScrollbarButtonHighlightStart]);
	getColorFromConfig("scrollbar_button_highlight_end", _colors[kScrollbarButtonHighlightEnd]);
	getColorFromConfig("scrollbar_slider_highlight_start", _colors[kScrollbarSliderHighlightStart]);
	getColorFromConfig("scrollbar_slider_highlight_end", _colors[kScrollbarSliderHighlightEnd]);

	getColorFromConfig("caret_color", _colors[kCaretColor]);

	getColorFromConfig("popupwidget_start", _colors[kPopUpWidgetStart]);
	getColorFromConfig("popupwidget_end", _colors[kPopUpWidgetEnd]);
	getColorFromConfig("popupwidget_highlight_start", _colors[kPopUpWidgetHighlightStart]);
	getColorFromConfig("popupwidget_highlight_end", _colors[kPopUpWidgetHighlightEnd]);

	getColorFromConfig("edittext_background_start", _colors[kEditTextBackgroundStart]);
	getColorFromConfig("edittext_background_end", _colors[kEditTextBackgroundEnd]);
}

#define FONT_NAME_NORMAL "newgui_normal"
#define FONT_NAME_BOLD "newgui_bold"
#define FONT_NAME_ITALIC "newgui_italic"
#define FONT_NAME_FIXED_NORMAL "newgui_fixed_normal"
#define FONT_NAME_FIXED_BOLD "newgui_fixed_bold"
#define FONT_NAME_FIXED_ITALIC "newgui_fixed_italic"

void ThemeModern::setupFonts() {
	if (_screen.w >= 400 && _screen.h >= 270) {
		setupFont("fontfile_bold", FONT_NAME_BOLD, kFontStyleBold);
		setupFont("fontfile_normal", FONT_NAME_NORMAL, kFontStyleNormal);
		setupFont("fontfile_italic", FONT_NAME_ITALIC, kFontStyleItalic);

		setupFont("fontfile_fixed_bold", FONT_NAME_FIXED_BOLD, kFontStyleFixedBold);
		setupFont("fontfile_fixed_normal", FONT_NAME_FIXED_NORMAL, kFontStyleFixedNormal);
		setupFont("fontfile_fixed_italic", FONT_NAME_FIXED_ITALIC, kFontStyleFixedItalic);
	} else {
		_fonts[kFontStyleBold] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		_fonts[kFontStyleNormal] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		_fonts[kFontStyleItalic] = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	}
}

void ThemeModern::deleteFonts() {
	const Graphics::Font *normal = FontMan.getFontByName(FONT_NAME_NORMAL);
	const Graphics::Font *bold = FontMan.getFontByName(FONT_NAME_BOLD);
	const Graphics::Font *italic = FontMan.getFontByName(FONT_NAME_ITALIC);

	delete normal;
	delete bold;
	delete italic;

	FontMan.removeFontName(FONT_NAME_NORMAL);
	FontMan.removeFontName(FONT_NAME_BOLD);
	FontMan.removeFontName(FONT_NAME_ITALIC);
}

void ThemeModern::setupFont(const Common::String &key, const Common::String &name, FontStyle style) {
	if (_evaluator->getVar(key) == EVAL_STRING_VAR) {
		_fonts[style] = FontMan.getFontByName(name);

		if (!_fonts[style]) {
			Common::String temp(_evaluator->getStringVar(key));

			_fonts[style] = loadFont(temp.c_str());
			if (!_fonts[style])
				error("Couldn't load %s font '%s'", key.c_str(), temp.c_str());

			FontMan.assignFontToName(name, _fonts[style]);
		}
	} else {
		_fonts[style] = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
	}
}

#pragma mark -

void ThemeModern::processExtraValues() {
	// TODO: think of unloading all old graphic files first, instead of just
	// loading new ones if needed when this is processed?

	// load the pixmap filenames from the config file
#define loadRectData(type, str) \
		_imageHandles[k##type##Corner] = _evaluator->getStringVar("pix_"str"_corner"); \
		_imageHandles[k##type##Top] = _evaluator->getStringVar("pix_"str"_top"); \
		_imageHandles[k##type##Left] = _evaluator->getStringVar("pix_"str"_left"); \
		_imageHandles[k##type] = _evaluator->getStringVar("pix_"str"_bkgd")

	loadRectData(DialogBkgd, "dialog");
	loadRectData(WidgetBkgd, "widget");
	loadRectData(WidgetSmallBkgd, "widget_small");
	loadRectData(TabBkgd, "tab");
	loadRectData(SliderBkgd, "slider_bkgd");
	loadRectData(Slider, "slider");
	loadRectData(ScrollbarBkgd, "scrollbar_bkgd");
	loadRectData(Scrollbar, "scrollbar");
	loadRectData(ButtonBkgd, "button");
	loadRectData(PopUpWidgetBkgd, "popupwidget");
	loadRectData(EditTextBkgd, "edittext_bkgd");

#undef loadRectData

	_imageHandles[kCheckboxEmpty] = _evaluator->getStringVar("pix_checkbox_empty");
	_imageHandles[kCheckboxChecked] = _evaluator->getStringVar("pix_checkbox_checked");

	_imageHandles[kWidgetArrow] = _evaluator->getStringVar("pix_widget_arrow");

	_imageHandles[kThemeLogo] = _evaluator->getStringVar("pix_theme_logo");
	_imageHandles[kThemeLogoSmall] = _evaluator->getStringVar("pix_theme_logo_small");

	_imageHandles[kGUICursor] = _evaluator->getStringVar("pix_cursor_image");

	for (int i = 0; i < kImageHandlesMax; ++i) {
		ImageMan.registerSurface(_imageHandles[i], 0);
		_images[i] = ImageMan.getSurface(_imageHandles[i]);
	}

	// load the gradient factors from the config file
	_gradientFactors[kMainDialogFactor] = _evaluator->getVar("gradient_dialog_main", 1);
	_gradientFactors[kDialogFactor] = _evaluator->getVar("gradient_dialog", 1);
	_gradientFactors[kDialogSpecialFactor] = _evaluator->getVar("gradient_dialog_special", 1);

	_gradientFactors[kWidgetSmallFactor] = _evaluator->getVar("gradient_widget_small", 1);
	_gradientFactors[kWidgetFactor] = _evaluator->getVar("gradient_widget", 1);

	_gradientFactors[kButtonFactor] = _evaluator->getVar("gradient_button", 1);

	_gradientFactors[kSliderFactor] = _evaluator->getVar("gradient_slider", 1);
	_gradientFactors[kSliderBackground] = _evaluator->getVar("gradient_silder_bkgd", 1);

	_gradientFactors[kTabFactor] = _evaluator->getVar("gradient_tab", 1);

	_gradientFactors[kScrollbarFactor] = _evaluator->getVar("gradient_scrollbar", 1);
	_gradientFactors[kScrollbarBkgdFactor] = _evaluator->getVar("gradient_scrollbar_background", 1);

	_gradientFactors[kPopUpWidgetFactor] = _evaluator->getVar("gradient_popupwidget", 1);

	_gradientFactors[kEditTextFactor] = _evaluator->getVar("gradient_edittext", 1);

	// load values with default values from the config file
	_shadowLeftWidth = _evaluator->getVar("shadow_left_width", 2);
	_shadowRightWidth = _evaluator->getVar("shadow_right_width", 4);
	_shadowTopHeight = _evaluator->getVar("shadow_top_height", 2);
	_shadowBottomHeight = _evaluator->getVar("shadow_bottom_height", 4);

	_cursorHotspotX = _evaluator->getVar("cursor_hotspot_x", 0);
	_cursorHotspotY = _evaluator->getVar("cursor_hotspot_y", 0);

	_cursorTargetScale = _evaluator->getVar("cursor_targetScale", 1);

	// inactive dialog shading stuff

	ShadingStyle shading = (ShadingStyle)_evaluator->getVar("inactive_dialog_shading", kShadingNone);

	switch (shading) {
	case kShadingNone:
		_dialogShadingCallback = 0;
		break;

	case kShadingLuminance:
		_dialogShadingCallback = &ThemeModern::calcLuminance;
		break;

	case kShadingDim:
		_dimPercentValue = _evaluator->getVar("shading_dim_percent", -1);

		if (_dimPercentValue < 0) {
			_dimPercentValue = 0;
		} else if (_dimPercentValue > 100) {
			_dimPercentValue = 100;
		}

		if (_dimPercentValue != 0) {
			_dimPercentValue = 256 * (100 - _dimPercentValue) / 100;
			_dialogShadingCallback = &ThemeModern::calcDimColor;
		}
		break;

	default:
			warning("no valid 'inactive_dialog_shading' specified");
	}

	setupFonts();

	// load the colors from the config file
	setupColors();

	// creates cursor image
	if (_system->hasFeature(OSystem::kFeatureCursorHasPalette)) {
		createCursor();
	}
}

#pragma mark -

OverlayColor ThemeModern::calcLuminance(OverlayColor col) {
	uint8 r, g, b;
	_system->colorToRGB(col, r, g, b);

	// A better (but slower) formula to calculate the luminance would be:
	//uint lum = (byte)((0.299 * r + 0.587 * g + 0.114 * b) + 0.5);
	// Note that the approximation below will only produce values between
	// (and including) 0 and 221.
	uint lum = (r >> 2) + (g >> 1) + (b >> 3);

	return _system->RGBToColor(lum, lum, lum);
}

OverlayColor ThemeModern::calcDimColor(OverlayColor col) {
	uint8 r, g, b;
	_system->colorToRGB(col, r, g, b);

	r = r * _dimPercentValue >> 8;
	g = g * _dimPercentValue >> 8;
	b = b * _dimPercentValue >> 8;

	return _system->RGBToColor(r, g, b);
}

#pragma mark -

void ThemeModern::setUpCursor() {
	CursorMan.pushCursorPalette(_cursorPal, 0, MAX_CURS_COLORS);
	CursorMan.pushCursor(_cursor, _cursorWidth, _cursorHeight, _cursorHotspotX, _cursorHotspotY, 255, _cursorTargetScale);
	CursorMan.showMouse(true);
}

void ThemeModern::createCursor() {
	const Surface *cursor = _images[kGUICursor];

	_cursorWidth = cursor->w;
	_cursorHeight = cursor->h;

	uint colorsFound = 0;
	const OverlayColor *src = (const OverlayColor*)cursor->pixels;

	byte *table = new byte[65536];
	assert(table);
	memset(table, 0, sizeof(byte)*65536);

	byte r, g, b;

	_system->colorToRGB(_colors[kColorTransparency], r, g, b);
	uint16 transparency = RGBToColor<ColorMasks<565> >(r, g, b);

	delete[] _cursor;

	_cursor = new byte[_cursorWidth * _cursorHeight];
	assert(_cursor);
	memset(_cursor, 255, sizeof(byte)*_cursorWidth*_cursorHeight);

	for (uint y = 0; y < _cursorHeight; ++y) {
		for (uint x = 0; x < _cursorWidth; ++x) {
			_system->colorToRGB(src[x], r, g, b);
			uint16 col = RGBToColor<ColorMasks<565> >(r, g, b);
			if (!table[col] && col != transparency) {
				table[col] = colorsFound++;

				uint index = table[col];
				_cursorPal[index * 4 + 0] = r;
				_cursorPal[index * 4 + 1] = g;
				_cursorPal[index * 4 + 2] = b;
				_cursorPal[index * 4 + 3] = 0xFF;

				if (colorsFound > MAX_CURS_COLORS)
					error("Cursor contains too much colors (%d, but only %d are allowed)", colorsFound, MAX_CURS_COLORS);
			}

			if (col != transparency) {
				uint index = table[col];
				_cursor[y * _cursorWidth + x] = index;
			}
		}
		src += _cursorWidth;
	}

	_useCursor = true;
	delete[] table;
}

#pragma mark -

template<class T>
inline OverlayColor getColorAlphaImpl(OverlayColor col1, OverlayColor col2, int alpha) {
	OverlayColor output = 0;
	output |= ((alpha * ((col1 & T::kRedMask) - (col2 & T::kRedMask)) >> 8) + (col2 & T::kRedMask)) & T::kRedMask;
	output |= ((alpha * ((col1 & T::kGreenMask) - (col2 & T::kGreenMask)) >> 8) + (col2 & T::kGreenMask)) & T::kGreenMask;
	output |= ((alpha * ((col1 & T::kBlueMask) - (col2 & T::kBlueMask)) >> 8) + (col2 & T::kBlueMask)) & T::kBlueMask;
	output |= ~(T::kRedMask | T::kGreenMask | T::kBlueMask);
	return output;
}

// broken implementation!
template<class T>
inline OverlayColor getColorAlphaImp2(OverlayColor col1, OverlayColor col2, int alpha) {
	OverlayColor output = 0;
	output |= ((alpha * ((~col1 & T::kRedMask) - (col2 & T::kRedMask)) >> 8) + (col2 & T::kRedMask)) & T::kRedMask;
	output |= ((alpha * ((~col1 & T::kGreenMask) - (col2 & T::kGreenMask)) >> 8) + (col2 & T::kGreenMask)) & T::kGreenMask;
	output |= ((alpha * ((~col1 & T::kBlueMask) - (col2 & T::kBlueMask)) >> 8) + (col2 & T::kBlueMask)) & T::kBlueMask;
	output |= ~(T::kRedMask | T::kGreenMask | T::kBlueMask);
	return output;
}

OverlayColor getColorAlpha(OverlayColor col1, OverlayColor col2, int alpha) {
	if (alpha >= 0) {
		if (gBitFormat == 565) {
			return getColorAlphaImpl<ColorMasks<565> >(col1, col2, alpha);
		} else {
			return getColorAlphaImpl<ColorMasks<555> >(col1, col2, alpha);
		}
	} else {
		if (gBitFormat == 565) {
			return getColorAlphaImp2<ColorMasks<565> >(col1, col2, -alpha - 256);
		} else {
			return getColorAlphaImp2<ColorMasks<555> >(col1, col2, -alpha - 256);
		}
	}
}

template<class T>
inline OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos) {
	OverlayColor output = 0;
	output |= (start + ((((end & T::kRedMask) - (start & T::kRedMask))) * pos >> 12)) & T::kRedMask;
	output |= (start + ((((end & T::kGreenMask) - (start & T::kGreenMask))) * pos >> 12)) & T::kGreenMask;
	output |= (start + ((((end & T::kBlueMask) - (start & T::kBlueMask))) * pos >> 12)) & T::kBlueMask;
	output |= ~(T::kRedMask | T::kGreenMask | T::kBlueMask);
	return output;
}

OverlayColor calcGradient(OverlayColor start, OverlayColor end, int pos, int max, uint factor = 1) {
	pos *= factor;
	if (pos >= max)
		return end;

	pos = (0x1000 * pos) / max;

	if (gBitFormat == 565) {
		return calcGradient<ColorMasks<565> >(start, end, pos);
	} else {
		return calcGradient<ColorMasks<555> >(start, end, pos);
	}
}

void getStateColor(OverlayColor &s, OverlayColor &e,
					OverlayColor enabledS, OverlayColor enabledE,
					OverlayColor highlightS, OverlayColor highlightE,
					Theme::WidgetStateInfo state) {
	if (state == Theme::kStateHighlight) {
		s = highlightS;
		e = highlightE;
	} else {
		s = enabledS;
		e = enabledE;
	}
}

} // end of namespace GUI

#endif
