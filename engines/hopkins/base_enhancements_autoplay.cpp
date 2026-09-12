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

#include "hopkins/base_enhancements_autoplay.h"

#include "hopkins/base_data.h"
#include "hopkins/base_engine.h"
#include "hopkins/base_types.h"

#include "common/algorithm.h"
#include "common/translation.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Hopkins {

namespace {

struct AutoplayDestination {
	int roomId;
	int exitMapPos;
	int approachMapPos;
	const char *label;
};

// WBASE transition cells and the adjacent cells used to approach them.
static const AutoplayDestination kAutoplayDestinations[] = {
	{ 94, 0x0860, 0x0861, "Exit 1: Sector 4" },
	{ 95, 0x087a, 0x0879, "Exit 2: Sector 2" },
	{ 96, 0x01df, 0x021f, "Exit 3: Office" },
	{ 97, 0x05ab, 0x05eb, "Exit 4: Laboratory" },
	{ 98, 0x0bab, 0x0b6b, "Exit 5: Resurrection" },
	{ 99, 0x01fb, 0x023b, "Exit 6: Bernie's room" }
};

static const int kAutoplayDestinationCount = ARRAYSIZE(kAutoplayDestinations);
static const int kAutoplayAimTolerance = 24;
static const int kAutoplayMoveTolerance = 48;
static const int kAutoplayWaypointTolerance = 12;
static const int kAutoplayCombatDistance = 600;
static const int kAutoplayStuckTicks = 24;

static int absoluteValue(int value) {
	return value < 0 ? -value : value;
}

static int signedAngleDelta(int from, int to) {
	int delta = normalizeBaseAngle(to - from);
	if (delta > kBaseHalfTurn)
		delta -= kBaseAngleCount;
	return delta;
}

static bool traversableEdge(uint16 code) {
	if (code == 0 || (code & kBaseWallPass))
		return true;
	const byte lowCode = code & 0xff;
	return (lowCode == kBaseDoorXCode || lowCode == kBaseDoorYCode) && !(code & kBaseDoorLocked);
}

static bool canStep(const BaseEngine &engine, int from, int to) {
	const int fromX = from & 63;
	const int fromY = from >> 6;
	const int toX = to & 63;
	const int toY = to >> 6;
	if (toX < 0 || toX >= kBaseMapWidth || toY < 0 || toY >= kBaseMapHeight)
		return false;

	if (toX == fromX + 1 && toY == fromY)
		return traversableEdge(engine.xGrid()[from + 1]);
	if (toX == fromX - 1 && toY == fromY)
		return traversableEdge(engine.xGrid()[from]);
	if (toX == fromX && toY == fromY + 1)
		return traversableEdge(engine.yGrid()[from + kBaseMapWidth]);
	if (toX == fromX && toY == fromY - 1)
		return traversableEdge(engine.yGrid()[from]);
	return false;
}

static byte nearestPaletteColor(const byte *palette, int red, int green, int blue) {
	int bestDistance = 0x7fffffff;
	byte bestColor = 0;
	for (int color = 0; color < 256; ++color) {
		const int offset = color * 3;
		const int redDelta = palette[offset] - red;
		const int greenDelta = palette[offset + 1] - green;
		const int blueDelta = palette[offset + 2] - blue;
		const int distance = redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;
		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = color;
		}
	}
	return bestColor;
}

static void fillRect(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	left = MAX(left, 0);
	top = MAX(top, 0);
	right = MIN(right, kBaseFrameWidth - 1);
	bottom = MIN(bottom, kBaseFrameHeight - 1);
	for (int y = top; y <= bottom; ++y)
		Common::fill(framebuffer + y * kBaseFrameWidth + left,
				framebuffer + y * kBaseFrameWidth + right + 1, color);
}

static void drawFrame(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	fillRect(framebuffer, left, top, right, top, color);
	fillRect(framebuffer, left, bottom, right, bottom, color);
	fillRect(framebuffer, left, top, left, bottom, color);
	fillRect(framebuffer, right, top, right, bottom, color);
}

} // End of anonymous namespace

