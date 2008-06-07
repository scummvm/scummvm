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

namespace GUI {

using namespace Graphics;

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
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>();
		break;

	case kGfxAntialias16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>();
		break;

	default:
		return;
	}

	_vectorRenderer = createRenderer(mode);
	_vectorRenderer->setSurface(_screen);
}

void InterfaceManager::init() {

}

void InterfaceManager::drawWidgetBackground(int x, int y, uint16 hints, WidgetBackground background, WidgetStateInfo state, float scale){

}

void InterfaceManager::drawButton(int x, int y, const Common::String &str, WidgetStateInfo state, uint16 hints, float scale) {

}

void InterfaceManager::drawSurface(int x, int y, const Graphics::Surface &surface, WidgetStateInfo state, int alpha, bool themeTrans, float scale) {

}

void InterfaceManager::drawSlider(int x, int y, int width, WidgetStateInfo state, float scale) {

}

void InterfaceManager::drawCheckbox(int x, int y, const Common::String &str, bool checked, WidgetStateInfo state, float scale) {

}

void InterfaceManager::drawTab(int x, int y, int tabHeight, int tabWidth, const Common::Array<Common::String> &tabs, int active, uint16 hints, int titleVPad, WidgetStateInfo state, float scale) {

}

void InterfaceManager::drawScrollbar(int x, int y, int sliderY, int sliderHeight, ScrollbarState, WidgetStateInfo state, float scale) {

}

void InterfaceManager::drawPopUpWidget(int x, int y, const Common::String &sel, int deltax, WidgetStateInfo state, TextAlign align, float scale) {

}

void InterfaceManager::drawCaret(int x, int y, bool erase, WidgetStateInfo state, float scale) {

}

void InterfaceManager::drawLineSeparator(int x, int y, WidgetStateInfo state, float scale) {

}

int InterfaceManager::runGUI() {
	Common::EventManager *eventMan = _system->getEventManager();
	_system->showOverlay();

	Graphics::DrawStep *steps = new Graphics::DrawStep[5];

	steps[0].color1.r = 214;
	steps[0].color1.g = 113;
	steps[0].color1.b = 8;
	steps[0].color2.r = 240;
	steps[0].color2.g = 200;
	steps[0].color2.b = 25;
	steps[0].fill_mode = VectorRenderer::kFillGradient;
	steps[0].drawing_call = &VectorRenderer::drawCallback_FILLSURFACE;
	steps[0].flags = DrawStep::kStepSetGradient | DrawStep::kStepSetFillMode;

	steps[1].color1.r = 206;
	steps[1].color1.g = 121;
	steps[1].color1.b = 99;
	steps[1].color2.r = 173;
	steps[1].color2.g = 40;
	steps[1].color2.b = 8;
	steps[1].x = 500;
	steps[1].y = 95;
	steps[1].r = 8;
	steps[1].w = 120;
	steps[1].h = 30;
	steps[1].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[1].flags = DrawStep::kStepSetGradient;

	steps[2].x = 500;
	steps[2].y = 135;
	steps[2].r = 8;
	steps[2].w = 120;
	steps[2].h = 30;
	steps[2].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[2].flags = DrawStep::kStepCallbackOnly;

	steps[3].x = 500;
	steps[3].y = 175;
	steps[3].r = 8;
	steps[3].w = 120;
	steps[3].h = 30;
	steps[3].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[3].flags = DrawStep::kStepCallbackOnly;

	bool running = true;
	while (running) { // draw!!

		for (int i = 0; i < 4; ++i)
			_vectorRenderer->drawStep(&steps[i]);

		_vectorRenderer->setFillMode(VectorRenderer::kFillGradient);
		_vectorRenderer->setFgColor(0, 0, 0);
		_vectorRenderer->drawTriangle(32, 32, 64, 64, VectorRenderer::kTriangleUp);

		_vectorRenderer->drawBeveledSquare(128, 128, 256, 64, 4);

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
