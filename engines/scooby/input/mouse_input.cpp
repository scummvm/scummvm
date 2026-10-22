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

// MANAGED-ONLY MOUSE SUPPORT (see mouse_input.h).

#include "mouse_input.h"

#include "common/util.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

#include "scooby/assets/rom.h"
#include "scooby/graphics/tile_scene.h"
#include "scooby/runtime/runtime_state.h"

namespace Scooby {

namespace {

// The room cursor sprite: four 4bpp tiles in Genesis sprite (column-major) order, the same bytes
// RoomSceneLoader uploads for CursorSpriteTileAttributes (RoomSceneLoader::kCursorSpriteTilesOffset).
const int kCrosshairTilesOffset = 0x31AFA;
const int kCrosshairTileBytes = 32;

// Host cursor colours for the crosshair: 0 transparent, 1 outline (sprite colour 1), 2 fill (colour 15).
const byte kCrosshairPalette[3 * 3] = {0, 0, 0, 0, 0, 0, 255, 255, 255};

const uint8 kInteractionPendingMask = 0x80;

} // namespace

MouseInput::MouseInput()
	: _scene(nullptr), _mode(Mode::Generic), _active(false), _moved(false), _leftClick(false),
	  _leftClickPoll(0), _rightClick(false), _rightClickPoll(0), _wheel(0), _wheelPoll(0), _pollCount(0),
	  _cursorStyle(CursorStyle::Hidden), _pulseCount(0), _pulseSamplesLeft(0),
	  _pulseReleasePending(false) {
	memset(_crosshairPixels, 0, sizeof(_crosshairPixels));
	for (int i = 0; i < kMaxQueuedPulses; i++) {
		_pulseQueue[i] = ControllerButtons::None;
	}
}

void MouseInput::attach(const ScoobyDooRom &rom, const TileScene &scene) {
	_scene = &scene;
	_room.attach(rom);
	Span<const uint8> tiles = rom.readBytes(kCrosshairTilesOffset, 4 * kCrosshairTileBytes);
	for (int y = 0; y < kCrosshairSize; y++) {
		for (int x = 0; x < kCrosshairSize; x++) {
			int tile = (x / 8) * 2 + (y / 8);
			uint8 packed = tiles[static_cast<size_t>(tile * kCrosshairTileBytes + (y % 8) * 4 + (x % 8) / 2)];
			uint8 colour = (x % 2) == 0 ? static_cast<uint8>(packed >> 4) : static_cast<uint8>(packed & 0x0F);
			_crosshairPixels[y * kCrosshairSize + x] = colour == 0 ? 0 : (colour == 1 ? 1 : 2);
		}
	}
}

void MouseInput::handleEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		if (event.mouse != _position) {
			_position = event.mouse;
			_moved = true;
			_active = true;
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		_position = event.mouse;
		_leftClick = true;
		_leftClickPosition = toContent(event.mouse);
		_leftClickPoll = _pollCount;
		_active = true;
		break;
	case Common::EVENT_RBUTTONDOWN:
		_position = event.mouse;
		_rightClick = true;
		_rightClickPoll = _pollCount;
		_active = true;
		break;
	case Common::EVENT_WHEELUP:
		_wheel--;
		_wheelPoll = _pollCount;
		_active = true;
		break;
	case Common::EVENT_WHEELDOWN:
		_wheel++;
		_wheelPoll = _pollCount;
		_active = true;
		break;
	default:
		break;
	}
}

void MouseInput::handleKeyboardOrPadInput() {
	_active = false;
	_moved = false;
	_leftClick = false;
	_rightClick = false;
	_wheel = 0;
	_room.reset();
}

void MouseInput::beginPoll() {
	// A click or wheel step nobody consumed during the sample it arrived in, and the loop that ran after
	// it, is dropped so it can never act on a later screen.
	_pollCount++;
	if (_leftClick && _leftClickPoll + 1 < _pollCount) {
		_leftClick = false;
	}

	if (_rightClick && _rightClickPoll + 1 < _pollCount) {
		_rightClick = false;
	}

	if (_wheel != 0 && _wheelPoll + 1 < _pollCount) {
		_wheel = 0;
	}
}