WBASEEnhancementsAutoplay::WBASEEnhancementsAutoplay() {
	reset();
}

void WBASEEnhancementsAutoplay::reset() {
	_active = false;
	_menuVisible = false;
	_selectedDestination = 0;
	_destination = -1;
	_route.clear();
	_routeIndex = 0;
	_lastPlayerX = -1;
	_lastPlayerY = -1;
	_stuckTicks = 0;
	_routeInvalidated = false;
}

void WBASEEnhancementsAutoplay::openMenu() {
	_menuVisible = true;
}

void WBASEEnhancementsAutoplay::closeMenu() {
	_menuVisible = false;
}

void WBASEEnhancementsAutoplay::selectPrevious() {
	_selectedDestination = (_selectedDestination + kAutoplayDestinationCount - 1) % kAutoplayDestinationCount;
}

void WBASEEnhancementsAutoplay::selectNext() {
	_selectedDestination = (_selectedDestination + 1) % kAutoplayDestinationCount;
}

void WBASEEnhancementsAutoplay::startSelected(const BaseEngine &engine) {
	_destination = _selectedDestination;
	_active = true;
	_menuVisible = false;
	_lastPlayerX = engine.playerX();
	_lastPlayerY = engine.playerY();
	_stuckTicks = 0;
	_routeInvalidated = false;
	rebuildRoute(engine);
}

void WBASEEnhancementsAutoplay::cancel() {
	_active = false;
	_menuVisible = false;
	_destination = -1;
	_route.clear();
	_routeIndex = 0;
}

bool WBASEEnhancementsAutoplay::rebuildRoute(const BaseEngine &engine) {
	_route.clear();
	_routeIndex = 0;
	_routeInvalidated = false;
	if (_destination < 0 || _destination >= kAutoplayDestinationCount)
		return false;

	const int start = baseWorldMapIndex(engine.playerX(), engine.playerY());
	const int goal = kAutoplayDestinations[_destination].approachMapPos;
	if (start < 0 || start >= kBaseMapCellCount || goal < 0 || goal >= kBaseMapCellCount)
		return false;

	int parent[kBaseMapCellCount];
	int queue[kBaseMapCellCount];
	Common::fill(parent, parent + ARRAYSIZE(parent), -2);
	int queueBegin = 0;
	int queueEnd = 0;
	queue[queueEnd++] = start;
	parent[start] = -1;

	static const int kOffsets[] = { 1, -1, kBaseMapWidth, -kBaseMapWidth };
	while (queueBegin < queueEnd && parent[goal] == -2) {
		const int current = queue[queueBegin++];
		for (uint direction = 0; direction < ARRAYSIZE(kOffsets); ++direction) {
			const int next = current + kOffsets[direction];
			if (next < 0 || next >= kBaseMapCellCount || parent[next] != -2)
				continue;
			if (!canStep(engine, current, next))
				continue;
			parent[next] = current;
			queue[queueEnd++] = next;
		}
	}

	if (parent[goal] == -2)
		return false;

	Common::Array<int> reverseRoute;
	for (int mapPos = goal; mapPos >= 0; mapPos = parent[mapPos])
		reverseRoute.push_back(mapPos);
	for (int index = (int)reverseRoute.size() - 1; index >= 0; --index)
		_route.push_back(reverseRoute[index]);
	_routeIndex = _route.size() > 1 ? 1 : 0;
	return true;
}

