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
struct DrawDataInfo;

struct WidgetDrawData {
	/** List of all the steps needed to draw this widget */
	Common::List<Graphics::DrawStep> _steps;
	
	/** Single step that defines the text shown inside the widget */
	Graphics::TextStep _textStep;
	bool _hasText;

	/** Extra space that the widget occupies when it's drawn.
	    E.g. when taking into account rounded corners, drop shadows, etc 
		Used when restoring the widget background */
	uint16 _backgroundOffset;

	/** Sets whether the widget is cached beforehand. */
	bool _cached;
	bool _buffer;

	/** Texture where the cached widget is stored. */
	Graphics::Surface *_surfaceCache;

	~WidgetDrawData() {
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

	/** Strings representing each value in the DrawData enum */

	/** Constant value to expand dirty rectangles, to make sure they are fully copied */
	static const int kDirtyRectangleThreshold = 2;
	
	/** Sets whether backcaching is enabled */
	static const bool kEnableBackCaching = true;

public:
	enum GraphicsMode {
		kGfxDisabled = 0,
		kGfxStandard16bit,
		kGfxAntialias16bit
	};

	enum DrawData {
		kDDMainDialogBackground,
		kDDSpecialColorBackground,
		kDDPlainColorBackground,
		kDDDefaultBackground,
		
		kDDWidgetBackgroundDefault,
		kDDWidgetBackgroundSmall,
		kDDWidgetBackgroundEditText,
		kDDWidgetBackgroundSlider,

		kDDButtonIdle,
		kDDButtonHover,
		kDDButtonDisabled,

		kDDSliderFull,

		kDDCheckboxEnabled,
		kDDCheckboxDisabled,

		kDDTabActive,
		kDDTabInactive,

		kDDScrollbarBase,
		kDDScrollbarHandle,

		kDDPopUpIdle,
		kDDPopUpHover,
		
		kDDCaret,
		kDDSeparator,
		kDrawDataMAX,
		kDDNone = -1
	};
	
	enum TextColor {
		kTextColorNone = -1,
		kTextColorDefault,
		kTextColorHover,
		kTextColorDisabled,
		kTextColorInverted,
		kTextColorMAX
	};
	
	struct DrawDataInfo {
		DrawData id;
		const char *name;
		bool buffer;
		DrawData parent;
	};
	
	struct DrawQueue {
		DrawData type;
		Common::Rect area;
		uint32 dynData;
	};
	
	struct DrawQueueText {
		DrawData type;
		Common::Rect area;
		Common::String text;
		TextColor colorId;
		TextAlign align;
	};
	
	static const DrawDataInfo kDrawData[];
	ThemeRenderer(Common::String themeName, GraphicsMode mode);

	~ThemeRenderer() {
		freeRenderer();
		freeScreen();
		unloadTheme();
		delete _parser;
	}

	// virtual methods from Theme
	bool init();
	void deinit();
	void clearAll();

	void refresh() {}
	void enable();
	void disable();

	void closeAllDialogs() {}
	
	void updateScreen();
	void resetDrawArea() {}

	void openDialog(bool top);

	/** Font management */
	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }

	/** Widget drawing */
	void drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background = kWidgetBackgroundPlain, WidgetStateInfo state = kStateEnabled);
	void drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state = kStateEnabled, uint16 hints = 0);
	void drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state = kStateEnabled, int alpha = 256, bool themeTrans = false);
	void drawSlider(const Common::Rect &r, int width, WidgetStateInfo state = kStateEnabled);
	void drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state = kStateEnabled);
	void drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state = kStateEnabled);
	void drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state = kStateEnabled);
	void drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state = kStateEnabled, TextAlign align = kTextAlignLeft);
	void drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state = kStateEnabled);
	void drawLineSeparator(const Common::Rect &r, WidgetStateInfo state = kStateEnabled);

	void drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state);
	void drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font);
	void drawChar(const Common::Rect &r, byte ch, const Graphics::Font *font, WidgetStateInfo state) {}

	bool addDirtyRect(Common::Rect r, bool backup = false, bool special = false) {
//		r.grow(kDirtyRectangleThreshold);
		_dirtyScreen.push_back(r);
		return true;
	}

	// custom stuff - tanoku
	DrawData getDrawDataId(Common::String &name) {
		for (int i = 0; i < kDrawDataMAX; ++i)
			if (name.compareToIgnoreCase(kDrawData[i].name) == 0)
				return kDrawData[i].id;

		return kDDNone;
	}

	void addDrawStep(Common::String &drawDataId, Graphics::DrawStep step);
	bool addDrawData(DrawData data_id, bool cached);
	bool addTextStep(Common::String &drawDataId, Graphics::TextStep step);

	ThemeParser *parser() {
		return _parser;
	}

	bool ready() {
		return _initOk && _themeOk;
	}

	bool loadTheme(Common::String themeName);
	void setGraphicsMode(GraphicsMode mode);

	void finishBuffering() {
		_buffering = false;
	}

protected:
	template<typename PixelType> void screenInit(bool backBuffer);

	bool loadThemeXML(Common::String themeName);
	bool loadDefaultXML();

	void unloadTheme() {
		if (!_themeOk)
			return;

		for (int i = 0; i < kDrawDataMAX; ++i) {
			delete _widgets[i];
			_widgets[i] = 0;
		}

		_themeOk = false;
	}

	void screenChange() {}
	void renderDirtyScreen();

	void freeRenderer() {
		delete _vectorRenderer;
		_vectorRenderer = 0;
	}
	
	void freeBackbuffer() {
		if (_backBuffer != 0) {
			_backBuffer->free();
			delete _backBuffer;
			_backBuffer = 0;
		}
	}

	void freeScreen() {
		if (_screen != 0) {
			_screen->free();
			delete _screen;
			_screen = 0;
		}
	}

	bool needRedraw() {
		return true;
	}

	void redrawDialogStack();

	bool hasWidgetText(DrawData type) {
		return (_widgets[type] != 0 && _widgets[type]->_hasText);
	}
	
	bool isWidgetCached(DrawData type, const Common::Rect &r);
	void drawCached(DrawData type, const Common::Rect &r);
	void calcBackgroundOffset(DrawData type);

	inline void drawDD(const DrawQueue &q, bool draw = true, bool restore = false);
	inline void drawDDText(const DrawQueueText &q);
	inline void queueDD(DrawData type,  const Common::Rect &r, uint32 dynamic = 0);
	inline void queueDDText(DrawData type, const Common::Rect &r, const Common::String &text, 
							TextColor colorId = kTextColorNone, TextAlign align = kTextAlignLeft);
	
	inline void debugWidgetPosition(const char *name, const Common::Rect &r);

	// TODO
	void restoreBackground(Common::Rect r, bool special = false);

	int getTabSpacing() const {
		return 0;
	}

	int getTabPadding() const {
		return 3;
	}
	
	TextColor getTextColor(WidgetStateInfo state) {
		switch (state) {
			case kStateDisabled:
			return kTextColorDisabled;
			
			case kStateHighlight:
			return kTextColorHover;
			
			default:
			return kTextColorDefault;
		}
	}

	OSystem *_system;
	Graphics::VectorRenderer *_vectorRenderer;
	GUI::ThemeParser *_parser;

	Graphics::Surface *_screen;
	Graphics::Surface *_backBuffer;
	bool _buffering;

	int _bytesPerPixel;
	GraphicsMode _graphicsMode;

	Common::String _fontName;
	const Graphics::Font *_font;

	WidgetDrawData *_widgets[kDrawDataMAX];
	Graphics::TextStep _texts[kTextColorMAX];
	Common::Array<Common::Rect> _dirtyScreen;
	
	Common::List<DrawQueue> _bufferQueue;
	Common::List<DrawQueue> _screenQueue;
	Common::List<DrawQueueText> _textQueue;

	bool _initOk;
	bool _themeOk;
	bool _caching;
	bool _enabled;

	Common::String _themeName;
};

} // end of namespace GUI.

#endif
