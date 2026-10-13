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

// Speed of a held piece, in pixels per millisecond
static const float kMagnetSpeed = 1.0f / 3.0f;

void MagnetMazePuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _boardImageName);
	readFilename(stream, _mazeImageName);

	for (int i = 0; i < kNumOverlays; ++i)
		readRect(stream, _overlaySrcRects[i]);
	for (int i = 0; i < kNumOverlays; ++i)
		readRect(stream, _overlayDestRects[i]);

	_overlayTransparent = (stream.readByte() != 0);
	_hideOverlays       = (stream.readByte() != 0);

	_requiredItem = stream.readSint16LE();
	_hoverUsesCursorCenter = (stream.readByte() != 0);

	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetSrcRects[i]);
	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetHomeRects[i]);
	for (int i = 0; i < kNumMagnets; ++i)
		readRect(stream, _magnetTargetRects[i]);

	readUIButton(stream, _resetButton);

	// Skip the decorative highlight RGB; read the wall RGB used for collision.
	stream.skip(3);
	_wallColorR = stream.readByte();
	_wallColorG = stream.readByte();
	_wallColorB = stream.readByte();

	char textBuf[100];

	_pickupSound.readNormal(stream);
	readFilename(stream, _pickupTextKey);
	stream.read(textBuf, sizeof(textBuf));
	textBuf[sizeof(textBuf) - 1] = '\0';
	_pickupText = textBuf;

	_dropSound.readNormal(stream);
	readFilename(stream, _dropTextKey);
	stream.read(textBuf, sizeof(textBuf));
	textBuf[sizeof(textBuf) - 1] = '\0';
	_dropText = textBuf;

	_moveSound.readNormal(stream);
	_lockSound.readNormal(stream);

	_solveScene._sceneChange.readData(stream);
	stream.skip(2);
	_solveScene._flag.label = stream.readSint16LE();
	_solveScene._flag.flag  = stream.readByte();
	_solveSoundDelay   = stream.readUint16LE();
	_solveSound.readNormal(stream);

	_exitScene._sceneChange.readData(stream);
	stream.skip(2);
	_exitScene._flag.label = stream.readSint16LE();
	_exitScene._flag.flag  = stream.readByte();

	readRect(stream, _exitHotspot);
}

