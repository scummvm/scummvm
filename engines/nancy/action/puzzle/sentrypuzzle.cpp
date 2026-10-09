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
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/sentrypuzzle.h"

namespace Nancy {
namespace Action {

// What a room keeps of the player under its name
enum {
	kSavedLeft		= 0,
	kSavedTop		= 1,
	kSavedRight		= 2,
	kSavedBottom	= 3,
	kSavedHeading	= 4,
	kNumSavedValues	= 5
};

static const double kWalkFramesPerSecond = 15.0;

// How far past the middle of its cell the player has to be asked to go
static const double kSnapNudge = 0.1;

static const int kMaxBeamLength = 1000;

static void storeSentryPosition(const Common::String &puzzleName, const Common::Rect &rect, int32 heading) {
	auto *data = (SavedStateData *)NancySceneState.getPuzzleData(SavedStateData::getTag());
	assert(data);

	Common::Array<int32> &values = data->states[puzzleName];
	values.resize(kNumSavedValues);
	values[kSavedLeft] = rect.left;
	values[kSavedTop] = rect.top;
	values[kSavedRight] = rect.right;
	values[kSavedBottom] = rect.bottom;
	values[kSavedHeading] = heading;
}

void SentryStartPosition::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _puzzleName);
	readExclusiveRect(stream, _rect);
	_heading = stream.readSint32LE();
}

void SentryStartPosition::execute() {
	if (!_puzzleName.empty()) {
		storeSentryPosition(_puzzleName, _rect, _heading);
	}

	_isDone = true;
}

void SentryPuzzle::readBeam(Common::SeekableReadStream &stream, Beam &beam) {
	beam.width = stream.readSint32LE();
	readExclusiveRect(stream, beam.src);
	readExclusiveRect(stream, beam.endSrc);
}

void SentryPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _name);
	readFilename(stream, _imageName);
	readFilename(stream, _mapName);
	_seenFlag = stream.readSint16LE();
	readExclusiveRect(stream, _startRect);
	_startHeading = stream.readSint32LE();
	_playerSpeed = stream.readSint32LE();
	_cellSize = MAX<int32>(stream.readSint32LE(), 1);

	_playerFrameSrcs.resize(stream.readUint16LE());
	for (Common::Rect &src : _playerFrameSrcs) {
		readExclusiveRect(stream, src);
	}

	_sentries.resize(stream.readUint16LE());
	for (Sentry &sentry : _sentries) {
		readFilename(stream, sentry.imageName);

		sentry.frameSrcs.resize(stream.readUint16LE());
		for (Common::Rect &src : sentry.frameSrcs) {
			readExclusiveRect(stream, src);
		}

		sentry.alertFrameSrcs.resize(stream.readUint16LE());
		for (Common::Rect &src : sentry.alertFrameSrcs) {
			readExclusiveRect(stream, src);
		}

		readBeam(stream, sentry.beam);
		readBeam(stream, sentry.alertBeam);

		// The path goes through the middle of each of its rects
		sentry.pathMode = stream.readByte();
		sentry.path.resize(stream.readUint16LE());
		for (Common::Point &point : sentry.path) {
			Common::Rect rect;
			readExclusiveRect(stream, rect);
			point.x = (rect.left + rect.right) / 2;
			point.y = (rect.top + rect.bottom) / 2;
		}

		sentry.speed = stream.readSint32LE();
		sentry.turnsPerSecond = stream.readDoubleLE();
		sentry.numBeamSteps = MAX<int32>(stream.readSint32LE(), 1);
	}

	readActionZoneArray(stream, _zones, true);
	readExitHotspot(stream);
}

void SentryPuzzle::init() {
	initViewportSurface();
	loadImage();
	g_nancy->_resource->loadImage(_mapName, _map);

	if (!_playerFrameSrcs.empty()) {
		// The player takes up a little less room than their picture
		_playerSize.x = _playerFrameSrcs[0].width() - 4;
		_playerSize.y = _playerFrameSrcs[0].height() - 4;
	}

	// The player is where the room last had them, or where the record
	// they came in through put them
	Common::Rect startRect = _startRect;
	int32 startHeading = _startHeading;

	const auto *data = (const SavedStateData *)NancySceneState.getPuzzleData(SavedStateData::getTag());
	assert(data);
	if (!_name.empty() && data->states.contains(_name) && data->states[_name].size() == kNumSavedValues) {
		const Common::Array<int32> &values = data->states[_name];
		startRect = Common::Rect(values[kSavedLeft], values[kSavedTop], values[kSavedRight], values[kSavedBottom]);
		startHeading = values[kSavedHeading];
	}

	_playerX = _targetX = (startRect.left + startRect.right) / 2;
	_playerY = _targetY = (startRect.top + startRect.bottom) / 2;
	_playerHeading = startHeading * M_PI / 180.0;

	for (Sentry &sentry : _sentries) {
		g_nancy->_resource->loadImage(sentry.imageName, sentry.image);

		if (!sentry.path.empty()) {
			sentry.x = sentry.path[0].x;
			sentry.y = sentry.path[0].y;
		}

		sentry.nextPoint = 1;
		sentry.hasStopped = sentry.path.size() < 2;
		sentry.beamStepOffset = g_nancy->_randomSource->getRandomNumber(sentry.numBeamSteps - 1);
		castBeam(sentry);
	}

	_startTime = _lastUpdateTime = g_nancy->getTotalPlayTime();
	redraw();
}

