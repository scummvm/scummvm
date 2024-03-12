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

#include "engines/nancy/util.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/state/scene.h"

#include "engines/nancy/action/puzzle/tangrampuzzle.h"

namespace Nancy {
namespace Action {

TangramPuzzle::~TangramPuzzle() {
	delete[] _zBuffer;
}

void TangramPuzzle::init() {
	Common::Rect screenBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(screenBounds.width(), screenBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(screenBounds);

	g_nancy->_resource->loadImage(_tileImageName, _tileImage);
	g_nancy->_resource->loadImage(_maskImageName, _maskImage);

	_zBuffer = new byte[_drawSurface.w * _drawSurface.h];
	memset(_zBuffer, -1, _drawSurface.w * _drawSurface.h);

	_tiles.resize(_tileSrcs.size() + 1);

	// First, add the mask as its own tile for easier handling
	Tile *curTile = &_tiles[0];
	curTile->_srcImage.create(_maskImage, _maskImage.getBounds());
	curTile->_drawSurface.copyFrom(_tiles[0]._srcImage);
	curTile->_id = 0;
	curTile->drawMask();
	curTile->moveTo(_maskImage.getBounds());
	curTile->setTransparent(true);
	curTile->setVisible(true);
	drawToBuffer(*curTile);
	curTile->setZ(_z + 1);

	// Then, add the actual tiles
	for (uint i = 0; i < _tileSrcs.size(); ++i) {
		curTile = &_tiles[i + 1];
		curTile->_srcImage.create(_tileImage, _tileSrcs[i]);
		curTile->_drawSurface.copyFrom(curTile->_srcImage);
		curTile->_id = i + 1;
		curTile->moveTo(_tileDests[i]);
		curTile->setTransparent(true);
		curTile->setVisible(true);
		curTile->setZ(_z + curTile->_id + 1);
		curTile->drawMask();
		drawToBuffer(*curTile);

		// Draw the highlighted tile
		curTile->_highlightedSrcImage.copyFrom(curTile->_srcImage);

		Graphics::PixelFormat format = curTile->_highlightedSrcImage.format;
		for (int y = 0; y < curTile->_highlightedSrcImage.h; ++y) {
			uint16 *p = (uint16 *)curTile->_highlightedSrcImage.getBasePtr(0, y);
			for (int x = 0; x < curTile->_highlightedSrcImage.w; ++x) {
				if (*p != g_nancy->_graphics->getTransColor()) {
					// I'm not sure *3/2 is the exact formula but it's close enough
					byte r, g, b;
					format.colorToRGB(*p, r, g, b);
					r = (byte)((((uint16)r) * 3) >> 1);
					g = (byte)((((uint16)g) * 3) >> 1);
					b = (byte)((((uint16)b) * 3) >> 1);
					*p = (uint16)format.RGBToColor((byte)r, (byte)g, (byte)b);
				}

				++p;
			}
		}
	}

	registerGraphics();
}

void TangramPuzzle::registerGraphics() {
	for (Tile &tile : _tiles) {
		tile.registerGraphics();
	}

	RenderActionRecord::registerGraphics();
}

void TangramPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _tileImageName);
	readFilename(stream, _maskImageName);

	stream.skip(2); // Supposedly number of tiles, actually useless

	for (uint i = 0; i < 15; ++i) {
		Common::Rect src, dest;
		readRect(stream, src);
		readRect(stream, dest);

		if ((src.width() == 1 && src.height() == 1) || (dest.width() == 1 && dest.height() == 1)) {
			continue;
		}

		_tileSrcs.push_back(src);
		_tileDests.push_back(dest);
	}

	readRect(stream, _maskSolveBounds);

	_pickUpSound.readNormal(stream);
	_putDownSound.readNormal(stream);
	_rotateSound.readNormal(stream);

	_solveScene.readData(stream);
	_solveSound.readNormal(stream);

