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

#include "graphics/VectorRenderer.h"

namespace GUI {

struct WidgetDrawData;
class InterfaceManager;

class InterfaceManager {

public:
	enum Graphics_Mode {
		kGfxDisabled = 0,
		kGfxStandard16bit,
		kGfxAntialias16bit
	};

	enum DrawData {
		kDrawDataBackground,
		kDrawDataButton,
		kDrawDataSurface,
		kDrawDataSlider,
		kDrawDataCheckbox,
		kDrawDataTab,
		kDrawDataScrollBar,
		kDrawDataPopUp,
		kDrawDataCaret,
		kDrawDataSeparator,
		kDrawDataMAX
	};

	enum FontStyle {
		kFontStyleBold = 0,			//! A bold font. This is also the default font.
		kFontStyleNormal = 1,		//! A normal font.
		kFontStyleItalic = 2,		//! Italic styled font.
		kFontStyleFixedNormal = 3,	//! Fixed size font.
		kFontStyleFixedBold = 4,	//! Fixed size bold font.
		kFontStyleFixedItalic = 5,	//! Fixed size italic font.
		kFontStyleMax
	};

	enum State {
		kStateDisabled,		//! Indicates that the widget is disabled, that does NOT include that it is invisible
		kStateEnabled,		//! Indicates that the widget is enabled
		kStateHighlight		//! Indicates that the widget is highlighted by the user
	};

	//! Widget background type
	enum WidgetBackground {
		kWidgetBackgroundNo,			//! No background at all
		kWidgetBackgroundPlain,			//! Simple background, this may not include borders
		kWidgetBackgroundBorder,		//! Same as kWidgetBackgroundPlain just with a border
		kWidgetBackgroundBorderSmall,	//! Same as kWidgetBackgroundPlain just with a small border
		kWidgetBackgroundEditText,		//! Background used for edit text fields
		kWidgetBackgroundSlider			//! Background used for sliders
	};

	typedef State WidgetStateInfo;

	//! State of the scrollbar
	enum ScrollbarState {
		kScrollbarStateNo,
		kScrollbarStateUp,
		kScrollbarStateDown,
		kScrollbarStateSlider,
		kScrollbarStateSinglePage
	};

	//! Defined the align of the text
	enum TextAlign {
		kTextAlignLeft,		//! Text should be aligned to the left
		kTextAlignCenter,	//! Text should be centered
		kTextAlignRight		//! Text should be aligned to the right
	};


	InterfaceManager(OSystem *system, Graphics_Mode mode) : _vectorRenderer(0), 
		_system(system), _graphicsMode(kGfxDisabled), _screen(0), _bytesPerPixel(0) {

		setGraphicsMode(mode);
	}

	~InterfaceManager() {
		freeRenderer();
		freeScreen();
	}

	void setGraphicsMode(Graphics_Mode mode);
	int runGUI();
	void init();

	/** Font management */
	const Graphics::Font *getFont(FontStyle font) const { return _font; }
	int getFontHeight(FontStyle font = kFontStyleBold) const { if (_initOk) return _font->getFontHeight(); return 0; }
	int getStringWidth(const Common::String &str, FontStyle font) const { if (_initOk) return _font->getStringWidth(str); return 0; }
	int getCharWidth(byte c, FontStyle font) const { if (_initOk) return _font->getCharWidth(c); return 0; }

	/** Widget drawing */
	void drawWidgetBackground(int x, int y, uint16 hints, WidgetBackground background, WidgetStateInfo state, float scale = 1.0f);
	void drawButton(int x, int y, const Common::String &str, WidgetStateInfo state, uint16 hints, float scale = 1.0f);
	void drawSurface(int x, int y, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans, float scale = 1.0f);
	void drawSlider(int x, int y, int width, WidgetStateInfo state, float scale = 1.0f);
	void drawCheckbox(int x, int y, const Common::String &str, bool checked, WidgetStateInfo state, float scale = 1.0f);
	void drawTab(int x, int y, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state, float scale = 1.0f);
	void drawScrollbar(int x, int y, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state, float scale = 1.0f);
	void drawPopUpWidget(int x, int y, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align, float scale = 1.0f);
	void drawCaret(int x, int y, bool erase, WidgetStateInfo state, float scale = 1.0f);
	void drawLineSeparator(int x, int y, WidgetStateInfo state, float scale = 1.0f);

protected:
	template<typename PixelType> void screenInit();

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

	OSystem *_system;
	Graphics::VectorRenderer *_vectorRenderer;
	Graphics::Surface *_screen;

	int _bytesPerPixel;
	Graphics_Mode _graphicsMode;

	Common::String _fontName;
	const Graphics::Font *_font;

	WidgetDrawData *_widgets[kDrawDataMAX];

	bool _initOk;
	bool _caching;
};

struct WidgetDrawData {
	Graphics::DrawStep **_steps;
	int _stepCount;

	bool _cached;
	Graphics::Surface *_surfaceCache;

	InterfaceManager::DrawData _type;
};

} // end of namespace GUI.

#endif
