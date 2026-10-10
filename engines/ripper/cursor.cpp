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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/cursor.h"

#include "common/debug.h"
#include "common/frac.h"
#include "common/system.h"
#include "graphics/cursorman.h"

#include "ripper/detection.h"
#include "ripper/resources.h"

namespace Ripper {

static const char *const kCursorAssetNames[] = {
	"mouse0.pl", "mouse1.pl", "mouse2.pl", "mouse3.pl",
	"mouse4.pl", "mouse5.pl", "mouse6.pl", "mouse7.pl",
	"mouse8.bbm", "mouse9.pl", "mouse10.pl", "mouse11.pl",
	"mouse12.pl", "mouse13.pl", "mouse14.bbm", "mouse15.bbm",
	"mouse16.bbm", "mouse17.bbm", "mouse18.pl", "mouse19.pl",
	"mouse20.pl", "mouse21.pl", "mouse22.pl", "mouse23.pl"
};

static const uint32 kCursorFrameDurationMillis = 100;

CursorManager::CursorManager() : _selectionIndex(0), _cursorIndex(0), _frameIndex(0),
		_nextFrameMillis(0), _initialized(false), _visible(false) {
}

CursorManager::~CursorManager() {
	setVisible(false);
}

bool CursorManager::initialize(ResourceManager &resources, uint assetCount) {
	if (assetCount == 0 || assetCount > ARRAYSIZE(kCursorAssetNames))
		return false;
	for (uint i = 0; i < assetCount; ++i) {
		if (!resources.loadInterfaceBitmapSequence(kCursorAssetNames[i], _assets[i]))
			return false;
	}

	_initialized = true;
	_selectionIndex = 0;
	_cursorIndex = 0;
	_frameIndex = 0;
	_nextFrameMillis = 0;
	debugC(1, kDebugCursor, "Ripper: loaded %u cursor assets", assetCount);
	return true;
}

void CursorManager::setSelectionIndex(int cursorIndex) {
	// SetUiSelectionIndex at 0x4a3fe clamps and stores the requested row without
	// applying its cursor presentation. Scene action 36 performs that second step.
	if (cursorIndex < 0)
		cursorIndex = 0;
	else if ((uint)cursorIndex >= ARRAYSIZE(kCursorAssetNames))
		cursorIndex = ARRAYSIZE(kCursorAssetNames) - 1;
	_selectionIndex = cursorIndex;
	debugC(2, kDebugCursor,
		"Ripper: stored UI selection index=%u", _selectionIndex);
}

void CursorManager::dispatchSelectionIndexChange(int cursorIndex) {
	// DispatchUiSelectionIndexChange at 0x4a630 ignores out-of-range rows and
	// applies a valid row independently of the stored current-selection index.
	if (cursorIndex < 0 || (uint)cursorIndex >= ARRAYSIZE(kCursorAssetNames)) {
		debugC(3, kDebugCursor,
			"Ripper: ignored invalid UI selection dispatch index=%d count=%u",
			cursorIndex, ARRAYSIZE(kCursorAssetNames));
		return;
	}
	update(cursorIndex);
}

void CursorManager::applyCustomCursor(const BitmapAssetFrame &frame,
		int hotspotX, int hotspotY, uint scale) {
	if (frame.pixels.empty() || frame.width == 0 || frame.height == 0)
		return;
	if (scale == 0)
		scale = 1;
	if (!frame.palette.empty())
		CursorMan.replaceCursorPalette(frame.palette.data(), 0, frame.palette.size() / 3);
	const frac_t cursorScale = intToFrac((int16)scale);
	CursorMan.replaceCursor(frame.pixels.data(), frame.width, frame.height,
		hotspotX, hotspotY, frame.transparentColor, nullptr, nullptr,
		cursorScale, cursorScale);
	debugC(11, kDebugCursor,
		"Ripper: applied custom cursor size=%ux%u hotspot=%d,%d scale=%u transparent=%u",
		frame.width, frame.height, hotspotX, hotspotY, scale,
		frame.transparentColor);
}

void CursorManager::getHotspot(const BitmapAssetFrame &frame, int &x, int &y) const {
	if (_cursorIndex == 14 || _cursorIndex == 16) {
		x = 0;
		y = 0;
	} else if (_cursorIndex == 15) {
		x = frame.width / 2;
		y = 0;
	} else {
		x = frame.width / 2;
		y = frame.height / 2;
	}
}

void CursorManager::applyFrame() {
	if (!_initialized || _assets[_cursorIndex].frames.empty())
		return;

	const BitmapAssetFrame &frame = _assets[_cursorIndex].frames[_frameIndex];
	int hotspotX = 0;
	int hotspotY = 0;
	getHotspot(frame, hotspotX, hotspotY);
	if (!frame.palette.empty())
		CursorMan.replaceCursorPalette(frame.palette.data(), 0, frame.palette.size() / 3);
	CursorMan.replaceCursor(frame.pixels.data(), frame.width, frame.height,
		hotspotX, hotspotY, frame.transparentColor);
	debugC(11, kDebugCursor,
		"Ripper: applied cursor=%u frame=%u size=%ux%u hotspot=%d,%d transparent=%u",
		_cursorIndex, _frameIndex, frame.width, frame.height, hotspotX, hotspotY,
		frame.transparentColor);
}

void CursorManager::update(uint cursorIndex) {
	if (!_initialized)
		return;
	if (cursorIndex >= ARRAYSIZE(kCursorAssetNames)) {
		warning("Ripper: invalid cursor index %u; using default", cursorIndex);
		cursorIndex = 0;
	}
	const uint32 now = g_system->getMillis(true);
	if (cursorIndex != _cursorIndex || _nextFrameMillis == 0) {
		_cursorIndex = cursorIndex;
		_frameIndex = 0;
		_nextFrameMillis = now + kCursorFrameDurationMillis;
		applyFrame();
		debugC(2, kDebugCursor, "Ripper: selected cursor=%u frames=%u",
			_cursorIndex, _assets[_cursorIndex].frames.size());
	} else if (_assets[_cursorIndex].frames.size() > 1 && now >= _nextFrameMillis) {
		_frameIndex = (_frameIndex + 1) % _assets[_cursorIndex].frames.size();
		_nextFrameMillis = now + kCursorFrameDurationMillis;
		applyFrame();
	}

	setVisible(true);
}

void CursorManager::refresh() {
	applyFrame();
	setVisible(true);
	debugC(11, kDebugCursor, "Ripper: refreshed cursor=%u frame=%u", _cursorIndex,
		_frameIndex);
}

void CursorManager::setVisible(bool visible) {
	if (_visible == visible)
		return;
	_visible = visible;
	CursorMan.showMouse(visible);
	debugC(2, kDebugCursor, "Ripper: cursor %s", visible ? "shown" : "hidden");
}

} // End of namespace Ripper