int WBASEEnhancementsAutoplay::findVisibleGuard(const BaseEngine &engine) const {
	int bestGuard = 0;
	int bestDistance = kAutoplayCombatDistance + 1;
	for (int id = 1; id <= kBaseMaxObjects; ++id) {
		const BaseObject &object = engine.object(id);
		if (!object.active || object.passable || object.mode > kBaseObjectFlee)
			continue;
		const int distance = engine.objectDistance(object);
		if (distance > kAutoplayCombatDistance || distance >= bestDistance)
			continue;

		const int angle = engine.objectAngle(object.x - engine.playerX(), object.y - engine.playerY());
		const BaseRayHit wall = engine.castRay(angle, kBaseViewHalfWidth);
		if (wall.hit && wall.distance + 24 < distance)
			continue;
		bestGuard = id;
		bestDistance = distance;
	}
	return bestGuard;
}

bool WBASEEnhancementsAutoplay::aimAt(const BaseEngine &engine, int targetX, int targetY,
		BaseInputState &input) const {
	const int desiredAngle = engine.objectAngle(targetX - engine.playerX(), targetY - engine.playerY());
	const int delta = signedAngleDelta(engine.playerAngle(), desiredAngle);
	if (delta > kAutoplayAimTolerance)
		input.turnRight = true;
	else if (delta < -kAutoplayAimTolerance)
		input.turnLeft = true;
	return absoluteValue(delta) <= kAutoplayAimTolerance;
}

void WBASEEnhancementsAutoplay::updateStuckState(const BaseEngine &engine, bool triedToMove) {
	if (!triedToMove || engine.playerX() != _lastPlayerX || engine.playerY() != _lastPlayerY) {
		_stuckTicks = 0;
	} else if (++_stuckTicks >= kAutoplayStuckTicks) {
		_routeInvalidated = true;
		_stuckTicks = 0;
	}
	_lastPlayerX = engine.playerX();
	_lastPlayerY = engine.playerY();
}

