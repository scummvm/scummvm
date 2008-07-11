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

#include "common/util.h"
#include "graphics/surface.h"
#include "graphics/colormasks.h"
#include "common/system.h"
#include "common/events.h"
#include "common/config-manager.h"

#include "gui/launcher.h"

#include "gui/ThemeRenderer.h"
#include "graphics/VectorRenderer.h"

namespace GUI {

using namespace Graphics;

const char *ThemeRenderer::kDrawDataStrings[] = {
	"mainmenu_bg",
	"special_bg",
	"plain_bg",
	"default_bg",

	"button_idle",
	"button_hover",
	"button_disabled",

	"surface",

	"slider_full",
	"slider_empty",

	"checkbox_enabled",
	"checkbox_disabled",

	"tab",

	"scrollbar_base",
	"scrollbar_top",
	"scrollbar_bottom",
	"scrollbar_handle",

	"popup",
	"caret",
	"separator"
};

ThemeRenderer::ThemeRenderer(Common::String themeName, GraphicsMode mode) : 
	_vectorRenderer(0), _system(0), _graphicsMode(kGfxDisabled), 
	_screen(0), _bytesPerPixel(0), _initOk(false), _themeOk(false), _enabled(false) {
	_system = g_system;
	_parser = new ThemeParser(this);

	for (int i = 0; i < kDrawDataMAX; ++i) {
		_widgets[i] = 0;
	}

	_graphicsMode = mode;
	setGraphicsMode(_graphicsMode);

	loadConfigFile("classic");

	_initOk = true;
	_themeName = themeName;
}

bool ThemeRenderer::init() {
	// reset everything and reload the graphics
	deinit();
	setGraphicsMode(_graphicsMode);

	if (_screen->pixels) {
		_initOk = true;
		clearAll();
		resetDrawArea();
	}

	if (isThemeLoadingRequired() || !_themeOk) {
		loadTheme(_themeName);

		Theme::loadTheme(_defaultConfig);
		Theme::loadTheme(_configFile, false, true);
	}

	if (_fontName.empty()) {
		if (_screen->w >= 400 && _screen->h >= 300) {
			_font = FontMan.getFontByUsage(Graphics::FontManager::kBigGUIFont);
		} else {
			_font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
		}
	}

	return true;
}

void ThemeRenderer::deinit() {
	if (_initOk) {
		_system->hideOverlay();
		freeRenderer();
		freeScreen();
		_initOk = false;
	}
}

void ThemeRenderer::clearAll() {
	if (!_initOk)
		return;

	_system->clearOverlay();
	_system->grabOverlay((OverlayColor*)_screen->pixels, _screen->w);
}

void ThemeRenderer::enable() {
	init();
	resetDrawArea();
	_system->showOverlay();
	clearAll();
	_enabled = true;
}

void ThemeRenderer::disable() {
	_system->hideOverlay();
	_enabled = false;
}

template<typename PixelType> 
void ThemeRenderer::screenInit() {
	freeScreen();

	_screen = new Surface;
	_screen->create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(PixelType));
	_system->clearOverlay();
}

void ThemeRenderer::setGraphicsMode(GraphicsMode mode) {
	switch (mode) {
	case kGfxStandard16bit:
	case kGfxAntialias16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>();
		break;

	default:
		return;
	}

	freeRenderer();
	_vectorRenderer = createRenderer(mode);
	_vectorRenderer->setSurface(_screen);
}

void ThemeRenderer::addDrawStep(Common::String &drawDataId, Graphics::DrawStep step) {
	DrawData id = getDrawDataId(drawDataId);
	
	assert(_widgets[id] != 0);
	_widgets[id]->_steps.push_back(step);
}

bool ThemeRenderer::addTextStep(Common::String &drawDataId, Graphics::TextStep step) {
	DrawData id = getDrawDataId(drawDataId);
	
	assert(_widgets[id] != 0);
	if (_widgets[id]->_hasText == true)
		return false;
		
	_widgets[id]->_textStep = step;
	_widgets[id]->_textStep.font = 0;
	_widgets[id]->_hasText = true;

	return true;
}

bool ThemeRenderer::addDrawData(DrawData data_id, bool cached) {
	assert(data_id >= 0 && data_id < kDrawDataMAX);

	if (_widgets[data_id] != 0)
		return false;

	_widgets[data_id] = new WidgetDrawData;
	_widgets[data_id]->_cached = cached;
	_widgets[data_id]->_surfaceCache = 0;
	_widgets[data_id]->_hasText = false;

	return true;
}

bool ThemeRenderer::loadTheme(Common::String themeName) {
	unloadTheme();

	if (themeName == "builtin" && !loadDefaultXML())
		error("Could not load default embeded theme.");

	if (!loadThemeXML(themeName)) {
		warning("Could not parse custom theme '%s'.\nFalling back to default theme", themeName.c_str());
		
		if (!loadDefaultXML()) // if we can't load the embeded theme, this is a complete failure
			error("Could not load default embeded theme");
	}

	for (int i = 0; i < kDrawDataMAX; ++i) {
		if (_widgets[i] == 0) {
#ifdef REQUIRE_ALL_DD_SETS
			warning("Error when parsing custom theme '%s': Missing data assets.", themeName.c_str());
			return false;
#endif
		} else if (_widgets[i]->_cached) {
			// draw the cached widget to the cache surface
		}
	}

	_themeOk = true;
	return true;
}

