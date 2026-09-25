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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/action/puzzle/paintpuzzle.h"

#include "engines/nancy/state/scene.h"

namespace Nancy {
namespace Action {

void PaintPuzzle::readData(Common::SeekableReadStream &stream) {
	const bool isNancy15 = g_nancy->getGameType() >= kGameTypeNancy15;

	readFilename(stream, _imageName);		// 0x3d
	if (isNancy15) {
		readFilename(stream, _objectsImageName);
	}
	_hoverCursorType = stream.readUint16LE();	// 0x5e
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
		if (isNancy15) {
			readRect(stream, color.objectRect);
			color.flagLabel = stream.readSint16LE();
		}
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

	if (isNancy15) {
		_unorderedSolution = stream.readByte();
		_paintOnce = stream.readByte();
		// Input outside the canvas rect / hover cursor over regions. All shipped
		// puzzles accept input everywhere and use no region hover cursor.
		stream.skip(2);
	}

	_sounds[0].readData(stream);	// 0xa4
	_sounds[1].readData(stream);	// 0xfa

	// Shorter than the SceneChangeWithFlag::readData() formats: no vertical
	// offset or scene sound field
	_solveScene._sceneChange.sceneID = stream.readUint16LE();	// 0x1a6
	_solveScene._sceneChange.frameID = stream.readUint16LE();
	_solveScene._sceneChange.continueSceneSound = kContinueSceneSound;
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag = stream.readByte();

	_solveSound.readData(stream);			// 0x150

	readExitHotspots(stream, _exitHotspots);
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
	if (!_objectsImageName.empty()) {
		g_nancy->_resource->loadImage(_objectsImageName, _objectsImage);
	}

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
	_solveHandled = false;
	_takenExit = -1;

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
		if (_regions[i].rect.isEmpty() || i >= _regionImages.size() ||
				(_paintOnce && _regions[i].currentColor != -1)) {
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
	int c = region.currentColor - 1;
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
	// the black outline behind them rather than covering it. Overlapping regions
	// blend over the ones drawn before them.
	const PaintColor &color = _colors[c];
	const Common::Rect &dst = region.rect;

	for (int y = 0; y < img.h && dst.top + y < _drawSurface.h; ++y) {
		for (int x = 0; x < img.w && dst.left + x < _drawSurface.w; ++x) {
			byte a = shapeAlpha(img, x, y);
			if (a != 0) {
				blendPixel(dst.left + x, dst.top + y, a, color.r, color.g, color.b);
			}
		}
	}
}

// Composites a pixel over what is already drawn at (x,y).
void PaintPuzzle::blendPixel(int x, int y, byte a, byte r, byte g, byte b) {
	if (x < 0 || y < 0 || x >= _drawSurface.w || y >= _drawSurface.h) {
		return;
	}

	if (a != 255) {
		byte da, dr, dg, db;
		_drawSurface.format.colorToARGB(_drawSurface.getPixel(x, y), da, dr, dg, db);

		uint destAlpha = da * (255 - a) / 255;
		uint outAlpha = a + destAlpha;
		r = (r * a + dr * destAlpha) / outAlpha;
		g = (g * a + dg * destAlpha) / outAlpha;
		b = (b * a + db * destAlpha) / outAlpha;
		a = outAlpha;
	}

	_drawSurface.setPixel(x, y, _drawSurface.format.ARGBToColor(a, r, g, b));
}

// Draws a sprite from img, skipping transparent pixels and compositing
// anti-aliased edges over what is already drawn.
void PaintPuzzle::drawSprite(const Graphics::ManagedSurface &img, const Common::Rect &src, const Common::Point &dest) {
	if (img.w == 0 || src.isEmpty()) {
		return;
	}

	byte tr, tg, tb;
	g_nancy->_graphics->getInputPixelFormat().colorToRGB(g_nancy->_graphics->getTransColor(), tr, tg, tb);

	for (int y = 0; y < src.height() && src.top + y < img.h; ++y) {
		const int dy = dest.y + y;
		if (dy < 0 || dy >= _drawSurface.h) {
			continue;
		}

		for (int x = 0; x < src.width() && src.left + x < img.w; ++x) {
			const int dx = dest.x + x;
			if (dx < 0 || dx >= _drawSurface.w) {
				continue;
			}

			byte a, r, g, b;
			img.format.colorToARGB(img.getPixel(src.left + x, src.top + y), a, r, g, b);
			if (a == 0 || (r == tr && g == tg && b == tb)) {
				continue;
			}

			blendPixel(dx, dy, a, r, g, b);
		}
	}
}

// Draws each color's object at its swatch, except for the held color's.
void PaintPuzzle::drawObjects() {
	for (uint i = 0; i < _colors.size(); ++i) {
		if ((int)i != _heldColor) {
			drawSprite(_objectsImage, _colors[i].objectRect,
				Common::Point(_colors[i].swatchRect.left, _colors[i].swatchRect.top));
		}
	}
}

// Draws the held color's brush sprite at the cursor. The cursor is blanked while a
// color is held, so the brush is what the player sees moving.
void PaintPuzzle::drawBrush() {
	if (_heldColor < 0 || _heldColor >= (int)_colors.size()) {
		return;
	}

	// The sprite is centered on the cursor, then shifted by the offset.
	const Common::Rect &src = _colors[_heldColor].fillRect;
	drawSprite(_image, src, Common::Point(_brushPos.x - src.width() / 2 - _offset.x,
		_brushPos.y - src.height() / 2 - _offset.y));
}

void PaintPuzzle::redraw() {
	_drawSurface.clear(0);

	// Only painted regions are drawn on the overlay; the picture outline and
	// palette come from the scene background.
	for (uint i = 0; i < _regions.size(); ++i) {
		if (_regions[i].currentColor > 0) {
			drawRegion(i);
		}
	}

	drawObjects();
	drawBrush();

	_needsRedraw = true;
}

bool PaintPuzzle::isSolved() const {
	if (_regions.empty()) {
		return false;
	}

	if (!_unorderedSolution) {
		for (uint i = 0; i < _regions.size(); ++i) {
			if (_regions[i].currentColor != _regions[i].targetColor) {
				return false;
			}
		}
		return true;
	}

	// Every target must be matched by the color of a different region
	Common::Array<bool> used(_regions.size(), false);
	for (uint i = 0; i < _regions.size(); ++i) {
		bool found = false;
		for (uint j = 0; j < _regions.size(); ++j) {
			if (!used[j] && _regions[j].currentColor == _regions[i].targetColor) {
				used[j] = true;
				found = true;
				break;
			}
		}
		if (!found) {
			return false;
		}
	}
	return true;
}

// Each color's flag is set while that color is on at least one region.
void PaintPuzzle::updateColorFlags() {
	for (uint i = 0; i < _colors.size(); ++i) {
		if (_colors[i].flagLabel == -1) {
			continue;
		}

		bool used = false;
		for (uint j = 0; j < _regions.size(); ++j) {
			if (_regions[j].currentColor == (int16)(i + 1)) {
				used = true;
				break;
			}
		}
		NancySceneState.setEventFlag(_colors[i].flagLabel, used ? g_nancy->_true : g_nancy->_false);
	}
}

void PaintPuzzle::paintRegion(uint regionIndex, int colorIndex) {
	// Region colors are 1-based palette indices; 0 means unpainted
	_regions[regionIndex].currentColor = (int16)(colorIndex + 1);
	playSoundBlock(_sounds[1]);
	updateColorFlags();
	if (isSolved()) {
		_solved = true;
	}
	redraw();
}

void PaintPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	if (block.names.empty()) {
		return;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return;
	}

	SoundDescription desc;
	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

bool PaintPuzzle::isSoundBlockPlaying(const RandomSoundBlock &block) const {
	return !block.names.empty() && g_nancy->_sound->isSoundPlaying((uint16)block.channel);
}

void PaintPuzzle::handleInput(NancyInput &input) {
	// A solve without a scene change leaves the puzzle playable
	if (_state != kRun || (_solved && !_solveHandled)) {
		return;
	}

	const bool isNancy15 = g_nancy->getGameType() >= kGameTypeNancy15;

	// A held color turns the cursor into its brush sprite, which the puzzle draws itself.
	if (_heldColor >= 0) {
		Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
		Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
		Common::Point brushPos(vpPt.left, vpPt.top);

		if (brushPos != _brushPos) {
			_brushPos = brushPos;
			redraw();
		}

		g_nancy->_cursor->setCursorType(CursorManager::kNancy13Blank, true, false);
	}

	// Exit hotspots: leave the puzzle. In Nancy15 they only work with empty hands,
	// as one covers the canvas.
	for (uint i = 0; i < _exitHotspots.size() && !(isNancy15 && _heldColor >= 0); ++i) {
		if (!_exitHotspots[i].hotspot.isEmpty() &&
				NancySceneState.getViewport().convertViewportToScreen(_exitHotspots[i].hotspot).contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType((CursorManager::CursorType)_exitHotspots[i].cursorType, true);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_takenExit = (int)i;
			}
			input.eatMouseInput();
			return;
		}
	}

	// Over a paintable region with a color picked: paint it on click.
	int region = _heldColor >= 0 ? regionAtCursor(input.mousePos) : -1;
	if (region >= 0) {
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			paintRegion((uint)region, _heldColor);
		}
		input.eatMouseInput();
		return;
	}

