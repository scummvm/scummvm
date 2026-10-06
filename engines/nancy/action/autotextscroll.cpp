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
#include "engines/nancy/graphics.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/util.h"

#include "engines/nancy/misc/hypertext.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/autotextscroll.h"

namespace Nancy {
namespace Action {

static const uint32 kMsPerTick = 20;
static const float kTicksPerSecond = 50.0f;

// Upper limit for the height of a single rendered entry
static const uint kMaxTextHeight = 8000;

class AutotextScrollRenderer : public Misc::HypertextParser {
public:
	void render(const Common::String &text, uint width, uint fontID, uint colorID, Graphics::ManagedSurface &image);
};

void AutotextScrollRenderer::render(const Common::String &text, uint width, uint fontID, uint colorID, Graphics::ManagedSurface &image) {
	_fullSurface.create(width, kMaxTextHeight, g_nancy->_graphics->getInputPixelFormat());
	_fullSurface.clear(g_nancy->_graphics->getTransColor());
	_defaultTextColor = colorID;

	addTextLine(text);
	drawAllText(_fullSurface.getBounds(), 0, fontID, fontID);

	uint height = CLIP<uint>(_drawnTextHeight, 1, kMaxTextHeight);
	image.create(width, height, _fullSurface.format);
	image.blitFrom(_fullSurface, Common::Rect(width, height), Common::Point());
	image.setTransparentColor(g_nancy->_graphics->getTransColor());
}

AutotextScroll::~AutotextScroll() {
	for (ScrollingText *text : _texts) {
		delete text;
	}
}

void AutotextScroll::readData(Common::SeekableReadStream &stream) {
	if (_hasDrawingOrder) {
		_z = stream.readSint32LE();
	}

	_startDepth = stream.readSint32LE();
	readRect(stream, _bounds);

	float interval = stream.readFloatLE();
	_intervalTicks = (int32)interval;
	_speed = stream.readFloatLE() / (interval * kTicksPerSecond);

	_fontID = stream.readUint32LE();
	_colorID = stream.readUint32LE();

	_keys.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Common::String &key : _keys) {
		readFilename(stream, key);
	}

	_doneFlag.label = stream.readSint16LE();
	_doneFlag.flag = stream.readByte();
}

// The text gets drawn over the viewport
void AutotextScroll::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	// The first entry starts right away
	_intervalCounter = _intervalTicks;
	_nextKey = 0;
	_lastTickTime = g_nancy->getTotalPlayTime();
}

void AutotextScroll::startText(const Common::String &key) {
	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	assert(autotext);

	ScrollingText *text = new ScrollingText();
	AutotextScrollRenderer renderer;
	renderer.render(autotext->texts[key], _bounds.width(), _fontID, _colorID, text->image);
	text->offset = _bounds.top - _startDepth;
	_texts.push_back(text);
}

// Returns true once the text is out of view
bool AutotextScroll::scrollText(ScrollingText &text, int32 ticks) {
	text.ticks += ticks;
	text.offset = (int32)((_bounds.top - _startDepth) - text.ticks * _speed);

	// The data rects include their bottom edge
	if (text.offset < _bounds.top - (_bounds.bottom - 1) && _speed >= 0) {
		return true;
	}

	return text.offset > text.image.h && _speed <= 0;
}

void AutotextScroll::update(int32 ticks) {
	_intervalCounter += ticks;
	if (_intervalCounter >= _intervalTicks && _nextKey < _keys.size()) {
		// Empty entries leave a gap
		if (!_keys[_nextKey].empty()) {
			startText(_keys[_nextKey]);
		}

		++_nextKey;
		_intervalCounter = 0;
	}

	for (uint i = 0; i < _texts.size(); ) {
		if (scrollText(*_texts[i], ticks)) {
			delete _texts[i];
			_texts.remove_at(i);
		} else {
			++i;
		}
	}
}

void AutotextScroll::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (const ScrollingText *text : _texts) {
		Common::Rect dest(_bounds.left, _bounds.top - text->offset, _bounds.left + text->image.w, _bounds.top - text->offset + text->image.h);
		Common::Rect visible = dest.findIntersectingRect(_bounds);
		if (visible.isEmpty()) {
			continue;
		}

		Common::Rect src = visible;
		src.translate(-dest.left, -dest.top);
		_drawSurface.blitFrom(text->image, src, visible.origin());
	}

	_needsRedraw = true;
}

void AutotextScroll::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		int32 ticks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += ticks * kMsPerTick;

		if (ticks > 0) {
			update(ticks);
		}

		if (_nextKey >= _keys.size() && _texts.empty()) {
			_state = kActionTrigger;
		}

		redraw();
		break;
	}
	case kActionTrigger:
		NancySceneState.setEventFlag(_doneFlag);
		finishExecution();
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