// Only the pieces that are not locked in place go back home
void MagnetMazePuzzle::resetMagnets() {
	for (int i = 0; i < kNumMagnets; ++i) {
		if (!_magnetLocked[i])
			_magnetPos[i] = _magnetHomeRects[i];
	}
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

	g_nancy->_resource->loadImage(_resetButton.primaryImageName, _resetButtonImage);
	_resetButtonImage.setTransparentColor(_drawSurface.getTransparentColor());

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
		for (int i = 0; i < kNumMagnets; ++i) {
			_magnetPos[i]    = _magnetHomeRects[i];
			_magnetLocked[i] = false;
		}
		_heldMagnet = -1;
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
			// The reset takes effect once the button's press is over
			if (_resetPressed && g_system->getMillis() > _resetPressEndTime &&
					!g_nancy->_sound->isSoundPlaying(_resetButton.clickSound)) {
				_resetPressed = false;
				resetMagnets();
				persistState();
				redraw();
			}

			lockMagnetsOnTargets();
			break;
		case kWaitWinDelay:
			if (g_system->getMillis() >= _winDelayEndTime) {
				if (hasSolveSound()) {
					playSolveSound();
					_subState = kWaitWinSound;
				} else {
					_subState = kExitToWin;
				}
			}
			break;
		case kWaitWinSound:
			if (!isSolveSoundPlaying()) {
				g_nancy->_sound->stopSound(_solveSound);
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
		g_nancy->_sound->stopSound(_dropSound);
		g_nancy->_sound->stopSound(_moveSound);
		g_nancy->_sound->stopSound(_lockSound);
		g_nancy->_sound->stopSound(_resetButton.clickSound);
		g_nancy->_sound->stopSound(_solveSound);
		if (_subState == kExitToWin) {
			MagnetMazePuzzleData *mmd = (MagnetMazePuzzleData *)NancySceneState.getPuzzleData(MagnetMazePuzzleData::getTag());
			if (mmd)
				mmd->magnetState.clear();
			_solveScene.execute();
		} else {
			_exitScene.execute();
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

// A loose piece that touches its target zone snaps to the middle of it and locks
void MagnetMazePuzzle::lockMagnetsOnTargets() {
	bool changed = false;

	for (int i = 0; i < kNumMagnets; ++i) {
		if (_magnetLocked[i] || !_magnetPos[i].intersects(_magnetTargetRects[i]))
			continue;

		const Common::Rect &tgt = _magnetTargetRects[i];
		const Common::Rect &src = _magnetHomeRects[i];
		int x = tgt.left + tgt.width() / 2 - src.width() / 2;
		int y = tgt.top + tgt.height() / 2 - src.height() / 2;
		_magnetPos[i] = Common::Rect(x, y, x + src.width(), y + src.height());
		_magnetLocked[i] = true;
		_magnetGrabbed[i] = false;

		if (_heldMagnet == i)
			playPuzzleSound(_lockSound);
		_heldMagnet = -1;
		changed = true;
	}

	if (!changed)
		return;

	persistState();
	redraw();

	for (int i = 0; i < kNumMagnets; ++i) {
		if (!_magnetLocked[i])
			return;
	}

	_isSolved = true;
	_subState = kWaitWinDelay;
	_winDelayEndTime = g_system->getMillis() + (uint32)_solveSoundDelay * 1000;
}

void MagnetMazePuzzle::dropMagnets() {
	for (int i = 0; i < kNumMagnets; ++i) {
		if (_magnetLocked[i] || !_magnetGrabbed[i])
			continue;

		_magnetGrabbed[i] = false;
		_heldMagnet = -1;
		_wasDropped = true;

		playPuzzleSound(_dropSound);
		showSubtitle(resolveSubtitleText(_dropTextKey, _dropText));
		persistState();
		redraw();
	}
}

// The held piece heads straight for the center of the cursor image, at a
// limited speed, and stops at the first wall in its way; it does not slide
// along walls. Returns false when it could not get any closer.
bool MagnetMazePuzzle::moveHeldMagnet(const Common::Point &cursorCenter) {
	Common::Rect &pos = _magnetPos[_heldMagnet];
	const int w = pos.width();
	const int h = pos.height();
	float dx = cursorCenter.x - w / 2 - pos.left;
	float dy = cursorCenter.y - h / 2 - pos.top;

	const uint32 now = g_system->getMillis();
	const float dist = sqrt(dx * dx + dy * dy);
	if (dist < 1.0f) {
		_lastMoveTime = now;
		return false;
	}

	// Wait for the next frame when not even a pixel's worth of time has passed
	const float maxDist = MIN<float>(dist, kMagnetSpeed * (now - _lastMoveTime));
	if (maxDist < 1.0f)
		return false;

	_lastMoveTime = now;
	dx /= dist;
	dy /= dist;

	Common::Rect reached = pos;
	float x = pos.left;
	float y = pos.top;
	for (int i = 0; i < (int)maxDist; ++i) {
		x += dx;
		y += dy;
		Common::Rect next((int)x, (int)y, (int)x + w, (int)y + h);
		if (collidesAt(next))
			break;
		reached = next;
	}

	if (reached == pos)
		return false;

	pos = reached;
	if (!g_nancy->_sound->isSoundPlaying(_moveSound))
		playPuzzleSound(_moveSound);
	redraw();
	return true;
}

bool MagnetMazePuzzle::isMagnetHovered(int idx, const Common::Rect &cursorRect, const Common::Point &cursorCenter) const {
	if (_hoverUsesCursorCenter)
		return _magnetPos[idx].contains(cursorCenter);

	return _magnetPos[idx].intersects(cursorRect);
}

// The sounds share their channels with the UI's (e.g. the taskbar's click), so
// they are loaded right before they play
void MagnetMazePuzzle::playPuzzleSound(SoundDescription &sound) {
	if (sound.name == "NO SOUND")
		return;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

void MagnetMazePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _subState != kPlaying)
		return;

	Common::Rect vpScreen = NancySceneState.getViewport().getScreenPosition();
	Common::Point mouseVP = input.mousePos - Common::Point(vpScreen.left, vpScreen.top);

	if (_resetButton.destRect.contains(mouseVP)) {
		if (_resetButton.hoverCursorFlag)
			g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

		if ((input.input & NancyInput::kLeftMouseButtonUp) && !_resetPressed) {
			_resetPressed = true;
			_resetPressEndTime = g_system->getMillis() + _resetButton.pressedDuration;
			if (_resetButton.clickSound.name != "NO SOUND") {
				g_nancy->_sound->loadSound(_resetButton.clickSound);
				g_nancy->_sound->playSound(_resetButton.clickSound);
			}
			redraw();
		}
		return;
	}

	if (hoverExitHotspot(input)) {
		if (input.input & NancyInput::kLeftMouseButtonUp)
			_subState = kExitToCancel;
		return;
	}

	// The pieces react to the cursor's image, not to the mouse position itself
	Common::Rect cursorRect = g_nancy->_cursor->getCurrentCursorBounds();
	cursorRect.moveTo(mouseVP - g_nancy->_cursor->getCurrentCursorHotspot());
	const Common::Point cursorCenter(cursorRect.left + (cursorRect.width() - 1) / 2,
	                                 cursorRect.top + (cursorRect.height() - 1) / 2);

	// A piece that is stuck is let go of, but is taken again right below if the
	// cursor is still on it
	if (_heldMagnet != -1 && !moveHeldMagnet(cursorCenter))
		_heldMagnet = -1;

	int hovered = -1;
	for (int i = 0; i < kNumMagnets; ++i) {
		if (!_magnetLocked[i] && isMagnetHovered(i, cursorRect, cursorCenter)) {
			hovered = i;
			break;
		}
	}

	if (hovered == -1) {
		dropMagnets();
		return;
	}

	if (_requiredItem != -1 && NancySceneState.getHeldItem() != _requiredItem) {
		if (!_cantPlayed[hovered]) {
			NancySceneState.playItemCantSound(_requiredItem);
			_cantPlayed[hovered] = true;
		}
		return;
	}

	g_nancy->_cursor->setCursorType(CursorManager::kHotspot);

	if (_heldMagnet == -1) {
		_heldMagnet = hovered;
		_magnetGrabbed[hovered] = true;
		_lastMoveTime = g_system->getMillis();
		if (_wasDropped) {
			_wasDropped = false;
			playPuzzleSound(_pickupSound);
			showSubtitle(resolveSubtitleText(_pickupTextKey, _pickupText));
		}
		redraw();
	}
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

void MagnetMazePuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	// The held piece goes on top of the others
	for (int pass = 0; pass < 2; ++pass) {
		for (int i = 0; i < kNumMagnets; ++i) {
			if ((i == _heldMagnet) != (pass == 1))
				continue;
			const Common::Rect &src = _magnetSrcRects[i];
			const Common::Rect &dst = _magnetPos[i];
			if (src.isEmpty() || dst.isEmpty())
				continue;
			_drawSurface.blitFrom(_boardImage, src, Common::Point(dst.left, dst.top));
		}
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

	if (_resetPressed) {
		const Common::Rect &src = _resetButton.sourceRects[kUIButtonPressed];
		if (!src.isEmpty())
			_drawSurface.blitFrom(_resetButtonImage, src, Common::Point(_resetButton.destRect.left, _resetButton.destRect.top));
	}

	_needsRedraw = true;
}

} // End of namespace Action
} // End of namespace Nancy