	int color = colorSwatchAtCursor(input.mousePos);
	if (color >= 0) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)_hoverCursorType, true);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (isNancy15 && color == _heldColor) {
				dropColor();
			} else {
				_heldColor = color;
				playSoundBlock(_sounds[0]);
				redraw();
			}
		}
		input.eatMouseInput();
		return;
	}

	// Clicking outside the canvas puts the held color back.
	if (isNancy15 && _heldColor >= 0 && (input.input & NancyInput::kLeftMouseButtonUp) &&
			!NancySceneState.getViewport().convertViewportToScreen(_canvasRect).contains(input.mousePos)) {
		dropColor();
		input.eatMouseInput();
	}
}

void PaintPuzzle::dropColor() {
	_heldColor = -1;
	playSoundBlock(_sounds[0]);
	redraw();
}

void PaintPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		break;
	case kRun:
		if (_solved && !_solveHandled && _solveScene._sceneChange.sceneID == kNoScene) {
			// No scene change: set the flag and keep the puzzle on screen, so
			// the scene's own records can react to it.
			_solveHandled = true;
			playSoundBlock(_solveSound);
			NancySceneState.setEventFlag(_solveScene._flag);
		} else if (_takenExit >= 0 || (_solved && !_solveHandled)) {
			if (_solved) {
				playSoundBlock(_solveSound);
			}
			_state = kActionTrigger;
		}
		break;
	case kActionTrigger:
		// The solve sound gets to finish first
		if (_takenExit < 0 && isSoundBlockPlaying(_solveSound)) {
			break;
		}

		// The overlay may outlive the puzzle, so don't leave the brush in it
		if (_heldColor >= 0) {
			_heldColor = -1;
			redraw();
		}

		if (_takenExit >= 0) {
			NancySceneState.setEventFlag(_exitHotspots[_takenExit].flag);
			NancySceneState.changeScene(_exitHotspots[_takenExit].scene);
		} else {
			_solveScene.execute();
		}

		finishExecution();
		break;
	default:
		break;
	}
}

} // End of namespace Action
} // End of namespace Nancy
