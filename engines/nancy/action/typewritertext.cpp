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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/util.h"
#include "engines/nancy/misc/hypertext.h"
#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/typewritertext.h"

namespace Nancy {
namespace Action {

static const uint32 kMsPerTick = 20;
static const double kTicksPerSecond = 50.0;

class TypewriterTextRenderer : public Misc::HypertextParser {
public:
	void render(const Common::String &text, uint fontID, uint colorID, Graphics::ManagedSurface &image);
};

void TypewriterTextRenderer::render(const Common::String &text, uint fontID, uint colorID, Graphics::ManagedSurface &image) {
	_fullSurface.create(image.w, image.h, image.format);
	_fullSurface.clear(g_nancy->_graphics->getTransColor());
	_defaultTextColor = colorID;

	addTextLine(text);
	drawAllText(_fullSurface.getBounds(), 0, fontID, fontID);

	image.blitFrom(_fullSurface, Common::Point());
}

void TypewriterText::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _textKey);
	readExclusiveRect(stream, _bounds);
	_charactersPerSecond = stream.readDoubleLE();
	_fontID = stream.readUint32LE();
	_colorID = stream.readUint32LE();
	_keySound.readData(stream);

	_doneFlag.label = stream.readSint16LE();
	_doneFlag.flag = stream.readByte();
}

void TypewriterText::init() {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);
	_text = autotext->texts.getValOrDefault(_textKey);

	_drawSurface.create(_bounds.width(), _bounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(_bounds);

	_lastTickTime = g_nancy->getTotalPlayTime();
}

// A tag gets written in one go, and doesn't count as a character
uint TypewriterText::typeCharacters(uint numCharacters) {
	uint numAdded = 0;
	bool isInsideTag = false;

	while ((numAdded < numCharacters || isInsideTag) && _numTypedCharacters < _text.size()) {
		char c = _text[_numTypedCharacters];
		if (c == '<') {
			isInsideTag = true;
		} else if (c == '>') {
			isInsideTag = false;
		}

		++_numTypedCharacters;
		++numAdded;
	}

	return numAdded;
}

void TypewriterText::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	TypewriterTextRenderer renderer;
	renderer.render(_text.substr(0, _numTypedCharacters), _fontID, _colorID, _drawSurface);

	_needsRedraw = true;
}

void TypewriterText::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		uint32 ticks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += ticks * kMsPerTick;
		_pendingTicks += ticks;

		// The time that passed only gets used up once it is worth a character
		uint numCharacters = (uint)(_pendingTicks * _charactersPerSecond / kTicksPerSecond);
		if (numCharacters > 0) {
			_pendingTicks = 0;

			if (typeCharacters(numCharacters) > 0) {
				_keySound.play();
				redraw();
			}

			if (_numTypedCharacters >= _text.size()) {
				_state = kActionTrigger;
			}
		}

		break;
	}
	case kActionTrigger:
		// The text stays up for as long as the scene does
		NancySceneState.setEventFlag(_doneFlag);
		_isDone = true;
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
