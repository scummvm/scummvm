/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/credits.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Credits);
}

namespace Nancy {
namespace State {

void Credits::process() {
	switch (_state) {
	case kInit:
		init();
		// fall through
	case kRun:
		run();
		break;
	}
}

void Credits::init() {
	_creditsData = g_nancy->_creditsData;
	assert(_creditsData);

	_background.init(_creditsData->imageName);
	_textSurface.moveTo(_creditsData->textScreenPosition);

	drawTextSurface(0);

	_textSurface._drawSurface.create(_fullTextSurface, _textSurface.getBounds());
	_textSurface.init();

	g_nancy->_sound->stopSound("MSND");

	g_nancy->_sound->loadSound(_creditsData->sound);
	g_nancy->_sound->playSound(_creditsData->sound);

	_background.registerGraphics();
	_textSurface.registerGraphics();

	g_nancy->setMouseEnabled(false);

	_state = kRun;
}

void Credits::run() {
	NancyInput input = g_nancy->_input->getInput();

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_state = kInit;
		g_nancy->_sound->stopSound(_creditsData->sound);
		g_nancy->setMouseEnabled(true);
		_fullTextSurface.free();
		g_nancy->setState(NancyState::kMainMenu);
		return;
	}

	Time currentTime = g_nancy->getTotalPlayTime();
	if (currentTime >= _nextUpdateTime) {
		_nextUpdateTime = currentTime + _creditsData->updateTime;

		Common::Rect newSrc = _textSurface.getScreenPosition();
		newSrc.moveTo(_textSurface._drawSurface.getOffsetFromOwner());
		newSrc.translate(0, _creditsData->pixelsToScroll);

		if (newSrc.bottom > _fullTextSurface.h) {
			newSrc.moveTo(Common::Point());
			if (_creditsData->textNames.size() > 1) {
				drawTextSurface(_currentTextImage == _creditsData->textNames.size() - 1 ? 0 : _currentTextImage + 1);
			}
		}

		_textSurface._drawSurface.create(_fullTextSurface, newSrc);
		_textSurface.setVisible(true);
	}
}

void Credits::drawTextSurface(uint id) {
	Graphics::ManagedSurface image;
	uint surfaceHeight = _textSurface.getBounds().height();
	g_nancy->_resource->loadImage(_creditsData->textNames[id], image);
	_fullTextSurface.create(image.w, image.h + (surfaceHeight * 2), g_nancy->_graphicsManager->getInputPixelFormat());
	_fullTextSurface.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
	_fullTextSurface.clear(_fullTextSurface.getTransparentColor());
	_fullTextSurface.blitFrom(image, Common::Point(0, surfaceHeight));

	if (image.hasPalette()) {
		uint8 palette[256 * 3];
		image.grabPalette(palette, 0, 256);
		_fullTextSurface.setPalette(palette, 0, 256);
	}

	_currentTextImage = id;
}

} // End of namespace State
} // End of namespace Nancy
