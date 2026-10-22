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
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/snowmobilepuzzle.h"

namespace Nancy {
namespace Action {

static const uint32 kMsPerTick = 20;
static const double kTicksPerSecond = 50.0;

static const double kTurnPerTick = M_PI / kTicksPerSecond;
static const double kBrakeFactor = 0.4;			// braking is this much quicker than speeding up
static const double kFrictionFactor = 3.0;		// seconds it takes to coast to a stop from full speed
static const double kMinSpeed = 0.01;
static const int32 kMaxDamage = 100;

// How fast the chased snowmobile runs through its path, depending on where the player is
static const double kChaseRateBehind = 5.0;		// it is behind the player
static const double kChaseRateInReach = 1.2;	// the player is within the next stage's reach
static const double kChaseRateFar = 0.6;		// more than half a view ahead
static const double kChaseRateOutOfView = 0.2;
static const double kChaseCloseDistance = 60.0;

static const uint16 kMouseSteeringCursor = 0;
static const uint16 kKeySteeringCursor = 23;

// The field is far larger than what a Common::Rect can hold. Its rects are stored
// with exclusive right and bottom edges
static void readRect32(Common::SeekableReadStream &stream, Common::Rect32 &rect) {
	rect.left = stream.readSint32LE();
	rect.top = stream.readSint32LE();
	rect.right = stream.readSint32LE();
	rect.bottom = stream.readSint32LE();
}

void SnowmobilePuzzle::Zone::centerOn(const Math::Vector2d &pos) {
	int32 dx = (int32)(pos.getX() - rect.center().x);
	int32 dy = (int32)(pos.getY() - rect.center().y);
	rect.translate(dx, dy);

	if (type == kSnowZoneOverlay || type == kSnowZoneTiledOverlay) {
		dest.translate(dx, dy);
	} else if (type == kSnowZonePointer) {
		target = Math::Vector2d(rect.center().x, rect.center().y);
	}
}

Common::Rect32 SnowmobilePuzzle::Snowmobile::getBounds() const {
	int32 left = (int32)(pos.getX() - size / 2);
	int32 top = (int32)(pos.getY() - size / 2);
	return Common::Rect32(left, top, left + size, top + size);
}

SnowmobilePuzzle::~SnowmobilePuzzle() {
	for (Graphics::ManagedSurface *sheet : _sheets) {
		delete sheet;
	}
}

uint SnowmobilePuzzle::loadSheet(const Common::String &name) {
	for (uint i = 0; i < _sheetNames.size(); ++i) {
		if (_sheetNames[i].equalsIgnoreCase(name)) {
			return i;
		}
	}

	Graphics::ManagedSurface *sheet = new Graphics::ManagedSurface();
	g_nancy->_resource->loadImage(Common::Path(name), *sheet);
	_sheets.push_back(sheet);
	_sheetNames.push_back(name);
	return _sheets.size() - 1;
}

void SnowmobilePuzzle::readZone(Common::SeekableReadStream &stream, Zone &zone) {
	zone.type = stream.readSint32LE() & 0xff;
	readRect32(stream, zone.rect);
	stream.skip(33);	// collision mask, not used by the snowmobile rides
	zone.minSpeed = stream.readDoubleLE();
	zone.maxSpeed = stream.readDoubleLE();
	zone.conditionFlag = stream.readSint16LE();
	zone.conditionValue = stream.readByte();
	stream.skip(4);		// delay before the zone takes effect, always 0
	zone.sound.readData(stream);

	switch (zone.type) {
	case kSnowZoneChangeScene:
		zone.sceneID = stream.readUint16LE();
		stream.skip(1);
		break;
	case kSnowZoneFlag:
		zone.flagLabel = stream.readSint16LE();
		zone.flagValue = stream.readByte();
		break;
	case kSnowZoneDamage:
		zone.minDamage = stream.readSint32LE();
		zone.maxDamage = stream.readSint32LE();
		break;
	case kSnowZoneOverlay:
	case kSnowZoneTiledOverlay:
	case kSnowZonePointer: {
		Common::String imageName;
		readFilename(stream, imageName);
		zone.imageID = loadSheet(imageName);

		// The rides only have still overlays
		int16 numFrames = stream.readSint16LE();
		if (numFrames < 1) {
			error("SnowmobilePuzzle: overlay zone without frames");
		}

		readRect32(stream, zone.src);
		stream.skip((numFrames - 1) * 16);
		readRect32(stream, zone.dest);
		stream.skip(4);	// animation loops
		zone.layer = stream.readSint32LE();
		break;
	}
	case kSnowZoneSound:
	case kSnowZoneBoundary:
	case kSnowZoneNoRandomZones:
		break;
	default:
		error("SnowmobilePuzzle: unhandled zone type %d", zone.type);
	}
}

void SnowmobilePuzzle::readZones(Common::SeekableReadStream &stream, Common::Array<Zone> &zones) {
	zones.resize(MAX<int16>(stream.readSint16LE(), 0));
	for (Zone &zone : zones) {
		readZone(stream, zone);
	}
}

void SnowmobilePuzzle::readData(Common::SeekableReadStream &stream) {
	stream.skip(33);	// name the ride's state is kept under, empty
	stream.skip(66);	// background image and collision mask, both empty
	readFilename(stream, _imageName);
	_startX = stream.readSint32LE();
	_startY = stream.readSint32LE();
	_startAngle = stream.readSint32LE();
	_forwardSpeed = stream.readSint32LE();
	_reverseSpeed = stream.readSint32LE();
	_resourceID = stream.readSint16LE();
	stream.skip(6);
	_crashScene = stream.readUint16LE();

	_crashSound.readData(stream);
	RandomSoundBlock hornSound;	// empty
	hornSound.readData(stream);
	_engineSound.readData(stream);

	// Only the first frame is used, turned to face the heading
	int16 numFrames = stream.readSint16LE();
	if (numFrames < 1) {
		error("SnowmobilePuzzle: no snowmobile frames");
	}

	readRect32(stream, _carSrc);
	stream.skip((numFrames - 1) * 16);

	if (_variant == kChase) {
		_stages.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (ChaseStage &stage : _stages) {
			stage.distance = stream.readSint16LE();
			stage.duration = stream.readSint32LE() * 1000;
			stage.reachedFlag = stream.readSint16LE();
			stage.inReachFlag = stream.readSint16LE();
		}

		_winScene = stream.readUint16LE();
		_winFlag = stream.readSint16LE();
		readFilename(stream, _chaserImageName);

		numFrames = stream.readSint16LE();
		if (numFrames < 1) {
			error("SnowmobilePuzzle: no frames for the chased snowmobile");
		}

		readRect32(stream, _chaserSrc);
		stream.skip((numFrames - 1) * 16);
	}

	readZones(stream, _zones);
	_numFixedZones = _zones.size();

	_cellSize = stream.readSint32LE();
	_density = stream.readSint32LE();
	if (_cellSize <= 0) {
		error("SnowmobilePuzzle: invalid cell size %d", _cellSize);
	}

	_sets.resize(stream.readUint16LE());
	int32 total = 0;
	for (ZoneSet &set : _sets) {
		total += (int32)stream.readFloatLE();
		set.maxRoll = total;
		readZones(stream, set.zones);
	}

	if (_variant == kChase) {
		// Zones for the chased snowmobile, none in the data
		Common::Array<Zone> chaserZones;
		readZones(stream, chaserZones);

		_path.resize(MAX<int16>(stream.readSint16LE(), 0));
		for (Waypoint &point : _path) {
			point.time = stream.readUint32LE();
			int16 x = stream.readSint16LE();
			int16 y = stream.readSint16LE();
			point.pos = Math::Vector2d(x, y);
			point.heading = stream.readDoubleLE();
		}

		if (_path.size() < 2 || _stages.empty()) {
			error("SnowmobilePuzzle: chase without a path or stages");
		}
	}
}

void SnowmobilePuzzle::finalizeZone(Zone &zone) const {
	if (zone.type != kSnowZoneOverlay && zone.type != kSnowZoneTiledOverlay && zone.type != kSnowZonePointer) {
		return;
	}

	// An overlay without a rect of its own shows whenever it may be in view
	if (zone.rect.isEmpty()) {
		zone.hasImplicitRect = true;
		zone.rect = Common::Rect32(zone.dest.left - _drawSurface.w, zone.dest.top - _drawSurface.h,
			zone.dest.right + _drawSurface.w, zone.dest.bottom + _drawSurface.h);
	}

	zone.target = Math::Vector2d(zone.rect.center().x, zone.rect.center().y);
}

void SnowmobilePuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();