	_exitScene.readData(stream);
	readRect(stream, _exitHotspot);
}

void TangramPuzzle::execute() {
	switch (_state) {
	case kBegin :
		init();
		g_nancy->_sound->loadSound(_pickUpSound);
		g_nancy->_sound->loadSound(_putDownSound);
		g_nancy->_sound->loadSound(_rotateSound);

		NancySceneState.setNoHeldItem();

		_state = kRun;
		// fall through
	case kRun :
		if (_pickedUpTile == -1 && _shouldCheck) {
			for (int y = 0; y < _maskSolveBounds.height(); ++y) {
				byte *p = &_zBuffer[(y + _maskSolveBounds.top) * _drawSurface.w + _maskSolveBounds.left];
				for (int x = 0; x < _maskSolveBounds.width(); ++x) {
					if (*p == 0) {
						_shouldCheck = false;
						return;
					}
					++p;
				}
			}

			g_nancy->_sound->loadSound(_solveSound);
			g_nancy->_sound->playSound(_solveSound);
			_solved = true;
			_state = kActionTrigger;
		}

		break;
	case kActionTrigger :
		if (_solved) {
			if (g_nancy->_sound->isSoundPlaying(_solveSound)) {
				break;
			}

			_solveScene.execute();
		} else {
			_exitScene.execute();
		}

		g_nancy->_sound->stopSound(_solveSound);
		g_nancy->_sound->stopSound(_pickUpSound);
		g_nancy->_sound->stopSound(_putDownSound);
		g_nancy->_sound->stopSound(_rotateSound);

		finishExecution();

		break;
	}
}

void TangramPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	Common::Rect viewport = NancySceneState.getViewport().getScreenPosition();

	if (!viewport.contains(input.mousePos)) {
		return;
	}

	Common::Point mousePos = input.mousePos;
	mousePos.x -= viewport.left;
	mousePos.y -= viewport.top;

	viewport.moveTo(Common::Point(0, 0));

	if (_pickedUpTile == -1) {
		// Not holding a tile, check what's under the cursor
		byte idUnderMouse = _zBuffer[mousePos.y * _drawSurface.w + mousePos.x];

		if (idUnderMouse != 0 && idUnderMouse != (byte)-1) {
			// A tile is under the cursor
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				pickUpTile(idUnderMouse);
				g_nancy->_sound->playSound(_pickUpSound);
			} else if (input.input & NancyInput::kRightMouseButtonUp) {
				rotateTile(idUnderMouse);
				g_nancy->_sound->playSound(_rotateSound);
			}

			return;
		}

		// No tile under cursor, check exit hotspot
		if (_exitHotspot.contains(mousePos)) {
			g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_state = kActionTrigger;
			}

			return;
		}
	} else {
		// Currently holding a tile
		Tile &tileHolding = _tiles[_pickedUpTile];

		// Check if we need to place it back down
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			putDownTile(_pickedUpTile);
			g_nancy->_sound->playSound(_putDownSound);
			return;
		}

		tileHolding.handleInput(input);
		bool rotated = false;

		// Check if we need to rotate it
		if (input.input & NancyInput::kRightMouseButtonUp) {
			rotateTile(_pickedUpTile);
			g_nancy->_sound->playSound(_rotateSound);
			rotated = true;
		}

		if (!rotated) {
			// Check if we need to highlight, but only if we haven't rotated,
			// since rotateTile() already checks as well
			if (checkBuffer(tileHolding) != tileHolding._isHighlighted) {
				tileHolding.setHighlighted(!tileHolding._isHighlighted);
			}
		}
	}
}

void TangramPuzzle::drawToBuffer(const Tile &tile, Common::Rect subRect) {
	if (subRect.isEmpty()) {
		subRect = tile._screenPosition;
	}

	uint16 xDiff = subRect.left - tile._screenPosition.left;
	uint16 yDiff = subRect.top - tile._screenPosition.top;

	for (int y = 0; y < subRect.height(); ++y) {
		byte *src = &tile._mask[(y + yDiff) * tile._drawSurface.w + xDiff];
		byte *dest = &_zBuffer[(subRect.top + y) * _drawSurface.w + subRect.left];
		for (int x = 0; x < subRect.width(); ++x) {
			if (*src != (byte)-1) {
				*dest = *src;
			}

			++src;
			++dest;
		}
	}
}

void TangramPuzzle::pickUpTile(uint id) {
	assert(id < _tiles.size() && id != 0);

	Tile &tileToPickUp = _tiles[id];

	moveToTop(id);
	_pickedUpTile = id;
	redrawBuffer(tileToPickUp._screenPosition);
	tileToPickUp.pickUp();

	// Make sure we don't have a frame with the correct zOrder, but wrong position
	NancyInput input = g_nancy->_input->getInput();
	input.input = 0;
	handleInput(input);
}

