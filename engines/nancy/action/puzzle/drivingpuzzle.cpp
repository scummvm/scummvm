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

#include "common/system.h"
#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/enginedata.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"
#include "engines/nancy/puzzledata.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/drivingpuzzle.h"

namespace Nancy {
namespace Action {

void DrivingPuzzle::readFrameRects(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &out) {
	int16 count = stream.readSint16LE();
	if (count <= 0) {
		return;
	}

	out.resize(count);
	for (int i = 0; i < count; ++i) {
		readRect(stream, out[i]);
	}
}

void DrivingPuzzle::readWaypoints(Common::SeekableReadStream &stream, Common::Array<Waypoint> &out) {
	int16 count = stream.readSint16LE();
	if (count <= 0) {
		return;
	}

	out.resize(count);
	for (int i = 0; i < count; ++i) {
		out[i].timeMs = stream.readUint32LE();
		out[i].x = stream.readSint16LE();
		out[i].y = stream.readSint16LE();
		out[i].heading = stream.readDoubleLE();
	}
}

void DrivingPuzzle::readBlob(Common::SeekableReadStream &stream) {
	// 130-byte PuzzleBase header blob: three 33-byte filenames followed by the car's
	// physics parameters (two short padding gaps separate some of the tail fields).
	readFilename(stream, _imageName);		// blob 0x00: visible town map
	readFilename(stream, _collisionName);	// blob 0x21: collision mask
	readFilename(stream, _carSpriteName);	// blob 0x42: car rotation atlas

	_startX = stream.readSint32LE();		// 0x63
	_startY = stream.readSint32LE();		// 0x67
	_startAngle = stream.readSint32LE();	// 0x6b
	_forwardSpeed = stream.readSint32LE();	// 0x6f
	_reverseSpeed = stream.readSint32LE();	// 0x73
	_frictionIndex = stream.readSint16LE();	// 0x77
	stream.skip(2);
	_distanceDivisor = stream.readSint32LE();	// 0x7b
	_retainState = stream.readByte() != 0;		// 0x7f
	_finishScene = stream.readUint16LE();		// 0x80: the scene to enter when a tire goes flat
}

void DrivingPuzzle::readData(Common::SeekableReadStream &stream) {
	// 160 (kDriving) / 167 (kChase): a 130-byte PuzzleBase header blob followed by
	// three random-sound blocks and a rotation-frame rect table.
	readBlob(stream);

	for (int i = 0; i < 3; ++i) {
		_soundBlocks[i].readData(stream);
	}

	readFrameRects(stream, _frameRects);

	if (_variant == kDriving) {
		readActionZoneArray(stream, _zones);
		return;
	}

	// 167 (kChase): five id/scene values, a second (chaser) car sprite name and a
	// second rotation-frame table, then two ActionZone arrays and two recorded
	// chaser paths.
	for (int i = 0; i < 5; ++i) {
		_chaseParams[i] = stream.readSint16LE();
	}

	readFilename(stream, _chaseCarImageName);
	readFrameRects(stream, _frameRects2);

	readActionZoneArray(stream, _zones);
	readActionZoneArray(stream, _zones2);

	readWaypoints(stream, _chaserPathA);
	readWaypoints(stream, _chaserPathB);
}

void DrivingPuzzle::classifyZones(const Common::Array<ActionZone> &zones) {
	// The car often starts already sitting inside a location's zone (its start position,
	// or a restored parking spot), so zones are edge-triggered: a zone only fires once the
	// car has left it and driven back in. Seed each zone's "inside" state from where the
	// car currently is (init() has already restored any saved position).
	Common::Point spawn((int)(_carX + 0.5), (int)(_carY + 0.5));

	for (uint i = 0; i < zones.size(); ++i) {
		const ActionZone &z = zones[i];
		switch (z.type) {
		case kZoneDestination:		// location entrance
		case kZoneSceneChange: {	// drive-in scene trigger (chase)
			DestinationZone dest;
			dest.rect = z.rect;
			dest.scene.sceneID = z.specialEffectId;
			if (z.hasSpecialEffect) {
				dest.hasFade = true;
				dest.fadeType = z.seType;
				dest.fadeTotalTime = z.seTotalTime;
				dest.fadeToBlackTime = z.seFadeToBlackTime;
				dest.fadeRect = z.seRect;
			}
			if (z.type == kZoneSceneChange) {
				dest.eventFlag = z.tailId;
				dest.eventFlagValue = z.tailFlag;
				dest.autoTrigger = true;	// the chase finish line fires on drive-in, no spacebar
			}
			dest.carInside = dest.rect.contains(spawn);
			_destinations.push_back(dest);
			break;
		}
		case kZoneEventFlag: {	// checkpoint: sets an event flag when driven into while armed
			Checkpoint cp;
			cp.rect = z.rect;
			cp.flagId = z.tailId;
			cp.flagValue = z.tailFlag;
			cp.condFlag = z.val49;
			cp.condValue = z.val4b;
			bool cond = cp.condFlag == -1 || NancySceneState.getEventFlag(cp.condFlag, cp.condValue);
			cp.wasActive = cond && cp.rect.contains(spawn);
			_checkpoints.push_back(cp);
			break;
		}
		case kZoneTerrain: {	// mud puddle: slows the car while inside
			MudZone mud;
			mud.rect = z.rect;
			mud.decel = z.terrainDecel;
			_mudZones.push_back(mud);
			break;
		}
		case kZoneFlatTire: {	// pothole: damages the tires on entry
			Pothole hole;
			hole.rect = z.rect;
			hole.minDamage = z.flatTireMin;
			hole.maxDamage = z.flatTireMax;
			hole.carInside = hole.rect.contains(spawn);
			_potholes.push_back(hole);
			break;
		}
		case kZoneOverlay: {	// cosmetic map decoration (buildings, cars, potholes, animations)
			if (z.overlayName.empty() || z.overlaySrcRects.empty() || z.overlayDestRect.isEmpty()) {
				break;
			}
			Overlay ov;
			ov.imageIndex = overlayImageIndex(z.overlayName);
			ov.srcRects = z.overlaySrcRects;
			ov.destRect = z.overlayDestRect;
			ov.condFlag = z.val49;
			ov.condValue = z.val4b;
			ov.aboveCar = z.overlayLayer != 0;
			if (ov.imageIndex >= 0) {
				_overlays.push_back(ov);
			}
			break;
		}
		case kZoneBoundary: {	// flag-gated road obstacle (a cow blocking the road, etc.)
			Obstacle obs;
			obs.rect = z.rect;
			obs.condFlag = z.val49;
			obs.condValue = z.val4b;
			_obstacles.push_back(obs);
			break;
		}
		default:
			// The remaining subtypes (terrain markers) are not simulated yet.
			break;
		}
	}
}

void DrivingPuzzle::playSoundBlock(const RandomSoundBlock &block) {
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
	desc.numLoops = block.numLoops;	// 0 == loop forever (the engine ambience)
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
}

void DrivingPuzzle::armExit(const DestinationZone &dest) {
	_exitScene = dest.scene;
	_exitHasFade = dest.hasFade;
	_exitFadeType = dest.fadeType;
	_exitFadeTotalTime = dest.fadeTotalTime;
	_exitFadeToBlackTime = dest.fadeToBlackTime;
	_exitFadeRect = dest.fadeRect;
	_exitFlag = dest.eventFlag;
	_exitFlagValue = dest.eventFlagValue;
	_state = kActionTrigger;
}

void DrivingPuzzle::armExitScene(uint16 sceneID, int16 flag, byte flagValue) {
	_exitScene = SceneChangeDescription();
	_exitScene.sceneID = sceneID;
	_exitHasFade = false;
	_exitFlag = flag;
	_exitFlagValue = flagValue;
	_state = kActionTrigger;
}

void DrivingPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);

