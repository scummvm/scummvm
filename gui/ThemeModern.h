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

#ifndef GUI_THEMEMODERN_H
#define GUI_THEMEMODERN_H

#ifndef DISABLE_FANCY_THEMES

#include "gui/theme.h"

namespace GUI {

class ThemeModern : public Theme {
public:
	ThemeModern(OSystem *system, const Common::String &stylefile, const Common::ConfigFile *cfg = 0);
	virtual ~ThemeModern();

	bool init();
	void deinit();

	void refresh();

	bool ownCursor() const { return _useCursor; }

	void enable();
	void disable();

	void openDialog(bool topDialog);
	void closeAllDialogs();

	void clearAll();
	void updateScreen();

	void setDrawArea(const Common::Rect &r);
	void resetDrawArea();

	const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const { return _fonts[font]; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getCharWidth(c); return 0; }

	void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state);
	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state);

	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state);
	void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans);
	void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state);
	void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state);
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align);
	void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state);
	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state);

	void restoreBackground(Common::Rect r, bool special = false);
	bool addDirtyRect(Common::Rect r, bool backup = false, bool special = false);

	int getTabSpacing() const;
	int getTabPadding() const;

	bool supportsImages() const { return true; }
	const Graphics::Surface *getImageSurface(const kThemeImages n) const;
private:
	void colorFade(const Common::Rect &r, OverlayColor start, OverlayColor end, uint factor = 1);
	void drawRect(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
				const Graphics::Surface *left, const Graphics::Surface *fill, int alpha, bool skipLastRow = false);
	void drawRectMasked(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
						const Graphics::Surface *left, const Graphics::Surface *fill, int alpha,
						OverlayColor start, OverlayColor end, uint factor = 1, bool skipLastRow = false, bool skipTopRow = false);
	void drawSurface(const Common::Rect &r, const Graphics::Surface *surf, bool upDown, bool leftRight, int alpha);
	void drawSurfaceMasked(const Common::Rect &r, const Graphics::Surface *surf, bool upDown, bool leftRight, int alpha,
							OverlayColor start, OverlayColor end, uint factor = 1);

	enum ShadowStyles {
		kShadowFull = 0,
		kShadowSmall = 1,
		kShadowButton = 2,
		kShadowEmboss = 3,
		kShadowPopUp = 4
	};

	Common::Rect shadowRect(const Common::Rect &r, uint32 shadowStyle);
	void drawShadow(const Common::Rect &r, const Graphics::Surface *corner, const Graphics::Surface *top,
					const Graphics::Surface *left, const Graphics::Surface *fill, uint32 shadowStyle, bool skipLastRow = false,
					bool skipTopRow = false);
	void drawShadowRect(const Common::Rect &r, const Common::Rect &area, const Graphics::Surface *corner,
						const Graphics::Surface *top, const Graphics::Surface *left, const Graphics::Surface *fill,
						int alpha, bool skipLastRow = false, bool skipTopRow = false);

	int _shadowLeftWidth, _shadowRightWidth;
	int _shadowTopHeight, _shadowBottomHeight;

	OSystem *_system;
	Graphics::Surface _screen;
	Common::Rect _shadowDrawArea;

	bool _initOk;
	bool _forceRedraw;
	bool _enabled;

	int _lastUsedBitMask;
	void resetupGuiRenderer();
	void setupColors();

	OverlayColor getColor(State state);

	struct DialogState {
		Graphics::Surface screen;
	} *_dialog;

	void setupFonts();
	void deleteFonts();

	void setupFont(const Common::String &key, const Common::String &name, FontStyle style);

	const Graphics::Font *_fonts[kFontStyleMax];

private:
	void processExtraValues();

	enum ImageHandles {
		kDialogBkgdCorner = 0,
		kDialogBkgdTop = 1,
		kDialogBkgdLeft = 2,
		kDialogBkgd = 3,

		kWidgetBkgdCorner = 4,
		kWidgetBkgdTop = 5,
		kWidgetBkgdLeft = 6,
		kWidgetBkgd = 7,

		kCheckboxEmpty = 8,
		kCheckboxChecked = 9,

		kWidgetArrow = 10,

		kTabBkgdCorner = 11,
		kTabBkgdTop = 12,
		kTabBkgdLeft = 13,
		kTabBkgd = 14,

		kSliderBkgdCorner = 15,
		kSliderBkgdTop = 16,
		kSliderBkgdLeft = 17,
		kSliderBkgd = 18,

		kSliderCorner = 19,
		kSliderTop = 20,
		kSliderLeft = 21,
		kSlider = 22,

		kScrollbarBkgdCorner = 23,
		kScrollbarBkgdTop = 24,
		kScrollbarBkgdLeft = 25,
		kScrollbarBkgd = 26,

		kScrollbarCorner = 27,
		kScrollbarTop = 28,
		kScrollbarLeft = 29,
		kScrollbar = 30,

		kButtonBkgdCorner = 31,
		kButtonBkgdTop = 32,
		kButtonBkgdLeft = 33,
		kButtonBkgd = 34,

		kWidgetSmallBkgdCorner = 35,
		kWidgetSmallBkgdTop = 36,
		kWidgetSmallBkgdLeft = 37,
		kWidgetSmallBkgd = 38,

		kThemeLogo = 39,
		kThemeLogoSmall = 40,

		kPopUpWidgetBkgdCorner = 41,
		kPopUpWidgetBkgdTop = 42,
		kPopUpWidgetBkgdLeft = 43,
		kPopUpWidgetBkgd = 44,

		kEditTextBkgdCorner = 45,
		kEditTextBkgdTop = 46,
		kEditTextBkgdLeft = 47,
		kEditTextBkgd = 48,

		kGUICursor = 49,

		kImageHandlesMax
	};