	// The overlays have soft edges, so the surface carries an alpha channel
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getTransparentPixelFormat());
	_drawSurface.clear(0);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	_car = Snowmobile();
	_car.pos = Math::Vector2d(_startX, _startY);
	_car.heading = _startAngle * M_PI / 180.0;
	_car.maxSpeed = _forwardSpeed;
	_car.minSpeed = -_reverseSpeed;
	_car.friction = _forwardSpeed / kFrictionFactor / kTicksPerSecond;
	_car.size = MAX<int32>((_carSrc.width() + _carSrc.height()) / 2, 2);

	for (Zone &zone : _zones) {
		finalizeZone(zone);
	}

	for (ZoneSet &set : _sets) {
		for (Zone &zone : set.zones) {
			finalizeZone(zone);
		}
	}

	_cellRadiusX = (_cellSize - 1 + _drawSurface.w) / _cellSize;
	_cellRadiusY = (_cellSize - 1 + _drawSurface.h) / _cellSize;

	if (_variant == kChase) {
		_chaserImageID = loadSheet(_chaserImageName);
		_pathDuration = MAX<uint32>(_path.back().time - _path[0].time, 1);
		_pathLap = _path.back().pos - _path[0].pos;
		_pathTime = 0;
		_chaserPos = getPathPos(0);
		_chaserHeading = wrapAngle(_path[findWaypoint(0)].heading);
		_curStage = 0;

		// The first pointer follows the chased snowmobile
		for (uint i = 0; i < _numFixedZones; ++i) {
			if (_zones[i].type == kSnowZonePointer) {
				_pointerZone = i;
				_zones[i].centerOn(_chaserPos);
				break;
			}
		}
	}

