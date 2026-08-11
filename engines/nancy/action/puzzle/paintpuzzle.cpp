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
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/paintpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PaintPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);		// 0x3d
	_field5e = stream.readSint16LE();		// 0x5e
	_offset.x = stream.readSint32LE();		// 0x60
	_offset.y = stream.readSint32LE();		// 0x64
	readRect(stream, _canvasRect);			// 0x68

	int16 numColors = stream.readSint16LE();
	_colors.resize(numColors);				// 0x78
	for (int16 i = 0; i < numColors; ++i) {
		PaintColor &color = _colors[i];
		color.r = stream.readByte();
		color.g = stream.readByte();
		color.b = stream.readByte();
		readRect(stream, color.swatchRect);
		readRect(stream, color.fillRect);
	}

	int16 numRegions = stream.readSint16LE();
	_regions.resize(numRegions);			// 0x94
	for (int16 i = 0; i < numRegions; ++i) {
		PaintRegion &region = _regions[i];
		readFilename(stream, region.name);
		readRect(stream, region.rect);
		region.currentColor = stream.readSint16LE();
		region.targetColor = stream.readSint16LE();
	}

	_sounds[0].readData(stream);	// 0xa4
	_sounds[1].readData(stream);	// 0xfa

	_field1a6 = stream.readSint16LE();		// 0x1a6
	_outcome.field0 = stream.readSint16LE();	// 0x1a8
	_outcome.sceneID = stream.readSint16LE();
	_outcome.flag = stream.readByte();

	_sounds[2].readData(stream);

	// Trailing count-prefixed array of 23-byte give-up hotspots
	// {Rect, uint16 cursorType, uint16 sceneID, int16 flagLabel, byte flagValue}.
	// The exit always jumps to the scene's first frame.
	int16 numExitZones = stream.readSint16LE();
	for (int16 i = 0; i < numExitZones; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 flagLabel = stream.readSint16LE();
		byte flagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			_exitScene.frameID = 0;
			_exitFlag.label = flagLabel;
			_exitFlag.flag = flagValue;
		}
	}
}

void PaintPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	// Use the BGRA32 transparent format so the recolored region shapes keep their
	// source alpha and their anti-aliased edges blend with the black line-art
	// behind them instead of covering it.
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.clear(0);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	_regionImages.resize(_regions.size());
	for (uint i = 0; i < _regions.size(); ++i) {
		if (!_regions[i].name.empty()) {
			g_nancy->_resource->loadImage(_regions[i].name, _regionImages[i]);
		}
	}

	_heldColor = -1;
	_hoverRegion = -1;
	_hoverColor = -1;
	_solved = false;
	_outcomeApplied = false;

	redraw();
}

int PaintPuzzle::colorSwatchAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _colors.size(); ++i) {
		if (_colors[i].swatchRect.isEmpty()) {
			continue;
		}
		if (NancySceneState.getViewport().convertViewportToScreen(_colors[i].swatchRect).contains(mousePos)) {
			return (int)i;
		}
	}
	return -1;
}

byte PaintPuzzle::shapeAlpha(const Graphics::ManagedSurface &img, int x, int y) const {
	if (img.w == 0 || x < 0 || y < 0 || x >= img.w || y >= img.h) {
		return 0;
	}
	uint32 p = img.getPixel(x, y);
	if (img.format.aBits() != 0) {
		byte a, r, g, b;
		img.format.colorToARGB(p, a, r, g, b);
		return a;
	}
	return p != img.getTransparentColor() ? 255 : 0;
}

int PaintPuzzle::regionAtCursor(const Common::Point &mousePos) const {
	for (uint i = 0; i < _regions.size(); ++i) {
		if (_regions[i].rect.isEmpty() || i >= _regionImages.size()) {
			continue;
		}
		Common::Rect screenRect = NancySceneState.getViewport().convertViewportToScreen(_regions[i].rect);
		if (!screenRect.contains(mousePos)) {
			continue;
		}
		// Hit-test the region's actual shape, not just its bounding rect, so
		// overlapping bounding boxes don't register clicks for the wrong region.
		if (shapeAlpha(_regionImages[i], mousePos.x - screenRect.left, mousePos.y - screenRect.top) != 0) {
			return (int)i;
		}
	}
	return -1;
}