Common::Rect SentryPuzzle::getPlayerRect(double x, double y) const {
	Common::Rect rect(_playerSize.x, _playerSize.y);
	rect.moveTo((int)x - _playerSize.x / 2, (int)y - _playerSize.y / 2);
	return rect;
}

// The map shows where the floor is; everything else is a wall
bool SentryPuzzle::isWalkable(int x, int y) const {
	if (x < 0 || y < 0 || x >= _map.w || y >= _map.h) {
		return false;
	}

	byte a, r, g, b;
	_map.format.colorToARGB(_map.getPixel(x, y), a, r, g, b);
	return a != 0;
}

bool SentryPuzzle::canPlayerBeAt(double x, double y) const {
	const Common::Rect rect = getPlayerRect(x, y);

	for (int row = rect.top; row < rect.bottom; ++row) {
		for (int column = rect.left; column < rect.right; ++column) {
			if (!isWalkable(column, row)) {
				return false;
			}
		}
	}

	// A zone can close off a part of the floor, for as long as its event flag says so
	for (const ActionZone &zone : _zones) {
		if (zone.type == kZoneBoundary && zone.rect.intersects(rect) &&
				(zone.val49 == -1 || NancySceneState.getEventFlag(zone.val49, zone.val4b))) {
			return false;
		}
	}

	return true;
}

// The middle of the last cell that a position is at or past
int32 SentryPuzzle::snapToCell(double position) const {
	const int32 half = _cellSize / 2;
	return ((int32)(position + half) / _cellSize) * _cellSize + half - _cellSize;
}

void SentryPuzzle::movePlayer(uint32 elapsed) {
	bool goesRight = _heldKeys & NancyInput::kMoveRight;
	bool goesLeft = _heldKeys & NancyInput::kMoveLeft;
	bool goesUp = _heldKeys & NancyInput::kMoveUp;
	bool goesDown = _heldKeys & NancyInput::kMoveDown;

	// The mouse pulls the player along once it is half a cell away
	if (_isMouseHeld) {
		const double reach = _cellSize * 0.5;
		goesRight = goesRight || _mousePos.x - _playerX > reach;
		goesLeft = goesLeft || _mousePos.x - _playerX < -reach;
		goesUp = goesUp || _mousePos.y - _playerY < -reach;
		goesDown = goesDown || _mousePos.y - _playerY > reach;
	}

	// The player heads for the middle of the next cell, along each direction on its own
	bool hasNewTarget = false;
	if (goesRight || goesLeft) {
		double x = goesRight ? snapToCell(_playerX + kSnapNudge) + _cellSize : snapToCell(_playerX - kSnapNudge);
		if (canPlayerBeAt(x, _playerY)) {
			_targetX = x;
			hasNewTarget = true;
		}
	}

	if (goesUp || goesDown) {
		double y = goesDown ? snapToCell(_playerY + kSnapNudge) + _cellSize : snapToCell(_playerY - kSnapNudge);
		if (canPlayerBeAt(_playerX, y)) {
			_targetY = y;
			hasNewTarget = true;
		}
	}

	const double dx = _targetX - _playerX;
	const double dy = _targetY - _playerY;
	const double distance = sqrt(dx * dx + dy * dy);

	if (distance <= 1.0 && !hasNewTarget) {
		if (canPlayerBeAt(_targetX, _targetY)) {
			_playerX = _targetX;
			_playerY = _targetY;
		}

		_isPlayerMoving = false;
		return;
	}

	_playerHeading = atan2(-dy, dx);
	if (_playerHeading < 0) {
		_playerHeading += 2 * M_PI;
	}

	// The player slows down on the way into a cell they weren't asked to leave
	double speed = _playerSpeed;
	if (!hasNewTarget) {
		speed *= distance / (_cellSize * 0.5);
	}

	const double step = MIN(speed * elapsed / 1000.0, distance);
	double x = _playerX;
	double y = _playerY;
	if (distance > 0) {
		x += dx / distance * step;
		y += dy / distance * step;
	}

	if (canPlayerBeAt(x, y)) {
		_playerX = x;
		_playerY = y;
	} else if (canPlayerBeAt(x, _playerY)) {
		_playerX = x;
	} else if (canPlayerBeAt(_playerX, y)) {
		_playerY = y;
	}

	if (!_isPlayerMoving) {
		_isPlayerMoving = true;
		_walkStartTime = g_nancy->getTotalPlayTime();
	}
}