private:
	int _dimPercentValue;
	typedef OverlayColor (ThemeModern::*InactiveDialogCallback)(OverlayColor col);
	InactiveDialogCallback _dialogShadingCallback;

	OverlayColor calcLuminance(OverlayColor col);
	OverlayColor calcDimColor(OverlayColor col);

	bool _useCursor;
	void setUpCursor();
	void createCursor();
	int _cursorHotspotX, _cursorHotspotY;
	int _cursorTargetScale;
#define MAX_CURS_COLORS 255
	byte *_cursor;
	bool _needPaletteUpdates;
	uint _cursorWidth, _cursorHeight;
	byte _cursorPal[4*MAX_CURS_COLORS];

private:
	Common::String _imageHandles[kImageHandlesMax];
	const Graphics::Surface **_images;

	enum ColorHandles {
		kMainDialogStart = 0,
		kMainDialogEnd = 1,

		kDialogStart = 2,
		kDialogEnd = 3,

		kColorStateDisabled = 4,
		kColorStateHighlight = 5,
		kColorStateEnabled = 6,
		kColorTransparency = 7,

		kTextInvertedBackground = 8,
		kTextInvertedColor = 9,

		kWidgetBackgroundStart = 10,
		kWidgetBackgroundEnd = 11,
		kWidgetBackgroundSmallStart = 12,
		kWidgetBackgroundSmallEnd = 13,

		kButtonBackgroundStart = 14,
		kButtonBackgroundEnd = 15,
		kButtonTextEnabled = 16,
		kButtonTextDisabled = 17,
		kButtonTextHighlight = 18,

		kSliderBackgroundStart = 19,
		kSliderBackgroundEnd = 20,
		kSliderStart = 21,
		kSliderEnd = 22,

		kTabBackgroundStart = 23,
		kTabBackgroundEnd = 24,

		kScrollbarBackgroundStart = 25,
		kScrollbarBackgroundEnd = 26,
		kScrollbarButtonStart = 27,
		kScrollbarButtonEnd = 28,
		kScrollbarSliderStart = 29,
		kScrollbarSliderEnd = 30,

		kCaretColor = 31,

		kSliderHighStart = 32,
		kSliderHighEnd = 33,

		kButtonBackgroundHighlightStart = 34,
		kButtonBackgroundHighlightEnd = 35,

		kScrollbarButtonHighlightStart = 36,
		kScrollbarButtonHighlightEnd = 37,
		kScrollbarSliderHighlightStart = 38,
		kScrollbarSliderHighlightEnd = 39,

		kPopUpWidgetStart = 40,
		kPopUpWidgetEnd = 41,
		kPopUpWidgetHighlightStart = 42,
		kPopUpWidgetHighlightEnd = 43,

		kEditTextBackgroundStart = 44,
		kEditTextBackgroundEnd = 45,

		kTabActiveStart = 46,
		kTabActiveEnd = 47,
		kTabInactiveStart = 48,
		kTabInactiveEnd = 49,

		kColorHandlesMax
	};

	OverlayColor _colors[kColorHandlesMax];

	enum GradientFactors {
		kMainDialogFactor = 0,
		kDialogFactor = 1,
		kDialogSpecialFactor = 2,

		kWidgetSmallFactor = 3,
		kWidgetFactor = 4,

		kButtonFactor = 5,

		kSliderFactor = 6,
		kSliderBackground = 7,

		kTabFactor = 7,

		kScrollbarFactor = 8,
		kScrollbarBkgdFactor = 9,

		kPopUpWidgetFactor = 10,

		kEditTextFactor = 11,

		kMaxGradientFactors
	};

	uint _gradientFactors[kMaxGradientFactors];
};

} // end of namespace GUI

#endif

#endif

