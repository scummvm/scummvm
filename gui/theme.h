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

#ifndef GUI_THEME_H
#define GUI_THEME_H

#include "common/stdafx.h"
#include "common/system.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/file.h"
#include "common/config-file.h"

#include "graphics/surface.h"
#include "graphics/fontman.h"

#include "gui/widget.h"

namespace GUI {

class Eval;

// Hints to the theme engine that the widget is used in a non-standard way.

enum {
	// Indicates that this is the first time the widget is drawn.
	THEME_HINT_FIRST_DRAW = 1 << 0,

	// Indicates that the widget will be redrawn often, e.g. list widgets.
	// It may therefore be a good idea to save the background so that it
	// can be redrawn quickly.
	THEME_HINT_SAVE_BACKGROUND = 1 << 1,
	
	// Indicates that this is the launcher dialog (maybe delete this in the future)
	THEME_HINT_MAIN_DIALOG = 1 << 2,
	
	// Indicates special colorfade
	THEME_HINT_SPECIAL_COLOR = 1 << 3,
	
	// Indicates no colorfade
	THEME_HINT_PLAIN_COLOR = 1 << 4,
	
	// Indictaes that a shadows should be drawn around the background
	THEME_HINT_USE_SHADOW = 1 << 5
};


class Theme {
	typedef Common::String String;
public:
	Theme();
		
	virtual ~Theme();

	enum TextAlign {
		kTextAlignLeft,
		kTextAlignCenter,
		kTextAlignRight
	};

	enum WidgetBackground {
		kWidgetBackgroundNo,
		kWidgetBackgroundPlain,
		kWidgetBackgroundBorder,
		kWidgetBackgroundBorderSmall,
		kWidgetBackgroundEditText
	};

	enum State {
		kStateDisabled,
		kStateEnabled,
		kStateHighlight
	};

	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	enum FontStyle {
		kFontStyleBold = 0,	// standard font
		kFontStyleNormal = 1,
		kFontStyleItalic = 2,
		kFontStyleFixedNormal = 3,
		kFontStyleFixedBold = 4,
		kFontStyleFixedItalic = 5,
		kFontStyleMax
	};

	enum ShadingStyle {
		kShadingNone,
		kShadingDim,
		kShadingLuminance
	};

	virtual bool init() = 0;
	virtual void deinit() = 0;

	virtual void refresh() = 0;

	virtual bool ownCursor() { return false; }

	virtual void enable() = 0;
	virtual void disable() = 0;

	virtual void openDialog(bool topDialog) = 0;
	virtual void closeDialog() = 0;

	virtual void clearAll() = 0;
	virtual void drawAll() = 0;
	
	virtual void setDrawArea(const Common::Rect &r) { _drawArea = r; }
	// resets the draw area to the screen size
	virtual void resetDrawArea() = 0;
	
	virtual const Common::ConfigFile &getConfigFile() { return _configFile; }

	virtual const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const = 0;
	virtual int getFontHeight(FontStyle font = kFontStyleBold) const = 0;
	virtual int getStringWidth(const Common::String &str, FontStyle font = kFontStyleBold) const = 0;
	virtual int getCharWidth(byte c, FontStyle font = kFontStyleBold) const = 0;

	virtual void drawDialogBackground(const Common::Rect &r, uint16 hints, State state = kStateEnabled) = 0;
	virtual void drawText(const Common::Rect &r, const Common::String &str, State state = kStateEnabled, TextAlign align = kTextAlignCenter, bool inverted = false, int deltax = 0, bool useEllipsis = true, FontStyle font = kFontStyleBold) = 0;
	// this should ONLY be used by the debugger until we get a nicer solution
	virtual void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, State state = kStateEnabled) = 0;

	virtual void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background = kWidgetBackgroundPlain, State state = kStateEnabled) = 0;
	virtual void drawButton(const Common::Rect &r, const Common::String &str, State state = kStateEnabled, uint16 hints = 0) = 0;
	virtual void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, State state = kStateEnabled, int alpha = 256, bool themeTrans = false) = 0;
	virtual void drawSlider(const Common::Rect &r, int width, State state = kStateEnabled) = 0;
	virtual void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, State state = kStateEnabled) = 0;
	virtual void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, State state = kStateEnabled) = 0;
	virtual void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, State state = kStateEnabled) = 0;
	virtual void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, State state = kStateEnabled, TextAlign align = kTextAlignLeft) = 0;
	virtual void drawCaret(const Common::Rect &r, bool erase, State state = kStateEnabled) = 0;
	virtual void drawLineSeparator(const Common::Rect &r, State state = kStateEnabled) = 0;

	virtual void restoreBackground(Common::Rect r, bool special = false) = 0;
	virtual bool addDirtyRect(Common::Rect r, bool save = false, bool special = false) = 0;

	virtual int getTabSpacing() const = 0;
	virtual int getTabPadding() const = 0;

	Graphics::TextAlignment convertAligment(TextAlign align) const {
		switch (align) {
		case kTextAlignLeft:
			return Graphics::kTextAlignLeft;
			break;

		case kTextAlignRight:
			return Graphics::kTextAlignRight;
			break;

		default:
			break;
		};
		return Graphics::kTextAlignCenter;
	};
	
	TextAlign convertAligment(Graphics::TextAlignment align) const {
		switch (align) {
		case Graphics::kTextAlignLeft:
			return kTextAlignLeft;
			break;

		case Graphics::kTextAlignRight:
			return kTextAlignRight;
			break;

		default:
			break;
		}
		return kTextAlignCenter;
	}

	void processResSection(Common::ConfigFile &config, const String &name, bool skipDefs = false, const String &prefix = "");
	void processSingleLine(const String &section, const String &prefix, const String &name, const String &str);
	void setSpecialAlias(const String &alias, const String &name);

	bool isThemeLoadingRequired();
	bool sectionIsSkipped(Common::ConfigFile &config, const char *name, int w, int h);
	void loadTheme(Common::ConfigFile &config, bool reset = true);

	Eval *_evaluator;