	populateField();
	seedZones();

	if (!_engineSound.names.empty()) {
		_engineSoundDesc.name = _engineSound.names[0];
		_engineSoundDesc.channelID = _engineSound.channel;
		_engineSoundDesc.numLoops = _engineSound.numLoops;
		_engineSoundDesc.volume = _engineSound.volume;
		g_nancy->_sound->loadSound(_engineSoundDesc);
		g_nancy->_sound->playSound(_engineSoundDesc);
	}

	_lastMousePos = g_nancy->_input->getInput().mousePos;
	_lastTickTime = g_nancy->getTotalPlayTime();
	redraw();
}

void SnowmobilePuzzle::getCurrentCell(int32 &cellX, int32 &cellY) const {
	Common::Rect32 bounds = _car.getBounds();
	cellX = bounds.center().x / _cellSize;
	cellY = bounds.center().y / _cellSize;
}

// Fills the surroundings, save for the cells right next to the snowmobile
void SnowmobilePuzzle::populateField() {
	getCurrentCell(_lastCellX, _lastCellY);
	if (_sets.empty()) {
		return;
	}

	for (int32 x = _lastCellX - _cellRadiusX; x <= _lastCellX + _cellRadiusX; ++x) {
		for (int32 y = _lastCellY - _cellRadiusY; y <= _lastCellY + _cellRadiusY; ++y) {
			if ((ABS(_lastCellX - x) > 1 || ABS(_lastCellY - y) > 1) && rollForCell(x, y)) {
				spawnSet(x, y);
			}
		}
	}
}

// Drops the sets left behind, and fills the cells that just came within range
void SnowmobilePuzzle::updateField() {
	int32 cellX, cellY;
	getCurrentCell(cellX, cellY);
	if (cellX == _lastCellX && cellY == _lastCellY) {
		return;
	}

	Common::Rect32 kept(cellX - _cellRadiusX, cellY - _cellRadiusY, cellX + _cellRadiusX + 1, cellY + _cellRadiusY + 1);
	for (uint i = _numFixedZones; i < _zones.size(); ) {
		if (kept.contains(_zones[i].cellX, _zones[i].cellY)) {
			++i;
		} else {
			_zones.remove_at(i);
		}
	}

	if (!_sets.empty()) {
		if (cellX != _lastCellX) {
			int32 x = cellX + (cellX < _lastCellX ? -_cellRadiusX : _cellRadiusX);
			for (int32 y = cellY - _cellRadiusY; y < cellY + _cellRadiusY; ++y) {
				if (!isCellExcluded(x, y) && rollForCell(x, y)) {
					spawnSet(x, y);
				}
			}
		}

		if (cellY != _lastCellY) {
			int32 y = cellY + (cellY < _lastCellY ? -_cellRadiusY : _cellRadiusY);
			for (int32 x = cellX - _cellRadiusX; x < cellX + _cellRadiusX; ++x) {
				if (!isCellExcluded(x, y) && rollForCell(x, y)) {
					spawnSet(x, y);
				}
			}
		}
	}

	_lastCellX = cellX;
	_lastCellY = cellY;
}