void SentryPuzzle::moveSentry(Sentry &sentry, uint32 elapsed) {
	double step = sentry.speed * elapsed / 1000.0;

	while (step > 0 && !sentry.hasStopped) {
		const Common::Point &target = sentry.path[sentry.nextPoint];
		const double dx = target.x - sentry.x;
		const double dy = target.y - sentry.y;
		const double distance = sqrt(dx * dx + dy * dy);

		if (distance > step) {
			sentry.x += dx / distance * step;
			sentry.y += dy / distance * step;
			break;
		}

		sentry.x = target.x;
		sentry.y = target.y;
		step -= distance;

		if (sentry.nextPoint + 1 < sentry.path.size()) {
			++sentry.nextPoint;
		} else if (sentry.pathMode == kPathStartsOver) {
			sentry.nextPoint = 0;
		} else if (sentry.pathMode == kPathTurnsAround) {
			for (uint i = 0; i < sentry.path.size() / 2; ++i) {
				SWAP(sentry.path[i], sentry.path[sentry.path.size() - 1 - i]);
			}

			sentry.nextPoint = 1;
		} else {
			sentry.hasStopped = true;
		}
	}
}

// The beam turns around the sentry, and reaches as far as the floor does
void SentryPuzzle::castBeam(Sentry &sentry) {
	const uint32 elapsed = g_nancy->getTotalPlayTime() - _startTime;
	const uint32 step = (sentry.beamStepOffset + (uint32)(elapsed * sentry.numBeamSteps * sentry.turnsPerSecond / 1000.0)) % sentry.numBeamSteps;
	const double angle = step * 2 * M_PI / sentry.numBeamSteps;
	const double dirX = cos(angle);
	const double dirY = -sin(angle);

	sentry.beamEndX = sentry.x;
	sentry.beamEndY = sentry.y;

	for (int i = 1; i <= kMaxBeamLength; ++i) {
		const double x = sentry.x + dirX * i;
		const double y = sentry.y + dirY * i;
		if (!isWalkable((int)x, (int)y)) {
			break;
		}

		sentry.beamEndX = x;
		sentry.beamEndY = y;
	}
}

bool SentryPuzzle::doesBeamHit(const Sentry &sentry, const Common::Rect &rect) const {
	const double dx = sentry.beamEndX - sentry.x;
	const double dy = sentry.beamEndY - sentry.y;
	const int length = (int)sqrt(dx * dx + dy * dy);

	for (int i = 0; i <= length; ++i) {
		const int x = (int)(sentry.x + (length ? dx * i / length : 0));
		const int y = (int)(sentry.y + (length ? dy * i / length : 0));
		if (rect.contains(x, y)) {
			return true;
		}
	}

	return false;
}

void SentryPuzzle::checkZones() {
	const Common::Rect rect = getPlayerRect(_playerX, _playerY);

	for (const ActionZone &zone : _zones) {
		if (zone.type != kZoneEventFlag || !zone.rect.intersects(rect)) {
			continue;
		}

		if (zone.val49 != -1 && !NancySceneState.getEventFlag(zone.val49, zone.val4b)) {
			continue;
		}

		NancySceneState.setEventFlag(zone.tailId, zone.tailFlag ? g_nancy->_true : g_nancy->_false);
	}
}

void SentryPuzzle::storePlayerPosition() {
	if (!_name.empty()) {
		storeSentryPosition(_name, getPlayerRect(_playerX, _playerY), (int32)(_playerHeading * 180.0 / M_PI));
	}
}

