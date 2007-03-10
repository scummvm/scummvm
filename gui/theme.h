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

#define THEME_VERSION 21

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
	THEME_HINT_USE_SHADOW = 1 << 5,

	// Indicates that no background should be restored when drawing the widget
	// (note that this can be silently ignored if for example the theme does
	// alpha blending and would blend over a allready drawn widget)
	// TODO: currently only ThemeModern::drawButton supports this
	THEME_HINT_NO_BACKGROUND_RESTORE = 1 << 6
};


class Theme {
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
		kWidgetBackgroundEditText,
		kWidgetBackgroundSlider
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

	void processResSection(Common::ConfigFile &config, const Common::String &name, bool skipDefs = false, const Common::String &prefix = "");
	void processSingleLine(const Common::String &section, const Common::String &prefix, const Common::String &name, const Common::String &str);
	void setSpecialAlias(const Common::String &alias, const Common::String &name);

	bool isThemeLoadingRequired();
	bool sectionIsSkipped(Common::ConfigFile &config, const char *name, int w, int h);
	void loadTheme(Common::ConfigFile &config, bool reset = true);

	Eval *_evaluator;

	static bool themeConfigUseable(const Common::String &file, const Common::String &style="", Common::String *cStyle=0, Common::ConfigFile *cfg=0);

	const Common::String &getStylefileName() const { return _stylefile; }
	const Common::String &getThemeName() const { return _stylename; }

	virtual bool supportsImages() const { return false; }

	enum kThemeImages {
		kImageLogo = 0
	};

	virtual const Graphics::Surface *getImageSurface(const kThemeImages n) const { return 0; }
protected:
	bool loadConfigFile(const Common::String &file);
	void getColorFromConfig(const Common::String &name, OverlayColor &col);
	void getColorFromConfig(const Common::String &value, uint8 &r, uint8 &g, uint8 &b);

	const Graphics::Font *loadFont(const char *filename);
	Common::String genCacheFilename(const char *filename);

	Common::String _stylefile, _stylename;

	Common::Rect _drawArea;
	Common::ConfigFile _configFile;
	Common::ConfigFile _defaultConfig;

public:
	bool needThemeReload() { return ((_loadedThemeX != g_system->getOverlayWidth()) ||
									 (_loadedThemeY != g_system->getOverlayHeight())); }

private:
	static const char *_defaultConfigINI;
	int _loadedThemeX, _loadedThemeY;
};
} // end of namespace GUI

#endif // GUI_THEME_H
