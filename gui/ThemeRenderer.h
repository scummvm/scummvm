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
 *
 */

#ifndef INTERFACE_MANAGER_H
#define INTERFACE_MANAGER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

#include "graphics/surface.h"
#include "graphics/fontman.h"

#include "gui/dialog.h"
#include "gui/ThemeParser.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

struct WidgetDrawData;

struct WidgetDrawData {
	Common::Array<Graphics::DrawStep*> _steps;

	bool _cached;
	Graphics::Surface *_surfaceCache;
	uint32 _cachedW, _cachedH;

	~WidgetDrawData() {
		for (uint i = 0; i < _steps.size(); ++i)
			delete _steps[i];

		_steps.clear();

		if (_surfaceCache) {
			_surfaceCache->free();
			delete _surfaceCache;
		}
	}
};

class ThemeRenderer : public Theme {

	typedef Common::String String;
	typedef GUI::Dialog Dialog;

	friend class GUI::Dialog;
	friend class GUI::GuiObject;

	static const char *kDrawDataStrings[];
	static const int kMaxDialogDepth = 4;

public:
	enum Graphics_Mode {
		kGfxDisabled = 0,
		kGfxStandard16bit,
		kGfxAntialias16bit
	};

	enum DrawData {
		kDDMainDialogBackground,
		kDDSpecialColorBackground,
		kDDPlainColorBackground,
		kDDDefaulBackground,

		kDDButtonIdle,
		kDDButtonHover,

		kDDSurface,

		kDDSliderFull,
		kDDSliderEmpty,

		kDDCheckboxEnabled,
		kDDCheckboxDisabled,

		kDDTab,

		kDDScrollbarBase,
		kDDScrollbarButtonTop,
		kDDScrollbarButtonBottom,
		kDDScrollbarHandle,

		kDDPopUp,
		kDDCaret,
		kDDSeparator,
		kDrawDataMAX
	};

	ThemeRenderer();

	~ThemeRenderer() {
		freeRenderer();
		freeScreen();
		unloadTheme();
		delete _parser;

		while (!_dialogStack.empty())
			delete _dialogStack.pop();
	}

	// virtual methods from Theme
	bool init();
	void deinit() {}
	void refresh() {}
	void enable() {}
	void disable() {}
	void openDialog() {}
	void closeAllDialogs() {}
	void clearAll() {}
	void updateScreen() {}
	void resetDrawArea() {}
	void openDialog(bool top) {}

	virtual bool isDynamic() {
		return true;
	}

	/** Font management */
	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }

	/** Widget drawing */
	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled) {}
	void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, uint16 hints = 0);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false) {}
	void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state = kStateEnabled);
	void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state = kStateEnabled);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state = kStateEnabled) {}
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state = kStateEnabled);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignLeft) {}
	void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state = kStateEnabled) {}
	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled);

	void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state) {}
	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font) {}
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state) {}

	bool addDirtyRect(Common::Rect r, bool backup = false, bool special = false) {
		_dirtyScreen.push_back(r);
		return true;
	}

	// custom stuff - tanoku
	DrawData getDrawDataId(Common::String &name) {
		for (int i = 0; i < kDrawDataMAX; ++i)
			if (name.compareToIgnoreCase(kDrawDataStrings[i]) == 0)
				return (DrawData)i;

		return (DrawData)-1;
	}

	void addDrawStep(Common::String &drawDataId, Graphics::DrawStep *step);
	bool addDrawData(DrawData data_id, bool cached);

	ThemeParser *parser() {
		return _parser;
	}

	bool ready() {
		return _initOk && _themeOk;
	}

	bool loadTheme() {
		ConfMan.registerDefault("gui_theme", "default");
		Common::String style(ConfMan.get("gui_theme"));

		if (style.compareToIgnoreCase("default") == 0)
			style = "modern";

		return loadTheme(style);
	}

	bool loadTheme(Common::String themeName);
	
	void closeTopDialog() {
		assert(_dialogStack.empty() == false);
		delete _dialogStack.pop();
	}

	void setGraphicsMode(Graphics_Mode mode);

protected:
	template<typename PixelType> void screenInit();

	bool loadThemeXML(Common::String themeName);
	bool loadDefaultXML();

	void unloadTheme() {
		if (!_themeOk)
			return;

		for (int i = 0; i < kDrawDataMAX; ++i) {
			delete _widgets[i];
			_widgets[i] = 0;
		}
	}

	void screenChange() {}
	void renderDirtyScreen();

	void freeRenderer() {
		delete _vectorRenderer;
		_vectorRenderer = 0;
	}

	void freeScreen() {
		if (_screen != 0) {
			_screen->free();
			delete _screen;
			_screen = 0;
		}
	}

	Dialog *getTopDialog() const {
		if (_dialogStack.empty())
			return 0;
		return _dialogStack.top();
	}

	bool needThemeReload() {
		return (_themeOk == false || _needThemeLoad == true);
	}

	bool needRedraw() {
		return true;
	}

	void redrawDialogStack();

	bool isWidgetCached(DrawData type, const Common::Rect &r);
	void drawCached(DrawData type, const Common::Rect &r);

	inline void drawDD(DrawData type, const Common::Rect &r);

	// TODO
	void restoreBackground(Common::Rect r, bool special = false) {}

	int getTabSpacing() const {
		return 0;
	}

	int getTabPadding() const {
		return 3;
	}

	OSystem *_system;
	Graphics::VectorRenderer *_vectorRenderer;
	GUI::ThemeParser *_parser;

	Graphics::Surface *_screen;

	int _bytesPerPixel;
	Graphics_Mode _graphicsMode;

	Common::String _fontName;
	const Graphics::Font *_font;

	WidgetDrawData *_widgets[kDrawDataMAX];
	Common::FixedStack<Dialog *, kMaxDialogDepth> _dialogStack;
	Common::Array<Common::Rect> _dirtyScreen;

	bool _initOk;
	bool _themeOk;
	bool _caching;
	bool _needThemeLoad;
	bool _enabled;
};

} // end of namespace GUI.

#endif
