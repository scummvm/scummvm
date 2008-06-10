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
	steps[1].radius = 8; // radius
	steps[1].fill_area = true;
	steps[1].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[1].flags = DrawStep::kStepSetGradient;
	steps[1].scale = (1 << 16);

	steps[2].radius = 8; // radius
	steps[2].fill_area = false;
	steps[2].x.relative = true;
	steps[2].x.pos = 32;
	steps[2].y.relative = false;
	steps[2].y.pos = 32;
	steps[2].w = 128;
	steps[2].h = 32;
	steps[2].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[2].flags = DrawStep::kStepCallbackOnly;
	steps[2].scale = (1 << 16);

	steps[3].drawing_call = &VectorRenderer::drawCallback_ROUNDSQ;
	steps[3].flags = DrawStep::kStepCallbackOnly;

	Common::Rect area = Common::Rect(32, 32, 256, 256);

	bool running = true;
	while (running) { // draw!!

		_vectorRenderer->drawStep(Common::Rect(), &steps[0]);
		_vectorRenderer->drawStep(area, &steps[1]);
		_vectorRenderer->drawStep(area, &steps[2]);
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
