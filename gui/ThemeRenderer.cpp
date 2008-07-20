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

const ThemeRenderer::DrawDataInfo ThemeRenderer::kDrawData[] = {
	{kDDMainDialogBackground, "mainmenu_bg", true, kDDNone},
	{kDDSpecialColorBackground, "special_bg", true, kDDNone},
	{kDDPlainColorBackground, "plain_bg", true, kDDNone},
	{kDDDefaultBackground, "default_bg", true, kDDNone},

	{kDDWidgetBackgroundDefault, "widget_default", true, kDDNone},
	{kDDWidgetBackgroundSmall, "widget_small", true, kDDNone},
	{kDDWidgetBackgroundEditText, "widget_textedit", true, kDDNone},
	{kDDWidgetBackgroundSlider, "widget_slider", true, kDDNone},

	{kDDButtonIdle, "button_idle", true, kDDNone},
	{kDDButtonHover, "button_hover", false, kDDButtonIdle},
	{kDDButtonDisabled, "button_disabled", true, kDDNone},

	{kDDSliderFull, "slider_full", false, kDDWidgetBackgroundSlider},

	{kDDCheckboxEnabled, "checkbox_enabled", false, kDDCheckboxDisabled},
	{kDDCheckboxDisabled, "checkbox_disabled", true, kDDNone},

	{kDDTabActive, "tab_active", false, kDDTabInactive},
	{kDDTabInactive, "tab_inactive", true, kDDNone},

	{kDDScrollbarBase, "scrollbar_base", true, kDDNone},
	{kDDScrollbarHandle, "scrollbar_handle", false, kDDScrollbarBase},

	{kDDPopUpIdle, "popup_idle", true, kDDNone},
	{kDDPopUpHover, "popup_hover", false, kDDPopUpIdle},

	{kDDCaret, "caret", false, kDDNone},
	{kDDSeparator, "separator", true, kDDNone},
};


ThemeRenderer::ThemeRenderer(Common::String themeName, GraphicsMode mode) : 
	_vectorRenderer(0), _system(0), _graphicsMode(kGfxDisabled), 
	_screen(0), _backBuffer(0), _bytesPerPixel(0), _initOk(false), 
	_themeOk(false), _enabled(false), _buffering(false) {
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
void ThemeRenderer::screenInit(bool backBuffer) {
	freeScreen();
	freeBackbuffer();
	
	if (backBuffer) {
		_backBuffer = new Surface;
		_backBuffer->create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(PixelType));
	}
	
	_screen = new Surface;
	_screen->create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(PixelType));
	_system->clearOverlay();
}