bool SnowmobilePuzzle::isCellExcluded(int32 cellX, int32 cellY) const {
	Common::Rect32 cell(cellX * _cellSize, cellY * _cellSize, (cellX + 1) * _cellSize, (cellY + 1) * _cellSize);
	for (uint i = 0; i < _numFixedZones; ++i) {
		if (_zones[i].type == kSnowZoneNoRandomZones && _zones[i].rect.intersects(cell)) {
			return true;
		}
	}

	return false;
}

bool SnowmobilePuzzle::rollForCell(int32 cellX, int32 cellY) const {
	bool result = (int32)g_nancy->_randomSource->getRandomNumber(99) < _density;

	// The chased snowmobile's path is kept clear
	int32 top, bottom;
	if (_variant == kChase && getPathRange(cellX * _cellSize, (cellX + 1) * _cellSize, top, bottom) &&
			(cellY - 1) * _cellSize <= bottom && top < (cellY + 2) * _cellSize) {
		return false;
	}

	return result;
}

void SnowmobilePuzzle::spawnSet(int32 cellX, int32 cellY) {
	int32 roll = g_nancy->_randomSource->getRandomNumber(99);
	const ZoneSet *set = nullptr;
	for (const ZoneSet &candidate : _sets) {
		if (roll < candidate.maxRoll) {
			set = &candidate;
			break;
		}
	}

	int32 x = cellX * _cellSize + (int32)g_nancy->_randomSource->getRandomNumber(_cellSize - 1);
	int32 y = cellY * _cellSize + (int32)g_nancy->_randomSource->getRandomNumber(_cellSize - 1);
	Math::Vector2d pos(x, y);

	if (!set) {
		return;
	}

	for (const Zone &source : set->zones) {
		Zone zone = source;
		zone.centerOn(pos);
		zone.cellX = cellX;
		zone.cellY = cellY;
		_zones.push_back(zone);
	}
}

double SnowmobilePuzzle::wrapAngle(double angle) {
	while (angle < 0) {
		angle += 2 * M_PI;
	}

	while (angle >= 2 * M_PI) {
		angle -= 2 * M_PI;
	}

	return angle;
}

// Angles go counterclockwise, with the y axis pointing down
double SnowmobilePuzzle::angleOf(const Math::Vector2d &v) {
	double angle = atan2(-v.getY(), v.getX());
	return angle < 0 ? angle + 2 * M_PI : angle;
}

void SnowmobilePuzzle::setSpeed(double speed) {
	speed = CLIP(speed, _car.minSpeed, _car.maxSpeed);
	_car.speed = fabs(speed) < kMinSpeed ? 0 : speed;
}

void SnowmobilePuzzle::steer(int32 ticks) {
	if (_inputFlags & NancyInput::kMoveLeft) {
		_car.heading = wrapAngle(_car.heading + kTurnPerTick * ticks);
	} else if (_inputFlags & NancyInput::kMoveRight) {
		_car.heading = wrapAngle(_car.heading - kTurnPerTick * ticks);
	}

	double acceleration = _car.maxSpeed / kTicksPerSecond * ticks;
	if (_inputFlags & (NancyInput::kMoveUp | NancyInput::kLeftMouseButtonDown | NancyInput::kLeftMouseButtonHeld)) {
		setSpeed(_car.speed + acceleration);
	} else if (_inputFlags & (NancyInput::kMoveDown | NancyInput::kRightMouseButtonDown | NancyInput::kRightMouseButtonHeld)) {
		setSpeed(_car.speed - acceleration / kBrakeFactor);
	}
}

// Friction never reverses the direction of travel
void SnowmobilePuzzle::applyFriction(double ticks) {
	if (fabs(_car.speed) < 0.00001) {
		setSpeed(0);
		return;
	}

	bool wasReversing = _car.speed < 0;
	double change = _car.friction * ticks;
	setSpeed(_car.speed + (wasReversing ? change : -change));
	if ((_car.speed < 0) != wasReversing) {
		setSpeed(0);
	}
}

// Moves in steps of at most a pixel, so nothing gets jumped over
void SnowmobilePuzzle::move(double ticks) {
	Math::Vector2d delta((float)(cos(_car.heading) * _car.speed / kTicksPerSecond * ticks),
		(float)(-sin(_car.heading) * _car.speed / kTicksPerSecond * ticks));

	if (fabs(delta.getX()) > 1.0 || fabs(delta.getY()) > 1.0) {
		move(ticks * 0.5);
		move(ticks * 0.5);
		return;
	}

	tryMove(_car.pos + delta);
}

