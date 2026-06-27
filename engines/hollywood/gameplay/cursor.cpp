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

#include "hollywood/gameplay/cursor.h"

#include "common/events.h"
#include "common/formats/winexe.h"
#include "common/path.h"
#include "common/ptr.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "graphics/wincursor.h"

namespace Hollywood {

const char *const kCursorExecutableName = "MONSTERS.EXE";
const uint16 kCursorInitialX = 0x140;
const uint16 kCursorInitialY = 0x0f0;
const uint16 kCursorMaxX = 0x27f;
const uint16 kCursorMaxY = 0x1df;
const uint32 kCursorFrameMillis = 75;
const uint16 kCursorGroupResourceIds[] = { 108, 109, 111, 112, 113, 114 };
const byte kCursorAnimationFrameMap[] = { 0, 1, 2, 3, 4, 5, 4, 3, 2, 1 };

HollywoodCursor::HollywoodCursor() :
		_frameTimer(0),
		_animationFrameIndex(0),
		_surfaceX(kCursorInitialX),
		_surfaceY(kCursorInitialY),
		_loaded(false),
		_visible(false) {
}

HollywoodCursor::~HollywoodCursor() {
	leaveInteractiveMode();
	clear();
}

bool HollywoodCursor::load() {
	if (_loaded)
		return true;

	Common::ScopedPtr<Common::WinResources> exe(Common::WinResources::createFromEXE(Common::Path(kCursorExecutableName)));
	if (!exe) {
		warning("Failed to load PE resources from %s for Hollywood cursor", kCursorExecutableName);
		return false;
	}

	_cursorGroups.reserve(ARRAYSIZE(kCursorGroupResourceIds));
	for (uint i = 0; i < ARRAYSIZE(kCursorGroupResourceIds); ++i) {
		Graphics::WinCursorGroup *group = Graphics::WinCursorGroup::createCursorGroup(exe.get(),
			Common::WinResourceID(kCursorGroupResourceIds[i]));
		if (!group || group->cursors.empty()) {
			delete group;
			clear();
			warning("Failed to load Hollywood cursor resource %u from %s", kCursorGroupResourceIds[i],
				kCursorExecutableName);
			return false;
		}
		_cursorGroups.push_back(group);
	}

	_loaded = true;
	return true;
}

void HollywoodCursor::enterInteractiveMode() {
	if (!load())
		return;

	_frameTimer = 0;
	_animationFrameIndex = 0;
	installCurrentFrame();
	CursorMan.showMouse(true);
	_visible = true;
}

void HollywoodCursor::leaveInteractiveMode() {
	if (_visible) {
		CursorMan.showMouse(false);
		_visible = false;
	}
}

void HollywoodCursor::updatePosition(const Common::Point &point) {
	_surfaceX = (uint16)CLIP<int>(point.x, 0, kCursorMaxX);
	_surfaceY = (uint16)CLIP<int>(point.y, 0, kCursorMaxY);
}

void HollywoodCursor::advance(uint32 deltaMillis) {
	if (!_visible || !_loaded)
		return;

	_frameTimer += deltaMillis;
	bool changed = false;
	while (_frameTimer >= kCursorFrameMillis) {
		_frameTimer -= kCursorFrameMillis;
		_animationFrameIndex = _animationFrameIndex + 1 == ARRAYSIZE(kCursorAnimationFrameMap) ?
			0 : _animationFrameIndex + 1;
		changed = true;
	}

	if (changed)
		installCurrentFrame();
}

void HollywoodCursor::clear() {
	for (uint i = 0; i < _cursorGroups.size(); ++i)
		delete _cursorGroups[i];
	_cursorGroups.clear();
	_loaded = false;
}

void HollywoodCursor::installCurrentFrame() {
	if (_animationFrameIndex >= ARRAYSIZE(kCursorAnimationFrameMap))
		return;

	const byte cursorGroupIndex = kCursorAnimationFrameMap[_animationFrameIndex];
	if (cursorGroupIndex >= _cursorGroups.size() || _cursorGroups[cursorGroupIndex]->cursors.empty())
		return;

	CursorMan.replaceCursor(_cursorGroups[cursorGroupIndex]->cursors[0].cursor);
}

} // End of namespace Hollywood