void TangramPuzzle::putDownTile(uint id) {
	Tile &tile = _tiles[id];
	_pickedUpTile = -1;

	drawToBuffer(tile);
	tile.putDown();

	if (tile._isHighlighted) {
		tile.setHighlighted(false);
	}

	_shouldCheck = true;
}

void TangramPuzzle::rotateTile(uint id) {
	assert(id < _tiles.size() && id != 0);

	Tile &tileToRotate = _tiles[id];

	if (tileToRotate._rotation == 3) {
		tileToRotate._rotation = 0;
	} else {
		++tileToRotate._rotation;
	}

	moveToTop(id);

	Common::Rect oldPos = tileToRotate._screenPosition;

	if (_pickedUpTile != -1 && checkBuffer(tileToRotate)) {
		tileToRotate.setHighlighted(true);
	} else {
		tileToRotate.setHighlighted(false);
	}

	Common::Rect newPos = tileToRotate._drawSurface.getBounds();
	newPos.moveTo(oldPos.left + oldPos.width() / 2 - newPos.width() / 2, oldPos.top + oldPos.height() / 2 - newPos.height() / 2);
	tileToRotate.moveTo(newPos);
	_needsRedraw = true;

	tileToRotate.drawMask();
	tileToRotate._needsRedraw = true;

	if (_pickedUpTile == -1) {
		redrawBuffer(oldPos);
		drawToBuffer(tileToRotate);
		_shouldCheck = true;
	}
}

void TangramPuzzle::moveToTop(uint id) {
	for (uint i = 1; i < _tiles.size(); ++i) {
		Tile &tile = _tiles[i];
		if (tile._z > _tiles[id]._z) {
			tile.setZ(tile._z - 1);
			tile.registerGraphics();
		}
	}

	_tiles[id].setZ(_z + _tiles.size());
	_tiles[id].registerGraphics();
}

void TangramPuzzle::redrawBuffer(const Common::Rect &rect) {
	// Redraw the zBuffer for all intersecting pixels, except for the topmost tile
	for (int y = 0; y < rect.height(); ++y) {
		byte *dest = &_zBuffer[(y + rect.top) * _drawSurface.w + rect.left];
		memset(dest, -1, rect.width());
	}

	for (uint z = _z + 1; z < _z + _tiles.size(); ++z) {
		for (uint i = 0; i < _tiles.size() - 1; ++i) {
			Tile &tile = _tiles[i];
			if (tile._z == z) {
				if (tile._screenPosition.intersects(rect)) {
					drawToBuffer(tile, tile._screenPosition.findIntersectingRect(rect));
				}

				break;
			}
		}
	}
}

bool TangramPuzzle::checkBuffer(const Tile &tile) const {
	// Check if the provided tile has any pixel overlapping with a non-zero in the zBuffer
	// In other words, this checks if we're placing on a valid empty spot
	for (int y = 0; y < tile._drawSurface.h; ++y) {
		const byte *tilePtr = &tile._mask[y * tile._drawSurface.w];
		const byte *bufPtr = &_zBuffer[(y + tile._screenPosition.top) * _drawSurface.w + tile._screenPosition.left];
		for (int x = 0; x < tile._drawSurface.w; ++x) {
			if (*tilePtr != (byte)-1 && *bufPtr != 0) {
				return false;
			}

			++tilePtr;
			++bufPtr;
		}
	}

	return true;
}

TangramPuzzle::Tile::Tile() : _mask(nullptr), _id(0), _rotation(0), _isHighlighted(false) {}

TangramPuzzle::Tile::~Tile() {
	delete _mask;
}

void TangramPuzzle::Tile::drawMask() {
	if (!_mask) {
		_mask = new byte[_drawSurface.w * _drawSurface.h];
	}

	uint16 transColor = g_nancy->_graphics->getTransColor();
	for (int y = 0; y < _drawSurface.h; ++y) {
		uint16 *src = (uint16 *)_drawSurface.getBasePtr(0, y);
		for (int x = 0; x < _drawSurface.w; ++x) {
			if (*src == transColor) {
				_mask[y * _drawSurface.w + x] = -1;
			} else {
				_mask[y * _drawSurface.w + x] = _id;
			}
			++src;
		}
	}
}

void TangramPuzzle::Tile::setHighlighted(bool highlighted) {
	_isHighlighted = highlighted;
	GraphicsManager::rotateBlit(_isHighlighted ? _highlightedSrcImage : _srcImage,
								_drawSurface,
								_rotation);
	setTransparent(true);
	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