bool SnowmobilePuzzle::isFree() const {
	Common::Rect32 bounds = _car.getBounds();
	for (const Zone &zone : _zones) {
		if (zone.type == kSnowZoneBoundary && zone.isActive && zone.rect.intersects(bounds)) {
			return false;
		}
	}

	return true;
}

// Each axis is tried on its own, which lets the snowmobile slide along boundaries
void SnowmobilePuzzle::tryMove(const Math::Vector2d &pos) {
	Math::Vector2d oldPos = _car.pos;
	Math::Vector2d newPos = oldPos;

	_car.pos = Math::Vector2d(pos.getX(), oldPos.getY());
	bool movedX = false;
	if (isFree()) {
		newPos.setX(pos.getX());
		movedX = pos.getX() != oldPos.getX();
	}

	_car.pos = Math::Vector2d(oldPos.getX(), pos.getY());
	bool movedY = false;
	if (isFree()) {
		newPos.setY(pos.getY());
		movedY = pos.getY() != oldPos.getY();
	}

	if (!movedX && !movedY) {
		_car.speed = 0;
	}

	_car.pos = newPos;
	if (!isFree()) {
		_car.speed = 0;
		_car.pos = oldPos;
	}
}

void SnowmobilePuzzle::update(int32 ticks) {
	steer(ticks);
	applyFriction(ticks);
	move(ticks);
	updateField();

	if (_car.damage >= kMaxDamage) {
		_car.damage = 0;
		_hasCrashed = true;
		playSoundBlock(_crashSound);
		changeScene(_crashScene);
	}

	if (_hasCrashed || NancySceneState.getUIResource(_resourceID) == 0) {
		_zonesEnabled = false;
	}
}

bool SnowmobilePuzzle::isInZone(const Zone &zone) const {
	if (zone.conditionFlag != -1 && !NancySceneState.getEventFlag(zone.conditionFlag, zone.conditionValue)) {
		return false;
	}

	double speed = fabs(_car.speed);
	if (speed < zone.minSpeed || speed > zone.maxSpeed) {
		return false;
	}

	int32 x = (int32)_car.pos.getX();
	int32 y = (int32)_car.pos.getY();

	switch (zone.type) {
	case kSnowZoneBoundary:
		return true;
	case kSnowZonePointer:
		// Shown while the snowmobile is away from what the arrow points at
		return !zone.rect.contains(x, y);
	case kSnowZoneTiledOverlay:
		if (zone.hasImplicitRect) {
			return y > zone.rect.top && y < zone.rect.bottom;
		}

		// fall through
	default:
		return zone.rect.contains(x, y);
	}
}

// The zones the snowmobile starts in only take effect once they are left and entered again
void SnowmobilePuzzle::seedZones() {
	for (Zone &zone : _zones) {
		switch (zone.type) {
		case kSnowZoneChangeScene:
		case kSnowZoneSound:
		case kSnowZoneFlag:
		case kSnowZoneDamage:
		case kSnowZoneNoRandomZones:
			zone.isActive = isInZone(zone);
			break;
		default:
			break;
		}
	}
}

void SnowmobilePuzzle::updateZones() {
	for (Zone &zone : _zones) {
		bool isInside = isInZone(zone);
		if (isInside && !zone.isActive) {
			zone.isActive = true;
			playSoundBlock(zone.sound);
			enterZone(zone);
		} else if (!isInside) {
			zone.isActive = false;
		}
	}
}

void SnowmobilePuzzle::enterZone(Zone &zone) {
	switch (zone.type) {
	case kSnowZoneChangeScene:
		changeScene(zone.sceneID);
		break;
	case kSnowZoneFlag:
		NancySceneState.setEventFlag(zone.flagLabel, zone.flagValue ? g_nancy->_true : g_nancy->_false);
		break;
	case kSnowZoneDamage:
		_car.damage += zone.minDamage;
		if (zone.maxDamage > zone.minDamage) {
			_car.damage += g_nancy->_randomSource->getRandomNumber(zone.maxDamage - zone.minDamage - 1);
		}

		break;
	default:
		break;
	}
}