protected:
	Common::Rect _drawArea;
	Common::ConfigFile _configFile;
	Common::ConfigFile _defaultConfig;

private:
	static const char *_defaultConfigINI;
	int _loadedThemeX, _loadedThemeY;
};

// enable this to disable transparency support for the classic theme
//#define CT_NO_TRANSPARENCY

class ThemeClassic : public Theme {
public:
	ThemeClassic(OSystem *system);
	virtual ~ThemeClassic();

	bool init();
	void deinit();

	void refresh();

	void enable();
	void disable();

	void openDialog(bool topDialog);
	void closeDialog();

	void clearAll();
	void drawAll();
	
	void resetDrawArea();

	typedef Common::String String;

	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }

	void drawDialogBackground(const Common::Rect &r, uint16 hints, State state);
	void drawText(const Common::Rect &r, const String &str, State state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, State state);

	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, State state);
	void drawButton(const Common::Rect &r, const String &str, State state, uint16 hints);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, State state, int alpha, bool themeTrans);
	void drawSlider(const Common::Rect &r, int width, State state);
	void drawCheckbox(const Common::Rect &r, const String &str, bool checked, State state);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<String> &tabs, int active, uint16 hints, int titleVPad, State state);
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, State state);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, State state, TextAlign align);
	void drawCaret(const Common::Rect &r, bool erase, State state);
	void drawLineSeparator(const Common::Rect &r, State state);
	void restoreBackground(Common::Rect r, bool special = false);
	bool addDirtyRect(Common::Rect r, bool save = false, bool special = false);

	int getTabSpacing() const;
	int getTabPadding() const;

private:
	void box(int x, int y, int width, int height, OverlayColor colorA, OverlayColor colorB, bool skipLastRow = false);
	void box(int x, int y, int width, int height);

	OverlayColor getColor(State state);

	OSystem *_system;
	Graphics::Surface _screen;

#ifndef CT_NO_TRANSPARENCY
	struct DialogState {
		Graphics::Surface screen;
	} *_dialog;

	void blendScreenToDialog();
#endif

	bool _forceRedraw;
	bool _initOk;
	bool _enabled;

	const Graphics::Font *_font;
	OverlayColor _color, _shadowcolor;
	OverlayColor _bgcolor;
	OverlayColor _textcolor;
	OverlayColor _textcolorhi;
};

#ifndef DISABLE_FANCY_THEMES

class ThemeNew : public Theme {
	typedef Common::String String;
public:
	ThemeNew(OSystem *system, const String &stylefile);
	virtual ~ThemeNew();

	bool init();
	void deinit();

	void refresh();

	bool ownCursor() { return _useCursor; }

	void enable();
	void disable();
	
	void openDialog(bool topDialog);
	void closeDialog();

	void clearAll();
	void drawAll();
	
	void setDrawArea(const Common::Rect &r);
	void resetDrawArea();

	const Graphics::Font *getFont(FontStyle font = kFontStyleBold) const { return _fonts[font]; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getFontHeight(); return 0; }
	int getStringWidth(const String &str, FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font = kFontStyleBold) const { if (_fonts[font]) return _fonts[font]->getCharWidth(c); return 0; }

	void drawDialogBackground(const Common::Rect &r, uint16 hints, State state);
	void drawText(const Common::Rect &r, const String &str, State state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, State state);

	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, State state);
	void drawButton(const Common::Rect &r, const String &str, State state, uint16 hints);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, State state, int alpha, bool themeTrans);
	void drawSlider(const Common::Rect &r, int width, State state);
	void drawCheckbox(const Common::Rect &r, const String &str, bool checked, State state);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<String> &tabs, int active, uint16 hints, int titleVPad, State state);
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, State state);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, State state, TextAlign align);
	void drawCaret(const Common::Rect &r, bool erase, State state);
	void drawLineSeparator(const Common::Rect &r, State state);
	const Graphics::Surface *getImageSurface(int n) { return _images[n]; }

	void restoreBackground(Common::Rect r, bool special = false);
	bool addDirtyRect(Common::Rect r, bool backup = false, bool special = false);

	int getTabSpacing() const;
	int getTabPadding() const;

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

	Common::String _stylefile;

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
	const Graphics::Font *loadFont(const char *filename);
	Common::String genCacheFilename(const char *filename);
	const Graphics::Font *_fonts[kFontStyleMax];

private:
	void setupFont(const String &key, const String &name, FontStyle style);
	void processExtraValues();
	void getColorFromConfig(const String &value, OverlayColor &color);

public:
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

		kPopUpWidgetBkgdCorner = 40,
		kPopUpWidgetBkgdTop = 41,
		kPopUpWidgetBkgdLeft = 42,
		kPopUpWidgetBkgd = 43,

		kEditTextBkgdCorner = 44,
		kEditTextBkgdTop = 45,
		kEditTextBkgdLeft = 46,
		kEditTextBkgd = 47,

		kGUICursor = 48,
	
		kImageHandlesMax
	};

private:
	int _dimPercentValue;
	typedef OverlayColor (ThemeNew::*InactiveDialogCallback)(OverlayColor col);	
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
	const String *_imageHandles;
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
#endif // DISABLE_FANCY_THEMES
} // end of namespace GUI

#endif // GUI_THEME_H
