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

#include "gui/InterfaceManager.h"
#include "graphics/VectorRenderer.h"

DECLARE_SINGLETON(GUI::InterfaceManager);

namespace GUI {

using namespace Graphics;

InterfaceManager::InterfaceManager() : 
	_vectorRenderer(0), _system(0), _graphicsMode(kGfxDisabled), 
	_screen(0), _bytesPerPixel(0) {
	_system = g_system;

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
		for (int i = 0; i < _widgets[type]->_stepCount; ++i)
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
	_system->showOverlay();

	Graphics::DrawStep *steps = new Graphics::DrawStep[5];

	steps[0].gradColor1.r = 214;
	steps[0].gradColor1.g = 113;
	steps[0].gradColor1.b = 8;
	steps[0].gradColor2.r = 240;
	steps[0].gradColor2.g = 200;
	steps[0].gradColor2.b = 25;
	steps[0].fillMode = VectorRenderer::kFillGradient;
	steps[0].drawingCall = &VectorRenderer::drawCallback_FILLSURFACE;
	steps[0].flags = DrawStep::kStepSetGradient | DrawStep::kStepSetFillMode;

	steps[1].gradColor1.r = 206;
	steps[1].gradColor1.g = 121;
	steps[1].gradColor1.b = 99;
	steps[1].gradColor2.r = 173;
	steps[1].gradColor2.g = 40;
	steps[1].gradColor2.b = 8;
	steps[1].radius = 8; // radius
	steps[1].fillArea = true;
	steps[1].drawingCall = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[1].flags = DrawStep::kStepSetGradient;
	steps[1].scale = (1 << 16);

	steps[2].radius = 8; // radius
	steps[2].fillArea = false;
	steps[2].x.relative = true;
	steps[2].x.pos = 32;
	steps[2].y.relative = false;
	steps[2].y.pos = 32;
	steps[2].w = 128;
	steps[2].h = 32;
	steps[2].drawingCall = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[2].flags = DrawStep::kStepCallbackOnly;
	steps[2].scale = (1 << 16);

	steps[3].fgColor.r = 255;
	steps[3].fgColor.g = 255;
	steps[3].fgColor.b = 255;
	steps[3].flags = DrawStep::kStepSettingsOnly | DrawStep::kStepSetFG;

	Common::Rect area = Common::Rect(32, 32, 256, 256);

	bool running = true;
	while (running) { // draw!!

		_vectorRenderer->drawStep(Common::Rect(), steps[0]);
		_vectorRenderer->drawStep(Common::Rect(), steps[3]);
		_vectorRenderer->drawStep(area, steps[1]);
		_vectorRenderer->drawStep(area, steps[2]);
//		_vectorRenderer->drawStep(Common::Rect(32, 32, 256, 256), &steps[3]);

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