// Returns the last waypoint that is not past the given time within a lap
uint SnowmobilePuzzle::findWaypoint(uint32 time) const {
	uint low = 0;
	uint high = _path.size();
	while (low < high) {
		uint mid = (low + high) / 2;
		if (_path[mid].time < time) {
			low = mid + 1;
		} else {
			high = mid;
		}
	}

	if (low == _path.size() || _path[low].time > time) {
		low = low == 0 ? _path.size() - 1 : low - 1;
	}

	return low;
}

Math::Vector2d SnowmobilePuzzle::getPathPos(uint32 time) const {
	uint32 lap = time / _pathDuration;
	uint32 lapTime = time % _pathDuration;

	uint from = findWaypoint(lapTime);
	uint to = from + 1;
	bool wraps = to == _path.size();
	if (wraps) {
		to = 0;
	}

	int32 span = (int32)(_path[to].time - _path[from].time) + (wraps ? (int32)_pathDuration : 0);
	double fraction = span != 0 ? (double)(int32)(lapTime - _path[from].time) / span : 0;

	Math::Vector2d fromPos = _path[from].pos + _pathLap * lap;
	Math::Vector2d toPos = _path[to].pos + _pathLap * (lap + (wraps ? 1 : 0));
	return fromPos + (toPos - fromPos) * (float)fraction;
}

// Finds the vertical extent of the path between two x coordinates
bool SnowmobilePuzzle::getPathRange(int32 left, int32 right, int32 &top, int32 &bottom) const {
	top = 0x7fffffff;
	bottom = -0x7fffffff - 1;

	Math::Vector2d offset = _pathLap * (int32)((left - _path[0].pos.getX()) / _pathLap.getX());
	bool isInside = false;
	for (const Waypoint &point : _path) {
		double x = point.pos.getX() + offset.getX();
		if (!isInside) {
			if (x < left) {
				continue;
			}

			isInside = true;
		}

		double y = point.pos.getY() + offset.getY();
		if (y < top) {
			top = (int32)y;
		}

		if (y > bottom) {
			bottom = (int32)y;
		}

		if (right <= x) {
			return true;
		}
	}

	return false;
}

// The chased snowmobile waits for the player when ahead, and hurries when caught up with
void SnowmobilePuzzle::updateChase(int32 ticks) {
	ChaseStage &stage = _stages[_curStage];
	int32 nextDistance = _curStage + 1 < _stages.size() ? _stages[_curStage + 1].distance : (int32)kChaseCloseDistance;

	Math::Vector2d offset = _chaserPos - _car.pos;
	double distance = offset.getMagnitude();

	double rate = 1.0;
	if (offset.getX() < 0) {
		rate = kChaseRateBehind;
	} else if (offset.getX() < kChaseCloseDistance) {
		rate = 1.0 + (kChaseCloseDistance - offset.getX()) / kChaseCloseDistance;
	} else if (nextDistance > distance) {
		rate = kChaseRateInReach;
	} else if (offset.getX() > _drawSurface.w) {
		rate = kChaseRateOutOfView;
	} else if (offset.getX() > _drawSurface.w / 2) {
		rate = kChaseRateFar;
	}

	_pathTime = (uint32)(ticks * (double)kMsPerTick * rate + _pathTime);
	_chaserPos = getPathPos(_pathTime);

	uint32 lapTime = _pathTime % _pathDuration;
	uint waypoint = findWaypoint(lapTime);
	if (_path[waypoint].time < lapTime && waypoint + 1 < _path.size()) {
		++waypoint;
	}

	_chaserHeading = wrapAngle(_path[waypoint].heading);

	if (_pointerZone != -1) {
		_zones[_pointerZone].centerOn(_chaserPos);
	}

	if (updateStage(stage, distance) && ++_curStage == _stages.size()) {
		changeScene(_winScene);
		NancySceneState.setEventFlag(_winFlag, g_nancy->_true);
	}
}

// Returns true once the player has stayed within reach for long enough
bool SnowmobilePuzzle::updateStage(ChaseStage &stage, double distance) {
	if (stage.distance <= distance) {
		if (stage.isInReach) {
			stage.isInReach = false;
			NancySceneState.setEventFlag(stage.inReachFlag, g_nancy->_false);
		}

		return false;
	}

	uint32 now = g_nancy->getTotalPlayTime();
	if (!stage.isInReach) {
		NancySceneState.setEventFlag(stage.reachedFlag, g_nancy->_true);
		stage.reachedFlag = -1;
		NancySceneState.setEventFlag(stage.inReachFlag, g_nancy->_true);
		stage.isInReach = true;
		stage.reachedTime = now;
	}

	return now - stage.reachedTime >= stage.duration;
}