void WBASEEnhancementsAutoplay::update(const BaseEngine &engine, BaseInputState &input) {
	input = BaseInputState();
	if (!_active || _menuVisible || _destination < 0 || _destination >= kAutoplayDestinationCount)
		return;

	const int guardId = findVisibleGuard(engine);
	if (guardId) {
		const BaseObject &guard = engine.object(guardId);
		if (aimAt(engine, guard.x, guard.y, input) && engine.weaponCounter() == 0)
			input.fire = true;
		updateStuckState(engine, false);
		return;
	}

	const AutoplayDestination &destination = kAutoplayDestinations[_destination];
	const int playerMapPos = baseWorldMapIndex(engine.playerX(), engine.playerY());
	if (playerMapPos == destination.approachMapPos) {
		const int exitX = (destination.exitMapPos & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int exitY = (destination.exitMapPos >> 6) * kBaseCellSize + kBaseCellSize / 2;
		const int approachX = (destination.approachMapPos & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int approachY = (destination.approachMapPos >> 6) * kBaseCellSize + kBaseCellSize / 2;
		const int centerDistance = absoluteValue(engine.playerX() - approachX) + absoluteValue(engine.playerY() - approachY);
		if (centerDistance > kAutoplayWaypointTolerance) {
			const bool aligned = aimAt(engine, approachX, approachY, input);
			input.forward = aligned;
			updateStuckState(engine, input.forward);
			return;
		}
		if (aimAt(engine, exitX, exitY, input))
			input.exitRequested = true;
		updateStuckState(engine, false);
		return;
	}

	if (_routeInvalidated || _route.empty() || _routeIndex >= _route.size())
		rebuildRoute(engine);
	if (_route.empty()) {
		cancel();
		return;
	}

	while (_routeIndex < _route.size() && playerMapPos == _route[_routeIndex]) {
		const int waypointX = (_route[_routeIndex] & 63) * kBaseCellSize + kBaseCellSize / 2;
		const int waypointY = (_route[_routeIndex] >> 6) * kBaseCellSize + kBaseCellSize / 2;
		if (absoluteValue(engine.playerX() - waypointX) + absoluteValue(engine.playerY() - waypointY) >
				kAutoplayWaypointTolerance)
			break;
		++_routeIndex;
	}

	if (_routeIndex >= _route.size()) {
		_routeInvalidated = true;
		updateStuckState(engine, false);
		return;
	}

	const int expectedCell = _routeIndex > 0 ? _route[_routeIndex - 1] : _route[0];
	if (playerMapPos != expectedCell && playerMapPos != _route[_routeIndex]) {
		_routeInvalidated = true;
		updateStuckState(engine, false);
		return;
	}

	const int waypointX = (_route[_routeIndex] & 63) * kBaseCellSize + kBaseCellSize / 2;
	const int waypointY = (_route[_routeIndex] >> 6) * kBaseCellSize + kBaseCellSize / 2;
	const int desiredAngle = engine.objectAngle(waypointX - engine.playerX(), waypointY - engine.playerY());
	const int delta = signedAngleDelta(engine.playerAngle(), desiredAngle);
	if (delta > kAutoplayAimTolerance)
		input.turnRight = true;
	else if (delta < -kAutoplayAimTolerance)
		input.turnLeft = true;
	input.forward = absoluteValue(delta) <= kAutoplayMoveTolerance;
	updateStuckState(engine, input.forward);
}

void WBASEEnhancementsAutoplay::render(const BaseData &data, byte *framebuffer, bool showStatus) const {
	if (!framebuffer || (!_menuVisible && (!_active || !showStatus)))
		return;

	Graphics::Surface surface;
	surface.init(kBaseFrameWidth, kBaseFrameHeight, kBaseFrameWidth, framebuffer,
			Graphics::PixelFormat::createFormatCLUT8());
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return;

	const byte *palette = data.palette();
	const byte background = nearestPaletteColor(palette, 0, 0, 0);
	const byte border = nearestPaletteColor(palette, 190, 190, 190);
	const byte text = nearestPaletteColor(palette, 255, 255, 255);
	const byte selected = nearestPaletteColor(palette, 40, 255, 60);

	if (_menuVisible) {
		static const int kLeft = 42;
		static const int kTop = 27;
		static const int kRight = 277;
		static const int kBottom = 172;
		fillRect(framebuffer, kLeft, kTop, kRight, kBottom, background);
		drawFrame(framebuffer, kLeft, kTop, kRight, kBottom, border);
		font->drawString(&surface, _("WBASE AUTOPLAY"), kLeft + 4, kTop + 9,
				kRight - kLeft - 7, text, Graphics::kTextAlignCenter);
		font->drawString(&surface, _("Fight your way to:"), kLeft + 4, kTop + 27,
				kRight - kLeft - 7, text, Graphics::kTextAlignCenter);
		for (int index = 0; index < kAutoplayDestinationCount; ++index) {
			const Common::U32String label = Common::U32String::format("%c %S",
					index == _selectedDestination ? '>' : ' ', _(kAutoplayDestinations[index].label).c_str());
			font->drawString(&surface, label, kLeft + 18, kTop + 45 + index * 12,
					kRight - kLeft - 35, index == _selectedDestination ? selected : text);
		}
		font->drawString(&surface, _("Up/Down: choose   Space: start   Esc: cancel"),
				kLeft + 4, kBottom - 15, kRight - kLeft - 7, text, Graphics::kTextAlignCenter);
		return;
	}

	const Common::U32String status = Common::U32String::format(_("AUTO: %S   A: route"),
			_(kAutoplayDestinations[_destination].label).c_str());
	const int statusWidth = MIN(font->getStringWidth(status) + 10, kBaseFrameWidth - 8);
	const int statusLeft = (kBaseFrameWidth - statusWidth) / 2;
	fillRect(framebuffer, statusLeft, 2, statusLeft + statusWidth - 1, 14, background);
	drawFrame(framebuffer, statusLeft, 2, statusLeft + statusWidth - 1, 14, border);
	font->drawString(&surface, status, statusLeft + 4, 4, statusWidth - 8, selected,
			Graphics::kTextAlignCenter);
}

} // End of namespace Hopkins
