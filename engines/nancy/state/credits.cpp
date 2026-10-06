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

#include "engines/nancy/misc/hypertext.h"

#include "engines/nancy/state/credits.h"

#include "common/events.h"
#include "common/config-manager.h"

namespace Common {
DECLARE_SINGLETON(Nancy::State::Credits);
}

namespace Nancy {
namespace State {

// Renders the Nancy 14+ credits, which are stored as hypertext in the AUTOTEXT chunk
class CreditsTextRenderer : public Misc::HypertextParser {
public:
	void render(const Common::String &text, uint width, Graphics::ManagedSurface &image);
};

void CreditsTextRenderer::render(const Common::String &text, uint width, Graphics::ManagedSurface &image) {
	// Default font and maximum surface height of the credits text
	const uint fontID = 2;
	const uint maxHeight = 5020;

	_fullSurface.create(width, maxHeight, g_nancy->_graphics->getInputPixelFormat());
	_fullSurface.clear(g_nancy->_graphics->getTransColor());

	const Font *font = g_nancy->_graphics->getFont(fontID);
	assert(font);
	uint margin = (font->getFontHeight() + 1) / 2 + 1;

	Common::Rect textBounds = _fullSurface.getBounds();
	textBounds.grow(-(int)margin);

	addTextLine(text);
	drawAllText(textBounds, 0, fontID, fontID);

	uint height = MIN<uint>(_drawnTextHeight + textBounds.top, maxHeight);
	image.create(width, height, _fullSurface.format);
	image.blitFrom(_fullSurface, Common::Rect(width, height), Common::Point());
}

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

void Credits::onStateEnter(const NancyState::NancyState prevState) {
	// Handle returning from the GMM
	if (prevState == NancyState::kPause) {
		g_nancy->_sound->pauseSound(_creditsData->sound, false);
	}
}

bool Credits::onStateExit(const NancyState::NancyState nextState) {
	// Handle the GMM being called
	if (nextState == NancyState::kPause) {
		g_nancy->_sound->pauseSound(_creditsData->sound, true);

		return false;
	} else {
		return true;
	}
}

void Credits::init() {
	_creditsData = GetEngineData(CRED);
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

		if (!ConfMan.hasKey("original_menus") || ConfMan.getBool("original_menus")) {
			g_nancy->setState(NancyState::kMainMenu);
			return;
		}

		Common::Event ev;
		ev.type = Common::EVENT_RETURN_TO_LAUNCHER;
		g_system->getEventManager()->pushEvent(ev);

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
	if (!_creditsData->textKey.empty()) {
		const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
		assert(autotext);

		CreditsTextRenderer renderer;
		renderer.render(autotext->texts[_creditsData->textKey], _textSurface.getBounds().width(), image);
	} else {
		g_nancy->_resource->loadImage(_creditsData->textNames[id], image);
	}

	_fullTextSurface.create(image.w, image.h + (surfaceHeight * 2), g_nancy->_graphics->getInputPixelFormat());
	_fullTextSurface.setTransparentColor(g_nancy->_graphics->getTransColor());
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