bool ThemeRenderer::loadThemeXML(Common::String themeName) {
	assert(_parser);

	if (ConfMan.hasKey("themepath"))
		Common::File::addDefaultDirectory(ConfMan.get("themepath"));

#ifdef DATA_PATH
	Common::File::addDefaultDirectoryRecursive(DATA_PATH);
#endif

	if (ConfMan.hasKey("extrapath"))
		Common::File::addDefaultDirectoryRecursive(ConfMan.get("extrapath"));

	if (!parser()->loadFile(themeName + ".xml"))
		return false;
	
	return parser()->parse();
}

bool ThemeRenderer::isWidgetCached(DrawData type, const Common::Rect &r) {
	return _widgets[type] && _widgets[type]->_cached &&
		_widgets[type]->_surfaceCache->w == r.width() && 
		_widgets[type]->_surfaceCache->h == r.height();
}

void ThemeRenderer::drawCached(DrawData type, const Common::Rect &r) {
	assert(_widgets[type]->_surfaceCache->bytesPerPixel == _screen->bytesPerPixel);
	_vectorRenderer->blitSurface(_widgets[type]->_surfaceCache, r);
}

void ThemeRenderer::drawDD(DrawData type, const Common::Rect &r) {
	if (_widgets[type] == 0)
		return;
		
	if (isWidgetCached(type, r)) {
		drawCached(type, r);
	} else {
		for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[type]->_steps.begin(); 
			 step != _widgets[type]->_steps.end(); ++step)
			_vectorRenderer->drawStep(r, *step);
	}
}

void ThemeRenderer::drawDDText(DrawData type, const Common::Rect &r, const Common::String &text) {
	if (hasWidgetText(type)) {
		if (_widgets[type]->_textStep.font == 0)
			_widgets[type]->_textStep.font = _font;

		_vectorRenderer->textStep(text, r, _widgets[type]->_textStep);
	}
}

void ThemeRenderer::drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints) {
	if (!ready())
		return;
		
	DrawData dd = kDDButtonIdle;

	if (state == kStateEnabled)
		dd = kDDButtonIdle;
	else if (state == kStateHighlight)
		dd = kDDButtonHover;
	else if (state == kStateDisabled)
		dd = kDDButtonDisabled;

	drawDD(dd, r);
	drawDDText(dd, r, str);	

	addDirtyRect(r);
	debugWidgetPosition(r);
}

void ThemeRenderer::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!ready())
		return;

	drawDD(kDDSeparator, r);
	addDirtyRect(r);

	debugWidgetPosition(r);
}

void ThemeRenderer::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	drawDD(checked ? kDDCheckboxEnabled : kDDCheckboxDisabled, r2);
	
	r2.left = r2.right + checkBoxSize;
	r2.right = r.right;
	
	drawDDText(checked ? kDDCheckboxEnabled : kDDCheckboxDisabled, r2, str);

	addDirtyRect(r);
	debugWidgetPosition(r);
}

void ThemeRenderer::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state) {
	if (!ready())
		return;

	drawDD(kDDSliderEmpty, r);

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));

	drawDD(kDDSliderFull, r2);

	addDirtyRect(r);
	debugWidgetPosition(r);
}

void ThemeRenderer::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState sb_state, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state) {
	if (!ready())
		return;
		
	if (hints & THEME_HINT_MAIN_DIALOG)
		drawDD(kDDMainDialogBackground, r);
	else if (hints & THEME_HINT_SPECIAL_COLOR) 
		drawDD(kDDSpecialColorBackground, r);
	else if (hints & THEME_HINT_PLAIN_COLOR)
		drawDD(kDDPlainColorBackground, r);	
	else
		drawDD(kDDDefaultBackground, r);
	
	debugWidgetPosition(r);
}

void ThemeRenderer::drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition(r);
}

void ThemeRenderer::debugWidgetPosition(const Common::Rect &r) {
	_screen->hLine(r.left, r.top, r.right, 0xFFFF);
	_screen->hLine(r.left, r.bottom, r.right, 0xFFFF);
	_screen->vLine(r.left, r.top, r.bottom, 0xFFFF);
	_screen->vLine(r.right, r.top, r.bottom, 0xFFFF);
}

void ThemeRenderer::updateScreen() {
//	renderDirtyScreen();
	_vectorRenderer->copyWholeFrame(_system);
}

void ThemeRenderer::renderDirtyScreen() {
	// TODO: This isn't really optimized. Check dirty squares for collisions
	// and all that.
	if (_dirtyScreen.empty())
		return;

	for (uint i = 0; i < _dirtyScreen.size(); ++i)
		_vectorRenderer->copyFrame(_system, _dirtyScreen[i]);

	_dirtyScreen.clear();
}

} // end of namespace GUI.