void ThemeRenderer::setGraphicsMode(GraphicsMode mode) {
	switch (mode) {
	case kGfxStandard16bit:
	case kGfxAntialias16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>(kEnableBackCaching);
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

	step.font = 0;
	
	if (id != -1) {
		assert(_widgets[id] != 0);
		if (_widgets[id]->_hasText == true)
			return false;

		_widgets[id]->_textStep = step;
		_widgets[id]->_hasText = true;
	} else {
		if (drawDataId == "default") {
			_texts[kTextColorDefault] = step;
		} else if (drawDataId == "hover") {
			_texts[kTextColorHover] = step;
		} else if (drawDataId == "disabled") {
			_texts[kTextColorDisabled] = step;
		} else return false;
	}

	return true;
}

bool ThemeRenderer::addDrawData(DrawData data_id, bool cached) {
	assert(data_id >= 0 && data_id < kDrawDataMAX);

	if (_widgets[data_id] != 0)
		return false;

	_widgets[data_id] = new WidgetDrawData;
	_widgets[data_id]->_cached = cached;
	_widgets[data_id]->_buffer = kDrawData[data_id].buffer;
	_widgets[data_id]->_surfaceCache = 0;
	_widgets[data_id]->_hasText = false;

	// TODO: set this only when needed
	// possibly add an option to the parser to set this
	// on each drawdata...
//	if (data_id >= kDDMainDialogBackground && data_id <= kDDWidgetBackgroundSlider)
//		_widgets[data_id]->_buffer = true;

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
		} else {
			calcBackgroundOffset((DrawData)i);

			// TODO: draw the cached widget to the cache surface
			if (_widgets[i]->_cached) {}
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

void ThemeRenderer::queueDD(DrawData type, const Common::Rect &r, uint32 dynamic) {
	if (_widgets[type] == 0)
		return;
		
	DrawQueue q;	
	q.type = type;
	q.area = r;
	q.dynData = dynamic;
	
	if (_buffering) {
		warning("Queued up a '%s' for the %s", kDrawData[type].name, _widgets[type]->_buffer ? "buffer" : "screen");
		
		if (_widgets[type]->_buffer)
			_bufferQueue.push_back(q);
		else {
			if (kDrawData[type].parent != kDDNone)
				queueDD(kDrawData[type].parent, r);

			_screenQueue.push_back(q);
		}
	} else {
		warning("Drawing a '%s' directly!", kDrawData[type].name);
		drawDD(q, !_widgets[type]->_buffer, _widgets[type]->_buffer);
	}
}

void ThemeRenderer::queueDDText(DrawData type, const Common::Rect &r, const Common::String &text, TextColor colorId, TextAlign align) {
	DrawQueueText q;
	q.type = type;
	q.area = r;
	q.text = text;
	q.colorId = colorId;
	q.align = align;
	
	if (_buffering) {		
		_textQueue.push_back(q);
	} else {
		drawDDText(q);
	}
}

void ThemeRenderer::drawDD(const DrawQueue &q, bool draw, bool restore) {
	Common::Rect extendedRect = q.area;
	extendedRect.grow(kDirtyRectangleThreshold);
	extendedRect.right += _widgets[q.type]->_backgroundOffset;
	extendedRect.bottom += _widgets[q.type]->_backgroundOffset;

	if (restore)
		restoreBackground(extendedRect);
		
	if (draw) {
		if (isWidgetCached(q.type, q.area)) {
			drawCached(q.type, q.area);
		} else {
			for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[q.type]->_steps.begin(); 
				 step != _widgets[q.type]->_steps.end(); ++step)
				_vectorRenderer->drawStep(q.area, *step, q.dynData);
		}
	}
	
	addDirtyRect(extendedRect);
}

void ThemeRenderer::drawDDText(const DrawQueueText &q) {	
	if (q.type == kDDNone) {
		restoreBackground(q.area);
		if (_texts[q.colorId].font == 0)
			_texts[q.colorId].font = _font;

		_vectorRenderer->textStep(q.text, q.area, _texts[q.colorId], q.align);
	} else {
		if (_widgets[q.type]->_textStep.font == 0)
			_widgets[q.type]->_textStep.font = _font;

		_vectorRenderer->textStep(q.text, q.area, _widgets[q.type]->_textStep);
	}
}

void ThemeRenderer::calcBackgroundOffset(DrawData type) {
	uint maxShadow = 0;
	for (Common::List<Graphics::DrawStep>::const_iterator step = _widgets[type]->_steps.begin(); 
		step != _widgets[type]->_steps.end(); ++step) {
		if (((*step).autoWidth || (*step).autoHeight) && (*step).shadow > maxShadow) 
			maxShadow = (*step).shadow;
	}

	_widgets[type]->_backgroundOffset = maxShadow;
}

void ThemeRenderer::restoreBackground(Common::Rect r, bool special) {
	const OverlayColor *src = (const OverlayColor*)_backBuffer->getBasePtr(r.left, r.top);
	OverlayColor *dst = (OverlayColor*)_screen->getBasePtr(r.left, r.top);

	int h = r.height();
	int w = r.width();
	while (h--) {
		memcpy(dst, src, w * sizeof(OverlayColor));
		src += _backBuffer->w;
		dst += _screen->w;
	}

//	debugWidgetPosition("", r);
//	printf(" BG_RESTORE ");
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

	queueDD(dd, r);
	queueDDText(dd, r, str);	
}

void ThemeRenderer::drawLineSeparator(const Common::Rect &r, WidgetStateInfo state) {
	if (!ready())
		return;

	queueDD(kDDSeparator, r);
}

void ThemeRenderer::drawCheckbox(const Common::Rect &r, const Common::String &str, bool checked, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	const int checkBoxSize = MIN((int)r.height(), getFontHeight());

	r2.bottom = r2.top + checkBoxSize;
	r2.right = r2.left + checkBoxSize;

	queueDD(checked ? kDDCheckboxEnabled : kDDCheckboxDisabled, r2);
	
	r2.left = r2.right + checkBoxSize;
	r2.right = r.right;
	
	queueDDText(checked ? kDDCheckboxEnabled : kDDCheckboxDisabled, r2, str);
}

void ThemeRenderer::drawSlider(const Common::Rect &r, int width, WidgetStateInfo state) {
	if (!ready())
		return;

	Common::Rect r2 = r;
	r2.setWidth(MIN((int16)width, r.width()));

	queueDD(kDDSliderFull, r2);
}

void ThemeRenderer::drawScrollbar(const Common::Rect &r, int sliderY, int sliderHeight, ScrollbarState sb_state, WidgetStateInfo state) {
	if (!ready())
		return;
		
	queueDD(kDDScrollbarBase, r);
	// TODO: Need to find a scrollbar in the GUI for testing... :p
}

void ThemeRenderer::drawDialogBackground(const Common::Rect &r, uint16 hints, WidgetStateInfo state) {
	if (!ready())
		return;
		
	if (hints & THEME_HINT_MAIN_DIALOG) {
		queueDD(kDDMainDialogBackground, r);
	} else if (hints & THEME_HINT_SPECIAL_COLOR) { 
		queueDD(kDDSpecialColorBackground, r);
	} else if (hints & THEME_HINT_PLAIN_COLOR) {
		queueDD(kDDPlainColorBackground, r);	
	} else {
		queueDD(kDDDefaultBackground, r);
	}
}

void ThemeRenderer::drawCaret(const Common::Rect &r, bool erase, WidgetStateInfo state) {
	if (!ready())
		return;

	debugWidgetPosition("Caret", r);
}

void ThemeRenderer::drawPopUpWidget(const Common::Rect &r, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align) {
	if (!ready())
		return;
		
	DrawData dd = (state == kStateHighlight) ? kDDPopUpHover : kDDPopUpIdle;
	
	queueDD(dd, r);
	
	if (!sel.empty()) {
		Common::Rect text(r.left, r.top, r.right - 16, r.bottom);
		queueDDText(dd, text, sel);
	}
}

void ThemeRenderer::drawSurface(const Common::Rect &r, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans) {
	if (!ready())
		return;

	debugWidgetPosition("Surface", r);
}

void ThemeRenderer::drawWidgetBackground(const Common::Rect &r, uint16 hints, WidgetBackground background, WidgetStateInfo state) {
	if (!ready())
		return;
		
	switch (background) {
	case kWidgetBackgroundBorderSmall:
		queueDD(kDDWidgetBackgroundSmall, r);
		break;
		
	case kWidgetBackgroundEditText:
		queueDD(kDDWidgetBackgroundEditText, r);
		break;
		
	case kWidgetBackgroundSlider:
		queueDD(kDDWidgetBackgroundSlider, r);
		break;
		
	default:
		queueDD(kDDWidgetBackgroundDefault, r);
		break;
	}
}

void ThemeRenderer::drawTab(const Common::Rect &r, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state) {
	if (!ready())
		return;
		
	const int tabOffset = 1;
	
	for (int i = 0; i < (int)tabs.size(); ++i) {
		if (i == active)
			continue;

		Common::Rect tabRect(r.left + i * (tabWidth + tabOffset), r.top, r.left + i * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		queueDD(kDDTabInactive, tabRect);
		queueDDText(kDDTabInactive, tabRect, tabs[i]);
	}
	
	if (active >= 0) {
		Common::Rect tabRect(r.left + active * (tabWidth + tabOffset), r.top, r.left + active * (tabWidth + tabOffset) + tabWidth, r.top + tabHeight);
		const uint16 tabLeft = active * (tabWidth + tabOffset);
		const uint16 tabRight =  r.right - tabRect.right;
		queueDD(kDDTabActive, tabRect, (tabLeft << 16) | (tabRight & 0xFFFF));
		queueDDText(kDDTabActive, tabRect, tabs[active]);
	}
}

void ThemeRenderer::drawText(const Common::Rect &r, const Common::String &str, WidgetStateInfo state, TextAlign align, bool inverted, int deltax, bool useEllipsis, FontStyle font) {
	if (!_initOk)
		return;

	// TODO: Queue this up too!
	// restoreBackground(r);
	// getFont(font)->drawString(_screen, str, r.left, r.top, r.width(), getTextColor(state), convertAligment(align), deltax, useEllipsis);
	// addDirtyRect(r);
	
	queueDDText(kDDNone, r, str, getTextColor(state), align);
}

void ThemeRenderer::debugWidgetPosition(const char *name, const Common::Rect &r) {
	_font->drawString(_screen, name, r.left, r.top, r.width(), 0xFFFF, Graphics::kTextAlignRight, 0, true);
	_screen->hLine(r.left, r.top, r.right, 0xFFFF);
	_screen->hLine(r.left, r.bottom, r.right, 0xFFFF);
	_screen->vLine(r.left, r.top, r.bottom, 0xFFFF);
	_screen->vLine(r.right, r.top, r.bottom, 0xFFFF);
}

void ThemeRenderer::updateScreen() {
//	renderDirtyScreen();
	
	if (!_bufferQueue.empty()) {
		_vectorRenderer->setSurface(_backBuffer);
		
		for (Common::List<DrawQueue>::const_iterator q = _bufferQueue.begin(); q != _bufferQueue.end(); ++q)
			drawDD(*q, true, false);
			
		_vectorRenderer->setSurface(_screen);
		_bufferQueue.clear();
		memcpy(_screen->pixels, _backBuffer->pixels, _screen->w * _screen->h * _screen->bytesPerPixel);
	}
	
	if (!_screenQueue.empty()) {
		_vectorRenderer->disableShadows();
		for (Common::List<DrawQueue>::const_iterator q = _screenQueue.begin(); q != _screenQueue.end(); ++q)
			drawDD(*q, true, false);
			
		_vectorRenderer->enableShadows();
		_screenQueue.clear();
	}
	
	if (!_textQueue.empty()) {
		for (Common::List<DrawQueueText>::const_iterator q = _textQueue.begin(); q != _textQueue.end(); ++q)
			drawDDText(*q);
			
		_textQueue.clear();
	}
		
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

void ThemeRenderer::openDialog(bool doBuffer) {
	if (doBuffer)
		_buffering = true;

//	memcpy(_backBuffer->pixels, _screen->pixels, _screen->w * _screen->h * _screen->bytesPerPixel);
}

} // end of namespace GUI.