void SnowmobilePuzzle::changeScene(uint16 sceneID) {
	if (_nextScene == kNoScene) {
		_nextScene = sceneID;
	}
}

Common::Rect32 SnowmobilePuzzle::getView() const {
	int32 left = (int32)(_car.pos.getX() - _drawSurface.w / 2);
	int32 top = (int32)(_car.pos.getY() - _drawSurface.h / 2);
	return Common::Rect32(left, top, left + _drawSurface.w, top + _drawSurface.h);
}

// The player's snowmobile always sits at the center of the view
Common::Rect SnowmobilePuzzle::getCarDest(const Common::Rect32 &frame) const {
	int16 left = _drawSurface.w / 2 - frame.width() / 2;
	int16 top = _drawSurface.h / 2 - frame.height() / 2;
	return Common::Rect(left, top, left + frame.width(), top + frame.height());
}

void SnowmobilePuzzle::blendPixel(int x, int y, const Graphics::ManagedSurface &image, uint32 color) {
	byte srcA, srcR, srcG, srcB;
	image.format.colorToARGB(color, srcA, srcR, srcG, srcB);
	if (srcA == 0) {
		return;
	}

	if (srcA != 255) {
		byte dstA, dstR, dstG, dstB;
		_drawSurface.format.colorToARGB(_drawSurface.getPixel(x, y), dstA, dstR, dstG, dstB);

		uint dstWeight = dstA * (255 - srcA) / 255;
		uint outA = srcA + dstWeight;
		srcR = (srcR * srcA + dstR * dstWeight) / outA;
		srcG = (srcG * srcA + dstG * dstWeight) / outA;
		srcB = (srcB * srcA + dstB * dstWeight) / outA;
		srcA = outA;
	}

	_drawSurface.setPixel(x, y, _drawSurface.format.ARGBToColor(srcA, srcR, srcG, srcB));
}

void SnowmobilePuzzle::drawSprite(const Graphics::ManagedSurface &image, const Common::Rect32 &src, int32 destX, int32 destY) {
	int32 startX = MAX<int32>(0, -destX);
	int32 startY = MAX<int32>(0, -destY);
	int32 endX = MIN<int32>(src.width(), _drawSurface.w - destX);
	int32 endY = MIN<int32>(src.height(), _drawSurface.h - destY);

	for (int32 y = startY; y < endY; ++y) {
		for (int32 x = startX; x < endX; ++x) {
			blendPixel(destX + x, destY + y, image, image.getPixel(src.left + x, src.top + y));
		}
	}
}

// Draws the sprite turned counterclockwise around the center of dest
void SnowmobilePuzzle::drawRotated(const Graphics::ManagedSurface &image, const Common::Rect32 &src, const Common::Rect &dest, double angle) {
	double cosA = cos(angle);
	double sinA = sin(angle);
	double halfW = src.width() * 0.5;
	double halfH = src.height() * 0.5;
	double centerX = dest.left + halfW;
	double centerY = dest.top + halfH;
	int32 radius = (int32)ceil(sqrt(halfW * halfW + halfH * halfH));

	int32 startX = MAX<int32>(0, (int32)centerX - radius);
	int32 startY = MAX<int32>(0, (int32)centerY - radius);
	int32 endX = MIN<int32>(_drawSurface.w, (int32)centerX + radius + 1);
	int32 endY = MIN<int32>(_drawSurface.h, (int32)centerY + radius + 1);

	for (int32 y = startY; y < endY; ++y) {
		for (int32 x = startX; x < endX; ++x) {
			double dx = x + 0.5 - centerX;
			double dy = y + 0.5 - centerY;
			int32 srcX = (int32)floor(dx * cosA - dy * sinA + halfW);
			int32 srcY = (int32)floor(dx * sinA + dy * cosA + halfH);
			if (srcX >= 0 && srcX < src.width() && srcY >= 0 && srcY < src.height()) {
				blendPixel(x, y, image, image.getPixel(src.left + srcX, src.top + srcY));
			}
		}
	}
}

