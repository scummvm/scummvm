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

#include "gui/InterfaceManager.h"
#include "graphics/VectorRenderer.h"

DECLARE_SINGLETON(GUI::InterfaceManager);

namespace GUI {

using namespace Graphics;

const char *InterfaceManager::kDrawDataStrings[] = {
	"mainmenu_bg",
	"special_bg",
	"plain_bg",
	"default_bg",

	"button_idle",
	"button_hover",

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

InterfaceManager::InterfaceManager() : 
	_vectorRenderer(0), _system(0), _graphicsMode(kGfxDisabled), 
	_screen(0), _bytesPerPixel(0), _initOk(false), _themeOk(false) {
	_system = g_system;
	_parser = new ThemeParser();

	for (int i = 0; i < kDrawDataMAX; ++i) {
		_widgets[i] = 0;
	}

	setGraphicsMode(kGfxStandard16bit);
}

template<typename PixelType> 
void InterfaceManager::screenInit() {
	freeScreen();

	_screen = new Surface;
	_screen->create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(PixelType));
	_system->clearOverlay();
}

void InterfaceManager::setGraphicsMode(Graphics_Mode mode) {
	if (mode == _graphicsMode)
		return;

	_graphicsMode = mode;

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

void InterfaceManager::addDrawStep(Common::String &drawDataId, Graphics::DrawStep *step) {
	DrawData id = getDrawDataId(drawDataId);
	
	assert(_widgets[id] != 0);
	_widgets[id]->_steps.push_back(step);
}

bool InterfaceManager::addDrawData(DrawData data_id, bool cached) {
	assert(data_id >= 0 && data_id < kDrawDataMAX);

	if (_widgets[data_id] != 0)
		return false;

	_widgets[data_id] = new WidgetDrawData;
	_widgets[data_id]->_cached = cached;
	_widgets[data_id]->_type = data_id;

	return true;
}

bool InterfaceManager::loadTheme(Common::String themeName) {
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

bool InterfaceManager::loadThemeXML(Common::String themeName) {
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

bool InterfaceManager::init() {
	return false;
}

bool InterfaceManager::isWidgetCached(DrawData type, const Common::Rect &r) {
	return _widgets[type] && _widgets[type]->_cached &&
		_widgets[type]->_surfaceCache->w == r.width() && 
		_widgets[type]->_surfaceCache->h == r.height();
}

void InterfaceManager::drawCached(DrawData type, const Common::Rect &r) {
	assert(_widgets[type]->_surfaceCache->bytesPerPixel == _screen->bytesPerPixel);
	_vectorRenderer->blitSurface(_widgets[type]->_surfaceCache, r);
}

void InterfaceManager::drawDD(DrawData type, const Common::Rect &r) {
	if (isWidgetCached(type, r)) {
		drawCached(type, r);
	} else {
		for (uint i = 0; i < _widgets[type]->_steps.size(); ++i)
			_vectorRenderer->drawStep(r, *_widgets[type]->_steps[i]);
	}
}

void InterfaceManager::drawButton(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, uint16 hints) {
	if (!_initOk)
		return;

	if (state == kStateEnabled)
		drawDD(kDDButtonIdle, r);
	else if (state == kStateHighlight)
		drawDD(kDDButtonHover, r);

	// TODO: Add text drawing.

	addDirtyRect(r);
}

void InterfaceManager::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!_initOk)
		return;

	drawDD(kDDSeparator, r);
	addDirtyRect(r);
}

void InterfaceManager::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!_initOk)
		return;

	drawDD(checked ? kDDCheckboxEnabled : kDDCheckboxDisabled, r);

	Common::Rect r2 = r;
	r2.left += 16; // TODO: add variable for checkbox text offset.

	// TODO: text drawing
//	getFont()->drawString(&_screen, str, r2.left, r2.top, r2.width(), getColor(state), Graphics::kTextAlignLeft, 0, false);

	addDirtyRect(r);
}

void InterfaceManager::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state) {
	if (!_initOk)
		return;

	drawDD(kDDSliderEmpty, r);

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));

	drawDD(kDDSliderFull, r2);

	addDirtyRect(r);
}

void InterfaceManager::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState sb_state, WidgetStateInfo state) {
	if (!_initOk)
		return;
}

int InterfaceManager::runGUI() {
	Common::EventManager *eventMan = _system->getEventManager();

	if (!loadTheme("modern_theme.xml"))
		return 0;

	_system->showOverlay();

	bool running = true;
	while (running) { // draw!!

		drawDD(kDDMainDialogBackground, Common::Rect());
		drawDD(kDDButtonIdle, Common::Rect(32, 32, 128, 128));

		_vectorRenderer->copyFrame(_system);

		Common::Event event;
		_system->delayMillis(100);
		while (eventMan->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT)
				running = false;
		}
	}

	_system->hideOverlay();
	return 1;
}



} // end of namespace GUI.