ControllerButtons MouseInput::sampleControllerButtons(RuntimeState &state, InputScreen screen) {
	updateCursor(screen);
	state.MouseCursorActive = _active && screen == InputScreen::Room;
	ControllerButtons held = ControllerButtons::None;
	bool roomIdle = _mode == Mode::RoomIdle && screen == InputScreen::Room &&
					(state.DisplayFlags & kInteractionPendingMask) == 0;
	if (screen == InputScreen::Room && !roomIdle) {
		_room.updateBusy(state);
	}

	if (!_active) {
		state.MouseOpenInterfaceRequest = false;
	} else if (roomIdle) {
		held = _room.updateIdle(state, *this);
	} else if (_mode == Mode::SoundTest) {
		mapSoundTestClicks();
	} else if (_mode != Mode::Handled) {
		mapGenericClicks();
	}

	return held | emitPulse();
}

bool MouseInput::takeIntroSkipClick() {
	updateCursor(InputScreen::Intro);
	bool clicked = _leftClick;
	_leftClick = false;
	_rightClick = false;
	_wheel = 0;
	return clicked;
}

bool MouseInput::takeMoved() {
	bool moved = _moved;
	_moved = false;
	return moved;
}

bool MouseInput::takeLeftClick(Common::Point &contentPos) {
	if (!_leftClick) {
		return false;
	}

	_leftClick = false;
	contentPos = _leftClickPosition;
	return true;
}

bool MouseInput::takeRightClick() {
	if (!_rightClick) {
		return false;
	}

	_rightClick = false;
	return true;
}

void MouseInput::pulse(ControllerButtons buttons) {
	if (_pulseCount < kMaxQueuedPulses) {
		_pulseQueue[_pulseCount++] = buttons;
	}
}

ControllerButtons MouseInput::emitPulse() {
	// Every press is followed by one released sample, so each queued pulse is a fresh original press edge.
	if (_pulseSamplesLeft > 0) {
		if (--_pulseSamplesLeft > 0) {
			return _pulseQueue[0];
		}

		for (int i = 1; i < _pulseCount; i++) {
			_pulseQueue[i - 1] = _pulseQueue[i];
		}

		_pulseCount--;
		_pulseReleasePending = true;
	}

	if (_pulseReleasePending || _pulseCount == 0) {
		_pulseReleasePending = false;
		return ControllerButtons::None;
	}

	_pulseSamplesLeft = kPulseSamples;
	return _pulseQueue[0];
}

ControllerButtons MouseInput::mapGenericClicks() {
	Common::Point click;
	if (takeLeftClick(click)) {
		pulse(ControllerButtons::B);
	}

	if (takeRightClick()) {
		pulse(ControllerButtons::A);
	}

	for (; _wheel < 0; _wheel++) {
		pulse(ControllerButtons::Up);
	}

	for (; _wheel > 0; _wheel--) {
		pulse(ControllerButtons::Down);
	}

	return ControllerButtons::None;
}

ControllerButtons MouseInput::mapSoundTestClicks() {
	Common::Point click;
	if (takeLeftClick(click)) {
		pulse(ControllerButtons::B);
	}

	if (takeRightClick()) {
		pulse(ControllerButtons::C);
	}

	for (; _wheel < 0; _wheel++) {
		pulse(ControllerButtons::Left);
	}

	for (; _wheel > 0; _wheel--) {
		pulse(ControllerButtons::Right);
	}

	return ControllerButtons::None;
}

Common::Point MouseInput::toContent(const Common::Point &screenPos) const {
	int contentLeft = _scene ? _scene->contentLeft() : 0;
	return Common::Point(static_cast<int16>(screenPos.x - contentLeft), screenPos.y);
}

void MouseInput::updateCursor(InputScreen screen) {
	CursorStyle wanted = !_active ? CursorStyle::Hidden
								  : (screen == InputScreen::Room ? CursorStyle::Crosshair : CursorStyle::Arrow);
	if (wanted == _cursorStyle) {
		return;
	}

	if (wanted == CursorStyle::Arrow) {
		Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
		CursorMan.replaceCursor(cursor);
		delete cursor;
	} else if (wanted == CursorStyle::Crosshair) {
		CursorMan.replaceCursor(_crosshairPixels, kCrosshairSize, kCrosshairSize, kCrosshairHotspot,
								kCrosshairHotspot, 0);
		CursorMan.replaceCursorPalette(kCrosshairPalette, 0, 3);
	}

	CursorMan.showMouse(wanted != CursorStyle::Hidden);
	_cursorStyle = wanted;
}

} // namespace Scooby