void SnowmobilePuzzle::drawZones(int32 layer) {
	Common::Rect32 view = getView();

	for (const Zone &zone : _zones) {
		if (!zone.isActive || zone.layer != layer) {
			continue;
		}

		const Graphics::ManagedSurface &image = *_sheets[zone.imageID];

		switch (zone.type) {
		case kSnowZoneOverlay:
			if (zone.dest.intersects(view)) {
				drawSprite(image, zone.src, zone.dest.left - view.left, zone.dest.top - view.top);
			}

			break;
		case kSnowZoneTiledOverlay: {
			int32 step = zone.dest.width();
			if (step <= 0) {
				break;
			}

			int32 x = (zone.dest.left - view.left) % step;
			if (x > 0) {
				x -= step;
			}

			for (; x < _drawSurface.w; x += step) {
				drawSprite(image, zone.src, x, zone.dest.top - view.top);
			}

			break;
		}
		case kSnowZonePointer: {
			Common::Rect dest(zone.dest.left, zone.dest.top, zone.dest.right, zone.dest.bottom);
			drawRotated(image, zone.src, dest, angleOf(zone.target - _car.pos));
			break;
		}
		default:
			break;
		}
	}
}

void SnowmobilePuzzle::redraw() {
	_drawSurface.clear(0);

	drawZones(0);

	Common::Rect carDest = getCarDest(_carSrc);

	if (_variant == kChase) {
		Math::Vector2d offset = _chaserPos - _car.pos;
		int32 left = carDest.left + (int32)offset.getX();
		int32 top = carDest.top + (int32)offset.getY();
		Common::Rect32 chaserDest(left, top, left + _chaserSrc.width(), top + _chaserSrc.height());
		if (chaserDest.intersects(Common::Rect32(0, 0, _drawSurface.w, _drawSurface.h))) {
			drawRotated(*_sheets[_chaserImageID], _chaserSrc,
				Common::Rect(chaserDest.left, chaserDest.top, chaserDest.right, chaserDest.bottom), _chaserHeading);
		}
	}

	drawRotated(_image, _carSrc, carDest, _car.heading);

	drawZones(1);

	_needsRedraw = true;
}

void SnowmobilePuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		int32 ticks = (now - _lastTickTime) / kMsPerTick;
		_lastTickTime += ticks * kMsPerTick;

		if (ticks > 0) {
			update(ticks);
			if (_variant == kChase) {
				updateChase(ticks);
			}
		}

		if (_zonesEnabled) {
			updateZones();
		}

		redraw();

		if (_nextScene != kNoScene) {
			_state = kActionTrigger;
		}

		break;
	}
	case kActionTrigger: {
		if (!_engineSoundDesc.name.empty()) {
			g_nancy->_sound->stopSound(_engineSoundDesc);
		}

		SceneChangeDescription sceneChange;
		sceneChange.sceneID = _nextScene;
		sceneChange.continueSceneSound = kContinueSceneSound;
		NancySceneState.changeScene(sceneChange);
		finishExecution();
		break;
	}
	}
}

void SnowmobilePuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	_inputFlags = input.input;

	// Cheat: Ctrl+Shift+H repairs the snowmobile, undoing the damage of the obstacles hit so far
	for (const Common::KeyState &key : input.otherKbdInput) {
		if ((key.flags & Common::KBD_CTRL) && (key.flags & Common::KBD_SHIFT) && key.keycode == Common::KEYCODE_h) {
			_car.damage = 0;
			debug("Snowmobile cheat: damage repaired");
		}

		// Cheat: Ctrl+Shift+W wins the chase, as if every stage of it was done
		if (_variant == kChase && (key.flags & Common::KBD_CTRL) && (key.flags & Common::KBD_SHIFT) &&
				key.keycode == Common::KEYCODE_w) {
			changeScene(_winScene);
			NancySceneState.setEventFlag(_winFlag, g_nancy->_true);
			debug("Snowmobile cheat: chase won");
		}
	}

	// The arrow keys take over the steering until the mouse is moved again
	if (_inputFlags & (NancyInput::kMoveUp | NancyInput::kMoveDown | NancyInput::kMoveLeft | NancyInput::kMoveRight)) {
		_isMouseSteering = false;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	if (input.mousePos != _lastMousePos) {
		_lastMousePos = input.mousePos;
		_isMouseSteering = true;

		// The snowmobile faces the cursor
		Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
			Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
		Common::Rect carDest = getCarDest(_carSrc);
		Math::Vector2d direction(mouseRect.left - (carDest.left + carDest.width() / 2),
			mouseRect.top - (carDest.top + carDest.height() / 2));
		_car.heading = angleOf(direction);
	}

	setDataCursor(_isMouseSteering ? kMouseSteeringCursor : kKeySteeringCursor, false);
}

} // End of namespace Action
} // End of namespace Nancy
