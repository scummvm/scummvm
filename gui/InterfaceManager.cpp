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

template<typename PixelType> 
void InterfaceManager::screenInit() {
	freeScreen();

	_screen = new Graphics::Surface;
	_screen->create(_system->getOverlayWidth(), _system->getOverlayHeight(), sizeof(PixelType));
	_system->clearOverlay();
}

void InterfaceManager::setGraphicsMode(Graphics_Mode mode) {
	if (mode == _graphicsMode)
		return;

	_graphicsMode = mode;

	switch (mode) {
	case GFX_Standard_16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>();
		break;

	case GFX_Antialias_16bit:
		_bytesPerPixel = sizeof(uint16);
		screenInit<uint16>();
		break;
	}

	_vectorRenderer = Graphics::createRenderer(mode);
	_vectorRenderer->setSurface(_screen);
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
	steps[0].fill_mode = Graphics::kFillMode_Gradient;
	steps[0].drawing_call = &Graphics::VectorRenderer::drawCallback_FILLSURFACE;
	steps[0].flags = Graphics::kDrawStep_SetGradient | Graphics::kDrawStep_SetFillMode;

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
	steps[1].drawing_call = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	steps[1].flags = Graphics::kDrawStep_SetGradient;

	steps[2].x = 500;
	steps[2].y = 135;
	steps[2].r = 8;
	steps[2].w = 120;
	steps[2].h = 30;
	steps[2].drawing_call = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	steps[2].flags = Graphics::kDrawStep_CallbackOnly;

	steps[3].x = 500;
	steps[3].y = 175;
	steps[3].r = 8;
	steps[3].w = 120;
	steps[3].h = 30;
	steps[3].drawing_call = &Graphics::VectorRenderer::drawCallback_ROUNDSQ;
	steps[3].flags = Graphics::kDrawStep_CallbackOnly;

	bool running = true;
	while (running) { // draw!!

		for (int i = 0; i < 4; ++i)
			_vectorRenderer->drawStep(&steps[i]);

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