// The player's picture looks down the screen, and gets turned to where they are heading
void SentryPuzzle::drawPlayer() {
	if (_playerFrameSrcs.empty() || _image.empty()) {
		return;
	}

	uint frame = _playerFrameSrcs.size() - 1;
	if (_isPlayerMoving && _playerFrameSrcs.size() > 1) {
		const uint32 elapsed = g_nancy->getTotalPlayTime() - _walkStartTime;
		frame = (uint)(elapsed * kWalkFramesPerSecond / 1000.0) % (_playerFrameSrcs.size() - 1);
	}

	const Common::Rect src = _playerFrameSrcs[frame].findIntersectingRect(_image.getBounds());
	const double angle = -(_playerHeading + M_PI / 2);
	const double cosAngle = cos(angle);
	const double sinAngle = sin(angle);
	const int radius = (int)sqrt((double)(src.width() * src.width() + src.height() * src.height())) / 2 + 1;

	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			const int destX = (int)_playerX + dx;
			const int destY = (int)_playerY + dy;
			if (destX < 0 || destY < 0 || destX >= _drawSurface.w || destY >= _drawSurface.h) {
				continue;
			}

			// Back to where this pixel comes from in the picture
			const int srcX = src.left + src.width() / 2 + (int)floor(dx * cosAngle + dy * sinAngle + 0.5);
			const int srcY = src.top + src.height() / 2 + (int)floor(-dx * sinAngle + dy * cosAngle + 0.5);
			if (!src.contains(srcX, srcY)) {
				continue;
			}

			byte a, r, g, b;
			_image.format.colorToARGB(_image.getPixel(srcX, srcY), a, r, g, b);
			if (a >= 0x80) {
				_drawSurface.setPixel(destX, destY, _drawSurface.format.ARGBToColor(0xFF, r, g, b));
			}
		}
	}
}

void SentryPuzzle::drawSentry(const Sentry &sentry) {
	if (sentry.image.empty()) {
		return;
	}

	// The beam is a line in the color of its picture, as wide as the two halves of it
	const Beam &beam = sentry.hasSeenPlayer ? sentry.alertBeam : sentry.beam;
	const Common::Rect beamSrc = beam.src.findIntersectingRect(sentry.image.getBounds());
	if (!beamSrc.isEmpty()) {
		byte a, r, g, b;
		sentry.image.format.colorToARGB(sentry.image.getPixel(beamSrc.left + beamSrc.width() / 2, beamSrc.top), a, r, g, b);
		const int width = beamSrc.width() * 2 + beam.width;
		_drawSurface.drawThickLine((int)sentry.x, (int)sentry.y, (int)sentry.beamEndX, (int)sentry.beamEndY,
			width / 2, width / 2, _drawSurface.format.ARGBToColor(0xFF, r, g, b));
	}

	const Common::Array<Common::Rect> &srcs = sentry.hasSeenPlayer ? sentry.alertFrameSrcs : sentry.frameSrcs;
	if (srcs.empty()) {
		return;
	}

	const uint32 elapsed = g_nancy->getTotalPlayTime() - _startTime;
	const Common::Rect &src = srcs[(uint)(elapsed * srcs.size() * sentry.turnsPerSecond / 1000.0) % srcs.size()];
	_drawSurface.blitFrom(sentry.image, src, Common::Point((int)sentry.x - src.width() / 2, (int)sentry.y - src.height() / 2));
}

void SentryPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	drawPlayer();
	for (const Sentry &sentry : _sentries) {
		drawSentry(sentry);
	}

	_needsRedraw = true;
}

void SentryPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		const uint32 now = g_nancy->getTotalPlayTime();
		const uint32 elapsed = now - _lastUpdateTime;
		_lastUpdateTime = now;

		if (!_isSeen) {
			movePlayer(elapsed);
			storePlayerPosition();
			checkZones();
		}

		const Common::Rect playerRect = getPlayerRect(_playerX, _playerY);
		for (Sentry &sentry : _sentries) {
			// A sentry that has seen the player stays as it is
			if (sentry.hasSeenPlayer) {
				continue;
			}

			moveSentry(sentry, elapsed);
			castBeam(sentry);

			if (doesBeamHit(sentry, playerRect)) {
				sentry.hasSeenPlayer = true;
				_isSeen = true;
				_isPlayerMoving = false;
				NancySceneState.setEventFlag(_seenFlag, g_nancy->_true);
			}
		}

		redraw();
		break;
	}
	case kActionTrigger:
		_exitScene.execute();
		finishExecution();
		break;
	}
}

void SentryPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	_heldKeys = input.input & (NancyInput::kMoveUp | NancyInput::kMoveDown | NancyInput::kMoveLeft | NancyInput::kMoveRight);

	if (hoverExitHotspot(input)) {
		_isMouseHeld = false;

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_state = kActionTrigger;
			input.eatMouseInput();
		}

		return;
	}

	const Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	const bool isInViewport = NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos);
	_isMouseHeld = isInViewport && (input.input & (NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonHeld));

	if (_isMouseHeld) {
		const Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
			Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
		_mousePos = mouseRect.origin();
	}
}

} // End of namespace Action
} // End of namespace Nancy
