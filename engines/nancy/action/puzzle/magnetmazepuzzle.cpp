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
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/action/puzzle/magnetmazepuzzle.h"

namespace Nancy {
namespace Action {

void MagnetMazePuzzle::readData(Common::SeekableReadStream &stream) {
	const int64 start = stream.pos();

	readFilename(stream, _boardImageName);
	readFilename(stream, _mazeImageName);

	for (int i = 0; i < kNumOverlays; ++i)
		readRect(stream, _overlaySrcRects[i]);
	for (int i = 0; i < kNumOverlays; ++i)
		readRect(stream, _overlayDestRects[i]);

	_overlayTransparent = (stream.readByte() != 0);
	_hideOverlays       = (stream.readByte() != 0);

	_requiredItem = stream.readSint16LE();
	stream.skip(1); // hit-test mode flag (unused in this port)

	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetSrcRects[i]);
	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetHomeRects[i]);
	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetTargetRects[i]);

	// CUIButton block at AR+0x1c7 (239 bytes). Internal layout: 15-byte
	// shared-art name, 4 × 16-byte state src rects at +0x46, hot rect at +0x96,
	// click sound at +0xbe. Only the hot rect is needed for hit-testing.
	stream.seek(start + 0x1c7 + 0x96);
	readRect(stream, _resetButtonHotspot);

	// Skip the decorative highlight RGB; read the wall RGB used for collision.
	stream.seek(start + 0x2b9);
	_wallColorR = stream.readByte();
	_wallColorG = stream.readByte();
	_wallColorB = stream.readByte();

	stream.seek(start + 0x2bc);

	_pickupSound.readNormal(stream);
	stream.seek(start + 0x372);
	_placeSound.readNormal(stream);
	stream.seek(start + 0x428);
	_resetSound.readNormal(stream);
	stream.seek(start + 0x459);
	_bumpSound.readNormal(stream);

	stream.seek(start + 0x48a);
	_winScene.readData(stream);
	stream.seek(start + 0x4a0);
	_winFlag.label = stream.readSint16LE();
	_winFlag.flag  = stream.readByte();
	_winDelaySec   = stream.readUint16LE();
	_winSound.readNormal(stream);

	stream.seek(start + 0x4d6);
	_cancelScene.readData(stream);
	stream.seek(start + 0x4ec);
	_cancelFlag.label = stream.readSint16LE();
	_cancelFlag.flag  = stream.readByte();

	readRect(stream, _exitHotspot);
}

void MagnetMazePuzzle::resetMagnets() {
	for (int i = 0; i < kNumMagnets; ++i) {
		_magnetPos[i]    = _magnetHomeRects[i];
		_magnetLocked[i] = false;
	}
	_heldMagnet = -1;
}

void MagnetMazePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_boardImageName, _boardImage);
	_boardImage.setTransparentColor(_drawSurface.getTransparentColor());

	g_nancy->_resource->loadImage(_mazeImageName, _mazeImage);

	MagnetMazePuzzleData *mmd = (MagnetMazePuzzleData *)NancySceneState.getPuzzleData(MagnetMazePuzzleData::getTag());
	bool restored = false;
	if (mmd && mmd->magnetState.size() >= kNumMagnets * 5) {
		const Common::Array<int16> &state = mmd->magnetState;
		for (int i = 0; i < kNumMagnets; ++i) {
			_magnetPos[i].left   = state[i * 5 + 0];
			_magnetPos[i].top    = state[i * 5 + 1];
			_magnetPos[i].right  = state[i * 5 + 2];
			_magnetPos[i].bottom = state[i * 5 + 3];
			_magnetLocked[i]     = (state[i * 5 + 4] != 0);
		}
		_heldMagnet = -1;
		restored = true;
	}
	if (!restored) {
		resetMagnets();
		persistState();
	}

	_isSolved = false;
	_subState = kPlaying;

	redraw();
}

void MagnetMazePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through

	case kRun:
		switch (_subState) {
		case kPlaying:
			break;
		case kWaitWinDelay:
			if (g_system->getMillis() >= _winDelayEndTime) {
				if (_winSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_winSound);
					g_nancy->_sound->playSound(_winSound);
					_subState = kWaitWinSound;
				} else {
					_subState = kExitToWin;
				}
			}
			break;
		case kWaitWinSound:
			if (!g_nancy->_sound->isSoundPlaying(_winSound)) {
				g_nancy->_sound->stopSound(_winSound);
				_subState = kExitToWin;
			}
			break;
		case kExitToWin:
		case kExitToCancel:
			_state = kActionTrigger;
			break;
		}
		break;

	case kActionTrigger:
		g_nancy->_sound->stopSound(_pickupSound);
		g_nancy->_sound->stopSound(_placeSound);
		g_nancy->_sound->stopSound(_resetSound);
		g_nancy->_sound->stopSound(_bumpSound);
		g_nancy->_sound->stopSound(_winSound);
		if (_subState == kExitToWin) {
			MagnetMazePuzzleData *mmd = (MagnetMazePuzzleData *)NancySceneState.getPuzzleData(MagnetMazePuzzleData::getTag());
			if (mmd)
				mmd->magnetState.clear();
			if (_winFlag.label != -1)
				NancySceneState.setEventFlag(_winFlag);
			if (_winScene.sceneID != kNoScene)
				NancySceneState.changeScene(_winScene);
		} else {
			if (_cancelFlag.label != -1)
				NancySceneState.setEventFlag(_cancelFlag);
			if (_cancelScene.sceneID != kNoScene)
				NancySceneState.changeScene(_cancelScene);
		}
		finishExecution();
		break;
	}
}

bool MagnetMazePuzzle::collidesAt(const Common::Rect &r) const {
	if (_mazeImage.empty() || _mazeImage.w == 0 || _mazeImage.h == 0)
		return false;

	const int mazeW = _mazeImage.w;
	const int mazeH = _mazeImage.h;
	int x0 = MAX<int>(0, r.left);
	int y0 = MAX<int>(0, r.top);
	int x1 = MIN<int>(mazeW, r.right);
	int y1 = MIN<int>(mazeH, r.bottom);

	// Anything that walks off the maze image counts as a wall.
	if (x0 >= x1 || y0 >= y1)
		return true;
	if (r.left < 0 || r.top < 0 || r.right > mazeW || r.bottom > mazeH)
		return true;

	// The chunk stores each wall channel as a 5-bit value (the original engine
	// masks with 0x1f and packs directly into a 16-bit display-format word).
	// Scale to 8-bit before RGBToColor so the resulting packed value matches
	// the actual maze-image pixels for the current pixel format.
	const byte r8 = (byte)((_wallColorR & 0x1f) << 3);
	const byte g8 = (byte)((_wallColorG & 0x1f) << 3);
	const byte b8 = (byte)((_wallColorB & 0x1f) << 3);
	const uint32 wall = _mazeImage.format.RGBToColor(r8, g8, b8);
	for (int y = y0; y < y1; ++y) {
		for (int x = x0; x < x1; ++x) {
			if (_mazeImage.getPixel(x, y) == wall)
				return true;
		}
	}
	return false;
}

void MagnetMazePuzzle::stepMagnetToward(Common::Rect &cur, const Common::Rect &target) const {
	int dx = target.left - cur.left;
	int dy = target.top - cur.top;

	// Single-pixel walk toward the target with axis-only fallback so a
	// diagonal blocked by a wall can still slide along it.
	while (dx != 0 || dy != 0) {
		const int sx = (dx > 0) ? 1 : (dx < 0 ? -1 : 0);
		const int sy = (dy > 0) ? 1 : (dy < 0 ? -1 : 0);

		if (sx != 0 && sy != 0) {
			Common::Rect cand(cur.left + sx, cur.top + sy, cur.right + sx, cur.bottom + sy);
			if (!collidesAt(cand)) {
				cur = cand;
				dx -= sx;
				dy -= sy;
				continue;
			}
		}
		if (sx != 0) {
			Common::Rect cand(cur.left + sx, cur.top, cur.right + sx, cur.bottom);
			if (!collidesAt(cand)) {
				cur = cand;
				dx -= sx;
				continue;
			}
		}
		if (sy != 0) {
			Common::Rect cand(cur.left, cur.top + sy, cur.right, cur.bottom + sy);
			if (!collidesAt(cand)) {
				cur = cand;
				dy -= sy;
				continue;
			}
		}
		break; // boxed in on every axis
	}
}

void MagnetMazePuzzle::snapMagnet(int idx) {
	const Common::Rect &tgt = _magnetTargetRects[idx];
	const Common::Rect &src = _magnetHomeRects[idx];
	int x = tgt.left + (tgt.width()  - src.width())  / 2;
	int y = tgt.top  + (tgt.height() - src.height()) / 2;
	_magnetPos[idx] = Common::Rect(x, y, x + src.width(), y + src.height());
	_magnetLocked[idx] = true;
}

void MagnetMazePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (_heldMagnet != -1 && _heldDrawPos != mouseVP) {
		_heldDrawPos = mouseVP;
		const Common::Rect &src = _magnetHomeRects[_heldMagnet];
		int w = src.width();
		int h = src.height();
		Common::Rect target(mouseVP.x - w / 2, mouseVP.y - h / 2,
		                    mouseVP.x - w / 2 + w, mouseVP.y - h / 2 + h);
		Common::Rect prev = _magnetPos[_heldMagnet];
		stepMagnetToward(_magnetPos[_heldMagnet], target);
		if (_magnetPos[_heldMagnet] != prev)
			redraw();
	}

	if (!_exitHotspot.isEmpty() && _exitHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(g_nancy->_cursor->_puzzleExitCursor);
		if (input.input & NancyInput::kLeftMouseButtonUp)
			_subState = kExitToCancel;
		return;
	}

	if (!_resetButtonHotspot.isEmpty() && _resetButtonHotspot.contains(mouseVP)) {
		g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			resetMagnets();
			if (_resetSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_resetSound);
				g_nancy->_sound->playSound(_resetSound);
			}
			persistState();
			redraw();
		}
		return;
	}

	if (_heldMagnet == -1) {
		for (int i = 0; i < kNumMagnets; ++i) {
			if (_magnetLocked[i])
				continue;
			if (!_magnetPos[i].contains(mouseVP))
				continue;
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				if (_requiredItem != -1 && NancySceneState.getHeldItem() != _requiredItem) {
					if (!_cantPlayed[i]) {
						NancySceneState.playItemCantSound(_requiredItem);
						_cantPlayed[i] = true;
					}
					return;
				}
				_heldMagnet = i;
				_heldDrawPos = mouseVP;
				if (_pickupSound.name != "NO SOUND") {
					g_nancy->_sound->loadSound(_pickupSound);
					g_nancy->_sound->playSound(_pickupSound);
				}
			}
			return;
		}
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
	if (!(input.input & NancyInput::kLeftMouseButtonUp))
		return;

	const Common::Rect &tgt = _magnetTargetRects[_heldMagnet];
	if (_magnetPos[_heldMagnet].intersects(tgt)) {
		snapMagnet(_heldMagnet);
		if (_placeSound.name != "NO SOUND") {
			g_nancy->_sound->loadSound(_placeSound);
			g_nancy->_sound->playSound(_placeSound);
		}
	}

	persistState();

	_heldMagnet = -1;
	checkSolved();
	redraw();
}

void MagnetMazePuzzle::persistState() {
	MagnetMazePuzzleData *mmd = (MagnetMazePuzzleData *)NancySceneState.getPuzzleData(MagnetMazePuzzleData::getTag());
	if (!mmd)
		return;
	Common::Array<int16> &state = mmd->magnetState;
	state.clear();
	for (int i = 0; i < kNumMagnets; ++i) {
		state.push_back((int16)_magnetPos[i].left);
		state.push_back((int16)_magnetPos[i].top);
		state.push_back((int16)_magnetPos[i].right);
		state.push_back((int16)_magnetPos[i].bottom);
		state.push_back(_magnetLocked[i] ? 1 : 0);
	}
}

void MagnetMazePuzzle::checkSolved() {
	for (int i = 0; i < kNumMagnets; ++i) {
		if (!_magnetLocked[i])
			return;
	}
	_isSolved = true;
	_subState = kWaitWinDelay;
	_winDelayEndTime = g_system->getMillis() + (uint32)_winDelaySec * 1000;
}

void MagnetMazePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	for (int i = 0; i < kNumMagnets; ++i) {
		const Common::Rect &src = _magnetSrcRects[i];
		const Common::Rect &dst = _magnetPos[i];
		if (src.isEmpty() || dst.isEmpty())
			continue;
		_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
	}

	if (!_hideOverlays) {
		for (int i = 0; i < kNumOverlays; ++i) {
			const Common::Rect &src = _overlaySrcRects[i];
			const Common::Rect &dst = _overlayDestRects[i];
			if (src.isEmpty() || dst.isEmpty())
				continue;
			_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
		}
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