// Draws a painted region: its overlay shape recolored to the flat fill color.
void PaintPuzzle::drawRegion(uint regionIndex) {
	const PaintRegion &region = _regions[regionIndex];
	int c = region.currentColor;
	if (c < 0 || c >= (int)_colors.size() || regionIndex >= _regionImages.size()) {
		return;
	}

	const Graphics::ManagedSurface &img = _regionImages[regionIndex];
	if (img.w == 0 || img.h == 0) {
		return;
	}

	// The region image is a shape mask (alpha channel, or color key for
	// alpha-less images). Recolor its shape pixels to the chosen flat color while
	// keeping each pixel's original alpha, so the anti-aliased edges blend with
	// the black outline behind them rather than covering it.
	const PaintColor &color = _colors[c];
	const Common::Rect &dst = region.rect;

	for (int y = 0; y < img.h && dst.top + y < _drawSurface.h; ++y) {
		for (int x = 0; x < img.w && dst.left + x < _drawSurface.w; ++x) {
			byte a = shapeAlpha(img, x, y);
			if (a != 0) {
				_drawSurface.setPixel(dst.left + x, dst.top + y,
					_drawSurface.format.ARGBToColor(a, color.r, color.g, color.b));
			}
		}
	}
}

void PaintPuzzle::redraw() {
	_drawSurface.clear(0);

	// Only painted regions are drawn on the overlay; the picture outline and
	// palette come from the scene background.
	for (uint i = 0; i < _regions.size(); ++i) {
		if (_regions[i].currentColor >= 0) {
			drawRegion(i);
		}
	}

	_needsRedraw = true;
}

bool PaintPuzzle::isSolved() const {
	if (_regions.empty()) {
		return false;
	}
	for (uint i = 0; i < _regions.size(); ++i) {
		if (_regions[i].currentColor != _regions[i].targetColor) {
			return false;
		}
	}
	return true;
}

void PaintPuzzle::paintRegion(uint regionIndex, int colorIndex) {
	_regions[regionIndex].currentColor = (int16)colorIndex;
	if (isSolved()) {
		_solved = true;
	}
	redraw();
}

void PaintPuzzle::applyOutcome(const SceneOutcome &outcome) {
	SceneChangeDescription desc;
	desc.sceneID = outcome.sceneID;
	NancySceneState.changeScene(desc);
	NancySceneState.setEventFlag(outcome.field0, outcome.flag);
}

void PaintPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _solved) {
		return;
	}

	// Give-up hotspot: leave the puzzle.
	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_exitRequested = true;
		}
		input.eatMouseInput();
		return;
	}

	int color = colorSwatchAtCursor(input.mousePos);
	if (color >= 0) {
		// Over a color swatch: show the blue puzzle-hotspot hand and pick the
		// color on click.
		g_nancy->_cursor->setCursorType(CursorManager::kPuzzleArrow);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_heldColor = color;
		}
		input.eatMouseInput();
		return;
	}

	// Over a paintable region with a color picked: paint it on click. The
	// cursor is left as the held paintbrush item.
	int region = regionAtCursor(input.mousePos);
	if (region >= 0 && _heldColor >= 0) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			paintRegion((uint)region, _heldColor);
		}
		input.eatMouseInput();
	}
}

void PaintPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		if (_exitRequested) {
			NancySceneState.setEventFlag(_exitFlag);
			NancySceneState.changeScene(_exitScene);
			break;
		}
		if (_solved && !_outcomeApplied) {
			_outcomeApplied = true;
			applyOutcome(_outcome);
		}
		break;
	default:
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