	g_nancy->_resource->loadImage(_carSpriteName, _carImage);
	_carImage.setTransparentColor(_drawSurface.getTransparentColor());

	if (_variant == kChase && !_chaseCarImageName.empty()) {
		g_nancy->_resource->loadImage(_chaseCarImageName, _chaseCarImage);
		_chaseCarImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	// The collision mask marks the drivable road in white; everything else is off-road.
	g_nancy->_resource->loadImage(_collisionName, _collisionMask);

	// Seed the car from the physics parameters decoded from the header blob.
	_carX = _startX;
	_carY = _startY;
	_carHeading = (double)_startAngle * (M_PI / 180.0);
	_carVelocity = 0.0;
	_speedCap = _forwardSpeed;
	_lastPhysicsMs = 0;

	// When the map keeps its state (retainState), resume from where the car was left the
	// last time this map was driven (i.e. before entering a building), and keep the tire
	// damage. The first ever visit starts from the header position.
	if (_retainState) {
		DrivingData *data = (DrivingData *)NancySceneState.getPuzzleData(DrivingData::getTag());
		if (data && data->valid) {
			_carX = data->carX;
			_carY = data->carY;
			_carHeading = data->heading;
			_tireDamage = data->tireDamage;
			_fuelBurnAccum = data->fuelBurnAccum;
			_infiniteFuel = data->infiniteFuel;
		}
	}

	// Seed the chaser at the start of its recorded path.
	if (!_chaserPathA.empty()) {
		_chaserX = _chaserPathA[0].x;
		_chaserY = _chaserPathA[0].y;
		_chaserHeading = _chaserPathA[0].heading;
	}
}

uint DrivingPuzzle::frameIndexForHeading(double heading, uint frameCount) const {
	if (frameCount == 0) {
		return 0;
	}

	// Quantize the heading evenly across the atlas frames (approximate).
	while (heading < 0.0) {
		heading += 2.0 * M_PI;
	}
	while (heading >= 2.0 * M_PI) {
		heading -= 2.0 * M_PI;
	}

	uint idx = (uint)(heading / (2.0 * M_PI) * frameCount + 0.5);
	return idx % frameCount;
}

Common::Point DrivingPuzzle::cameraOffset() const {
	// Car-centered camera, clamped to the map bounds.
	int camX = CLIP<int>((int)(_carX + 0.5) - _drawSurface.w / 2, 0, MAX(0, _image.w - _drawSurface.w));
	int camY = CLIP<int>((int)(_carY + 0.5) - _drawSurface.h / 2, 0, MAX(0, _image.h - _drawSurface.h));
	return Common::Point(camX, camY);
}

int DrivingPuzzle::overlayImageIndex(const Common::String &name) {
	for (uint i = 0; i < _overlayImageNames.size(); ++i) {
		if (_overlayImageNames[i] == name) {
			return (int)i;
		}
	}

	Graphics::ManagedSurface surf;
	g_nancy->_resource->loadImage(Common::Path(name), surf);
	if (surf.empty()) {
		return -1;
	}
	surf.setTransparentColor(_drawSurface.getTransparentColor());

	_overlayImages.push_back(Common::move(surf));
	_overlayImageNames.push_back(name);
	return (int)_overlayImages.size() - 1;
}

void DrivingPuzzle::drawOverlays(const Common::Point &cam, bool aboveCar) {
	if (_overlays.empty()) {
		return;
	}

	const int frameMs = 66;		// decoration animation speed (0x42 ms/frame)
	uint32 nowMs = g_system->getMillis();
	Common::Rect view(cam.x, cam.y, cam.x + _drawSurface.w, cam.y + _drawSurface.h);

	for (uint i = 0; i < _overlays.size(); ++i) {
		const Overlay &ov = _overlays[i];
		if (ov.aboveCar != aboveCar) {
			continue;
		}
		if (ov.imageIndex < 0 || !view.intersects(ov.destRect)) {
			continue;
		}

		// Only draw the decoration while its event-flag condition holds (a car appears
		// once its story flag is set, etc.).
		if (ov.condFlag != -1 && !NancySceneState.getEventFlag(ov.condFlag, ov.condValue)) {
			continue;
		}

		uint frame = ov.srcRects.size() == 1 ? 0 : (nowMs / frameMs) % ov.srcRects.size();
		_drawSurface.blitFrom(_overlayImages[ov.imageIndex], ov.srcRects[frame],
			Common::Point(ov.destRect.left - cam.x, ov.destRect.top - cam.y));
	}
}

void DrivingPuzzle::saveState() const {
	if (!_retainState) {
		return;
	}

	DrivingData *data = (DrivingData *)NancySceneState.getPuzzleData(DrivingData::getTag());
	if (data) {
		data->valid = true;
		data->carX = (int32)(_carX + 0.5);
		data->carY = (int32)(_carY + 0.5);
		data->heading = _carHeading;
		data->tireDamage = _tireDamage;
		data->fuelBurnAccum = _fuelBurnAccum;
		data->infiniteFuel = _infiniteFuel;
	}
}

void DrivingPuzzle::refillFuel() {
	const UIRC *uirc = GetEngineData(UIRC)
	if (uirc && _frictionIndex >= 0 && (uint)_frictionIndex < uirc->items.size()) {
		NancySceneState.setUIResource(_frictionIndex, uirc->items[_frictionIndex].id);
		_fuelBurnAccum = 0.0;
	}
}

void DrivingPuzzle::repairTire() {
	_tireDamage = 0;
	const UIRC *uirc = GetEngineData(UIRC)
	if (uirc && kTireResourceIndex < uirc->items.size()) {
		NancySceneState.setUIResource(kTireResourceIndex, uirc->items[kTireResourceIndex].id);
	}
}

bool DrivingPuzzle::isWall(int px, int py) const {
	if (px < 0 || py < 0 || px >= _collisionMask.w || py >= _collisionMask.h) {
		return true;	// off the map
	}

	// The road is white; anything darker is off-road.
	byte r, g, b;
	_collisionMask.format.colorToRGB(_collisionMask.getPixel(px, py), r, g, b);
	return r < 128 || g < 128 || b < 128;
}

bool DrivingPuzzle::isBlocked(const Common::Point &p) const {
	if (isWall(p.x, p.y)) {
		return true;
	}

	// A cow (or similar) is blocking the road while its story flag is set.
	for (uint i = 0; i < _obstacles.size(); ++i) {
		const Obstacle &obs = _obstacles[i];
		if (obs.rect.contains(p) &&
			(obs.condFlag == -1 || NancySceneState.getEventFlag(obs.condFlag, obs.condValue))) {
			return true;
		}
	}

	return false;
}

void DrivingPuzzle::drawScene() {
	Common::Point cam = cameraOffset();
	int camX = cam.x;
	int camY = cam.y;

	_drawSurface.blitFrom(_image, Common::Rect(camX, camY, camX + _drawSurface.w, camY + _drawSurface.h), Common::Point(0, 0));

	// Ground-level decorations (potholes, parked cars, cows, fountain) lie on the road,
	// under the cars.
	drawOverlays(cam, false);

	// The chaser car (kChase), drawn under the player car.
	if (_variant == kChase && !_frameRects2.empty() && _chaseCarImage.w > 0) {
		const Common::Rect &src = _frameRects2[frameIndexForHeading(_chaserHeading, _frameRects2.size())];
		int sx = (int)(_chaserX + 0.5) - camX - src.width() / 2;
		int sy = (int)(_chaserY + 0.5) - camY - src.height() / 2;
		_drawSurface.blitFrom(_chaseCarImage, src, Common::Point(sx, sy));
	}

	// The player car sprite frame, centered on its on-screen position.
	if (!_frameRects.empty()) {
		const Common::Rect &src = _frameRects[frameIndexForHeading(_carHeading, _frameRects.size())];
		int sx = (int)(_carX + 0.5) - camX - src.width() / 2;
		int sy = (int)(_carY + 0.5) - camY - src.height() / 2;
		_drawSurface.blitFrom(_carImage, src, Common::Point(sx, sy));
	}

	// Tall decorations (buildings, trees, power-line poles, flags) draw over the cars,
	// so the car passes behind them.
	drawOverlays(cam, true);

	_needsRedraw = true;
}

void DrivingPuzzle::updateChaser() {
	const Common::Array<Waypoint> &path = _chaserOnPathB ? _chaserPathB : _chaserPathA;
	if (path.empty()) {
		return;
	}

	if (!_chaseStarted) {
		_chaseStartTime = g_system->getMillis();
		_chaseStarted = true;
		_chaserWaypoint = 0;
	}

	// Play the recorded path back in real time: advance to the last waypoint whose
	// timestamp the elapsed chase time has passed.
	uint32 elapsed = g_system->getMillis() - _chaseStartTime;
	while (_chaserWaypoint + 1 < path.size() && path[_chaserWaypoint].timeMs < elapsed) {
		++_chaserWaypoint;
	}

	const Waypoint &wp = path[_chaserWaypoint];
	_chaserX = wp.x;
	_chaserY = wp.y;
	_chaserHeading = wp.heading;

	// The closer the chaser is, the lower the player's speed cap (it bottoms out at a
	// full stop once the chaser is right on top of the car).
	double dx = _carX - _chaserX;
	double dy = _carY - _chaserY;
	double dist = sqrt(dx * dx + dy * dy);
	const double slowRadius = 60.0;
	const double slowSlope = 5.0;
	_speedCap = dist >= slowRadius ? (double)_forwardSpeed : (double)_forwardSpeed - (slowRadius - dist) * slowSlope;

	// Losing Jane off the edge of the map is only a loss during the main pursuit, when the
	// player must stay right behind her. Once a checkpoint clears the pursuit gate, the chase
	// enters the shortcut phase: Nancy deliberately lets Jane go and races her to the state
	// line by another road, so the chaser leaving the view is expected.
	if (_chaseState == kPursuit) {
		Common::Point cam = cameraOffset();
		Common::Rect viewport(0, 0, _drawSurface.w, _drawSurface.h);
		Common::Rect chaserRect(1, 1);
		if (!_frameRects2.empty()) {
			chaserRect = _frameRects2[0];
		}
		chaserRect.moveTo((int)(_chaserX + 0.5) - cam.x - chaserRect.width() / 2,
			(int)(_chaserY + 0.5) - cam.y - chaserRect.height() / 2);
		if (!chaserRect.intersects(viewport)) {
			armExitScene(_chaseParams[kChaseOffViewScene], _chaseParams[kChaseOffViewFlag], 1);
			return;
		}
	}

	// Chase state machine (mirrors the original): flags gating it are set by the chase's
	// own checkpoints (zones2 type 0x0b) and by the scene scripts.
	switch (_chaseState) {
	case kPursuit:
		if (_chaseParams[kChaseGate01Flag] != -1 &&
				NancySceneState.getEventFlag(_chaseParams[kChaseGate01Flag], g_nancy->_false)) {
			_chaseState = kShortcut;
		}
		break;
	case kShortcut:
		// Switch onto the second path once Jane is caught.
		if (_chaseParams[kChaseGate12Flag] != -1 &&
				NancySceneState.getEventFlag(_chaseParams[kChaseGate12Flag], g_nancy->_true)) {
			_chaserOnPathB = true;
			_chaserWaypoint = 0;
			_chaseStarted = false;
			_chaseState = kCaught;
		}
		break;
	case kCaught:
		// The chaser has completed its route (Jane crashes) - the win.
		if (_chaserWaypoint + 1 >= path.size()) {
			armExitScene(_chaseParams[kChasePathEndScene], -1, 0);
			return;
		}
		break;
	default:
		break;
	}
}

// Per-frame car physics. Throttle is +1 (forward), -1 (reverse) or 0 (coast); the car
// already faces the cursor (steering happens in handleInput). cursorDist is how far the
// cursor is from the car, which sets the forward speed. Velocity is in pixels per second
// and integrated against the real elapsed time, so the car covers the same distance per
// second at any frame rate - the chaser plays back in real time, so a frame-rate-dependent
// car speed made the chase unwinnable.
void DrivingPuzzle::updatePhysics(int throttle, double cursorDist) {
	const double refStep = 0.02;			// the original's nominal per-frame step
	const double decay = 0.98;				// coast decay per refStep
	const int kTireFlatThreshold = 100;		// accumulated pothole damage that blows a tire

	// Cursor distance at which the car reaches top speed. The chase needs Nancy to sustain
	// full speed to keep pace with Jane's recorded run, so full throttle comes at a much
	// shorter cursor distance there than in the fuel-economy-minded free-driving map.
	const double kFullSpeedDist = (_variant == kChase) ? 120.0 : 250.0;

	uint32 nowMs = g_system->getMillis();
	double dt = (_lastPhysicsMs == 0) ? refStep : CLIP<double>((nowMs - _lastPhysicsMs) / 1000.0, 0.0, 0.1);
	_lastPhysicsMs = nowMs;

	// The further the cursor, the faster the car; the chase slowdown (_speedCap) caps it.
	double distanceCap = CLIP<double>(cursorDist / kFullSpeedDist * _forwardSpeed, 0.0, (double)_forwardSpeed);
	double forwardCap = MIN(distanceCap, MAX(0.0, _speedCap));

	// Mud slows the car (it does not stop it): while sitting in a puddle its top speed is
	// cut. Jane's car is a recorded playback that ignores the terrain, so the penalty is
	// kept mild - enough to matter in the free-driving map without making the chase, where
	// she never slows, unwinnable.
	Common::Point cur((int)(_carX + 0.5), (int)(_carY + 0.5));
	for (uint i = 0; i < _mudZones.size(); ++i) {
		if (_mudZones[i].decel > 0.0 && _mudZones[i].rect.contains(cur)) {
			forwardCap = MIN(forwardCap, (double)_forwardSpeed * 0.7);
			break;
		}
	}

	if (throttle > 0) {
		_carVelocity += forwardCap * dt;
	} else if (throttle < 0) {
		_carVelocity -= ((double)_forwardSpeed / 0.4) * dt;
	} else {
		_carVelocity *= pow(decay, dt / refStep);
	}
	_carVelocity = CLIP<double>(_carVelocity, -(double)_reverseSpeed, forwardCap);

	// Move against the collision mask, sub-stepped ~1px at a time so a fast car can't
	// tunnel through a thin road edge. On hitting off-road, slide along it (keep whichever
	// single axis stays on the road) instead of stopping dead. If the car somehow starts
	// the frame off-road, let it move out freely so it can never get wedged.
	double preX = _carX;
	double preY = _carY;
	double moveX = cos(_carHeading) * _carVelocity * dt;
	double moveY = -sin(_carHeading) * _carVelocity * dt;
	int steps = MAX(1, (int)(MAX(ABS(moveX), ABS(moveY)) + 0.5));
	double stepX = moveX / steps;
	double stepY = moveY / steps;
	bool escaping = isBlocked(cur);

	for (int s = 0; s < steps; ++s) {
		double tryX = CLIP<double>(_carX + stepX, 0.0, MAX(0, _image.w - 1));
		double tryY = CLIP<double>(_carY + stepY, 0.0, MAX(0, _image.h - 1));

		if (escaping || !isBlocked(Common::Point((int)(tryX + 0.5), (int)(tryY + 0.5)))) {
			_carX = tryX;
			_carY = tryY;
		} else if (!isBlocked(Common::Point((int)(tryX + 0.5), (int)(_carY + 0.5)))) {
			_carX = tryX;
		} else if (!isBlocked(Common::Point((int)(_carX + 0.5), (int)(tryY + 0.5)))) {
			_carY = tryY;
		} else {
			_carVelocity = 0.0;
			break;
		}
	}

	Common::Point next((int)(_carX + 0.5), (int)(_carY + 0.5));

	// The gas tank empties by the distance the car actually travels this frame divided by
	// the header's distance divisor. The DT_RESOURCE scene dependency reads the same
	// resource to warn Nancy when it runs low.
	if (_distanceDivisor > 0 && !_infiniteFuel) {
		double moved = sqrt((_carX - preX) * (_carX - preX) + (_carY - preY) * (_carY - preY));
		_fuelBurnAccum += moved / (double)_distanceDivisor;
		if (_fuelBurnAccum >= 1.0) {
			int burn = (int)_fuelBurnAccum;
			_fuelBurnAccum -= burn;
			int fuel = NancySceneState.getUIResource(_frictionIndex);
			NancySceneState.setUIResource(_frictionIndex, MAX(0, fuel - burn));
		}
	}

	// Driving into a pothole (on entry) damages the tires by a random amount; at 100 the
	// tire blows: the damage resets (a fresh spare goes on) and the car leaves for the
	// flat-tire scene once the blowout sound has played.
	for (uint i = 0; i < _potholes.size(); ++i) {
		Pothole &hole = _potholes[i];
		bool nowInside = hole.rect.contains(next);
		if (nowInside && !hole.carInside && !_flatTirePending) {
			int dmg = hole.minDamage;
			if (hole.maxDamage > hole.minDamage) {
				dmg += g_nancy->_randomSource->getRandomNumber(hole.maxDamage - hole.minDamage);
			}
			_tireDamage += dmg;
			if (_tireDamage >= kTireFlatThreshold) {
				_tireDamage = 0;
				_flatTirePending = true;
				playSoundBlock(_soundBlocks[0]);	// tire blowout
			}
		}
		hole.carInside = nowInside;
	}

	// Driving into a checkpoint sets its event flag, but only while the checkpoint's own
	// condition holds (so the chase phases fire in order). Edge-triggered on entering the
	// armed state.
	for (uint i = 0; i < _checkpoints.size(); ++i) {
		Checkpoint &cp = _checkpoints[i];
		bool cond = cp.condFlag == -1 || NancySceneState.getEventFlag(cp.condFlag, cp.condValue);
		bool active = cond && cp.rect.contains(next);
		if (active && !cp.wasActive && cp.flagId != -1) {
			NancySceneState.setEventFlag(cp.flagId, cp.flagValue ? g_nancy->_true : g_nancy->_false);
		}
		cp.wasActive = active;
	}

	// A drive-in destination (the chase finish line) fires on entry; a parking
	// destination (a location) is only noted here and entered with space in handleInput.
	_parkedDest = -1;
	for (uint i = 0; i < _destinations.size(); ++i) {
		DestinationZone &dest = _destinations[i];
		bool nowInside = dest.scene.sceneID != kNoScene && dest.rect.contains(next);
		if (dest.autoTrigger) {
			if (nowInside && !dest.carInside) {
				armExit(dest);
			}
			dest.carInside = nowInside;
		} else if (nowInside && _parkedDest < 0) {
			_parkedDest = (int)i;
		}
	}

	// Remember where the car is so it resumes here after a building visit or a save/load.
	saveState();
}

void DrivingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		if (_variant == kChase && _chaseParams[kChaseGate01Flag] != -1) {
			// Arm the main pursuit: with the gate flag set the chase begins in state 0 (Nancy
			// must keep Jane in sight). A checkpoint later clears it to start the shortcut. The
			// original relies on the scene to set this; do it here so the phase is never skipped.
			NancySceneState.setEventFlag(_chaseParams[kChaseGate01Flag], g_nancy->_true);
		}
		classifyZones(_zones);
		if (_variant == kChase) {
			classifyZones(_zones2);
		}
		playSoundBlock(_soundBlocks[2]);	// looping engine ambience
		drawScene();
		_state = kRun;
		break;
	case kRun:
		break;
	case kActionTrigger:
		g_nancy->_sound->stopSound(_soundBlocks[2].channel);	// stop the engine ambience
		NancySceneState.setEventFlag(_exitFlag, _exitFlagValue ? g_nancy->_true : g_nancy->_false);
		if (_exitHasFade)
			NancySceneState.specialEffect(_exitFadeType, _exitFadeTotalTime, _exitFadeToBlackTime, _exitFadeRect);
		NancySceneState.changeScene(_exitScene);
		finishExecution();
		break;
	}
}

void DrivingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Cheats: Ctrl+Shift+G toggles infinite fuel (tops the tank off and stops the drain);
	// Ctrl+Shift+T repairs the spare tire (clears pothole wear and restores it to good).
	for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
		const Common::KeyState &key = input.otherKbdInput[i];
		if ((key.flags & Common::KBD_CTRL) == 0 || (key.flags & Common::KBD_SHIFT) == 0) {
			continue;
		}
		if (key.keycode == Common::KEYCODE_g) {
			_infiniteFuel = !_infiniteFuel;
			if (_infiniteFuel) {
				refillFuel();
			}
			saveState();
			debug("Gas cheat: infinite fuel %s", _infiniteFuel ? "ON" : "OFF");
		} else if (key.keycode == Common::KEYCODE_t) {
			repairTire();
			saveState();
			debug("Tire cheat: spare tire repaired");
		}
	}

	// A tire has blown: hold the car still until the blowout sound finishes, then leave
	// for the flat-tire scene (where Nancy fits the spare).
	if (_flatTirePending) {
		if (!g_nancy->_sound->isSoundPlaying(_soundBlocks[0].channel)) {
			saveState();
			SceneChangeDescription scene;
			scene.sceneID = _finishScene;
			NancySceneState.changeScene(scene);
			finishExecution();
		}
		return;
	}

	// Parked in a location: pressing space gets Nancy out of the car and into it.
	if (_parkedDest >= 0 && _parkedDest < (int)_destinations.size()) {
		for (uint i = 0; i < input.otherKbdInput.size(); ++i) {
			if (input.otherKbdInput[i].keycode == Common::KEYCODE_SPACE) {
				armExit(_destinations[_parkedDest]);
				return;
			}
		}
	}

	// Throttle with the mouse buttons: left drives forward, right reverses.
	int throttle = 0;
	if (input.input & NancyInput::kLeftMouseButtonHeld) {
		throttle = 1;
	} else if (input.input & NancyInput::kRightMouseButtonHeld) {
		throttle = -1;
	}

	// Steer the car to face the cursor; its distance sets the driving speed.
	double cursorDist = 0.0;
	if (throttle != 0) {
		Common::Point cam = cameraOffset();
		Common::Rect mouseVp = NancySceneState.getViewport().convertScreenToViewport(
			Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
		double dx = (double)mouseVp.left - (_carX - cam.x);
		double dy = (double)mouseVp.top - (_carY - cam.y);
		cursorDist = sqrt(dx * dx + dy * dy);
		if (dx != 0.0 || dy != 0.0) {
			_carHeading = atan2(-dy, dx);
			if (_carHeading < 0.0) {
				_carHeading += 2.0 * M_PI;
			}
		}
	}

	// Drive continuously so momentum and the chaser animate every frame.
	if (_variant == kChase) {
		updateChaser();
		if (_state != kRun) {
			return;	// a chase outcome fired
		}
	}

	updatePhysics(throttle, cursorDist);

	if (_state == kRun) {
		drawScene();
	}
}

} // End of namespace Action
} // End of namespace Nancy
