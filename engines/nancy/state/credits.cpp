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
	Common::SeekableReadStream *cred = g_nancy->getBootChunkStream("CRED");
	cred->seek(0);

	Common::String imageName;
	readFilename(*cred, imageName);
	_background.init(imageName);

	readFilename(*cred, imageName);

	cred->skip(0x20); // Skip the src and dest rectangles
	readRect(*cred, _text._screenPosition);
	cred->skip(0x10);
	_updateTime = cred->readUint16LE();
	_pixelsToScroll = cred->readUint16LE();
	_sound.read(*cred, SoundDescription::kMenu);

	g_nancy->_resource->loadImage(imageName, _fullTextSurface);

	Common::Rect src = _text._screenPosition;
	src.moveTo(Common::Point());
	_fullTextSurface.setTransparentColor(g_nancy->_graphicsManager->getTransColor());
	_text._drawSurface.create(_fullTextSurface, src);
	_text.init();

	g_nancy->_sound->stopSound("MSND");

	g_nancy->_sound->loadSound(_sound);
	g_nancy->_sound->playSound(_sound);

	_background.registerGraphics();
	_text.registerGraphics();

	g_nancy->setMouseEnabled(false);

	_state = kRun;
}

void Credits::run() {
	NancyInput input = g_nancy->_input->getInput();

	if (input.input & NancyInput::kLeftMouseButtonDown) {
		_state = kInit;
		g_nancy->_sound->stopSound(_sound);
		g_nancy->setMouseEnabled(true);
		_fullTextSurface.free();
		g_nancy->setState(NancyState::kMainMenu);
		return;
	}

	Time currentTime = g_nancy->getTotalPlayTime();
	if (currentTime >= _nextUpdateTime) {
		_nextUpdateTime = currentTime + _updateTime;

		Common::Rect newSrc = _text._screenPosition;
		newSrc.moveTo(_text._drawSurface.getOffsetFromOwner());
		newSrc.translate(0, _pixelsToScroll);

		if (newSrc.bottom > _fullTextSurface.h) {
			newSrc.moveTo(Common::Point());
		}

		_text._drawSurface.create(_fullTextSurface, newSrc);
		_text._needsRedraw = true;
	}
}

} // End of namespace State
} // End of namespace Nancy
