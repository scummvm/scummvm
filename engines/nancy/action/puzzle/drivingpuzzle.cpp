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
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/util.h"

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
	stream.skip(2);
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
	// The car often spawns already sitting inside its starting location's zone, so
	// zones are edge-triggered: a zone only fires once the car has left it and driven
	// back in. Seed each zone's "inside" state from the spawn point.
	Common::Point spawn(_startX, _startY);

	for (uint i = 0; i < zones.size(); ++i) {
		const ActionZone &z = zones[i];
		switch (z.type) {
		case 0x11:		// location entrance
		case 0x0c: {	// chase finish line
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
			if (z.type == 0x0c) {
				dest.eventFlag = z.tailId;
				dest.eventFlagValue = z.tailFlag;
			}
			dest.carInside = dest.rect.contains(spawn);
			_destinations.push_back(dest);
			break;
		}
		case 0x0b: {	// checkpoint: sets an event flag once driven over
			Checkpoint cp;
			cp.rect = z.rect;
			cp.flagId = z.tailId;
			cp.flagValue = z.tailFlag;
			cp.carInside = cp.rect.contains(spawn);
			_checkpoints.push_back(cp);
			break;
		}
		case 0x14:	// play-area boundary
			_boundaries.push_back(z.rect);
			break;
		default:
			// The remaining subtypes (overlays, driving hazards, terrain markers) are
			// not simulated yet.
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
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
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

	// Seed the car from the physics parameters decoded from the header blob.
	_carX = _startX;
	_carY = _startY;
	_carHeading = (double)_startAngle * (M_PI / 180.0);
	_carVelocity = 0.0;
	_speedCap = _forwardSpeed;

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

void DrivingPuzzle::drawScene() {
	int w = _drawSurface.w;
	int h = _drawSurface.h;

	// Car-centered camera, clamped to the map bounds.
	int camX = CLIP<int>((int)(_carX + 0.5) - w / 2, 0, MAX(0, _image.w - w));
	int camY = CLIP<int>((int)(_carY + 0.5) - h / 2, 0, MAX(0, _image.h - h));

	_drawSurface.blitFrom(_image, Common::Rect(camX, camY, camX + w, camY + h), Common::Point(0, 0));

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

	_needsRedraw = true;
}

void DrivingPuzzle::updateChaser() {
	if (_chaserPathA.empty()) {
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
	while (_chaserWaypoint + 1 < _chaserPathA.size() && _chaserPathA[_chaserWaypoint].timeMs < elapsed) {
		++_chaserWaypoint;
	}

	const Waypoint &wp = _chaserPathA[_chaserWaypoint];
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

	// TODO: switch onto the second path (_chaserPathB), the event-flag outcomes
	// (_chaseParams) and the "chaser left the viewport" loss branch are not simulated.
}

// Per-frame car physics. The acceleration divisors (1.0/0.4) and 0.02 timestep are
// exact; the steering rate and velocity decay are playability stand-ins.
void DrivingPuzzle::updatePhysics(const NancyInput &input) {
	const double timeStep = 0.02;
	const double steerRate = 0.05;
	const double decay = 0.98;
	const double forwardCap = MAX(0.0, _speedCap);

	if (input.input & NancyInput::kMoveLeft) {
		_carHeading += steerRate;
	}
	if (input.input & NancyInput::kMoveRight) {
		_carHeading -= steerRate;
	}
	if (_carHeading < 0.0) {
		_carHeading += 2.0 * M_PI;
	} else if (_carHeading >= 2.0 * M_PI) {
		_carHeading -= 2.0 * M_PI;
	}

	if (input.input & NancyInput::kMoveUp) {
		_carVelocity += (forwardCap / 1.0) * timeStep;
	} else if (input.input & NancyInput::kMoveDown) {
		_carVelocity -= ((double)_forwardSpeed / 0.4) * timeStep;
	} else {
		_carVelocity *= decay;
	}
	_carVelocity = CLIP<double>(_carVelocity, -(double)_reverseSpeed, forwardCap);

	double newX = _carX + cos(_carHeading) * _carVelocity * timeStep;
	double newY = _carY - sin(_carHeading) * _carVelocity * timeStep;

	// Keep the car on the map and out of the boundary zones (coarse rect test, no mask).
	newX = CLIP<double>(newX, 0.0, MAX(0, _image.w - 1));
	newY = CLIP<double>(newY, 0.0, MAX(0, _image.h - 1));

	Common::Point next((int)(newX + 0.5), (int)(newY + 0.5));
	for (uint i = 0; i < _boundaries.size(); ++i) {
		if (_boundaries[i].contains(next)) {
			_carVelocity = 0.0;
			return;
		}
	}

	_carX = newX;
	_carY = newY;

	// TODO: burn fuel here - decrement the gas-gauge UI resource (index _frictionIndex)
	// by distance/_distanceDivisor and stop the car at 0. Deferred: the resource is an
	// integer, so the rate needs runtime tuning to avoid corrupting the saved value.

	// Driving into a checkpoint (on entry) sets its event flag once.
	for (uint i = 0; i < _checkpoints.size(); ++i) {
		Checkpoint &cp = _checkpoints[i];
		bool nowInside = cp.rect.contains(next);
		if (nowInside && !cp.carInside && !cp.triggered && cp.flagId != -1) {
			cp.triggered = true;
			NancySceneState.setEventFlag(cp.flagId, cp.flagValue ? g_nancy->_true : g_nancy->_false);
		}
		cp.carInside = nowInside;
	}

	// Driving into a location entrance / finish line (on entry) transitions there. The
	// zone the car spawned inside does not fire until the car leaves and re-enters.
	for (uint i = 0; i < _destinations.size(); ++i) {
		DestinationZone &dest = _destinations[i];
		bool nowInside = dest.rect.contains(next);
		if (nowInside && !dest.carInside && _triggeredDest < 0 && dest.scene.sceneID != kNoScene) {
			_triggeredDest = (int)i;
			_state = kActionTrigger;
		}
		dest.carInside = nowInside;
	}
}

void DrivingPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
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
		if (_triggeredDest >= 0 && _triggeredDest < (int)_destinations.size()) {
			const DestinationZone &dest = _destinations[_triggeredDest];
			if (dest.eventFlag != -1) {
				NancySceneState.setEventFlag(dest.eventFlag, dest.eventFlagValue ? g_nancy->_true : g_nancy->_false);
			}
			if (dest.hasFade) {
				NancySceneState.specialEffect(dest.fadeType, dest.fadeTotalTime, dest.fadeToBlackTime, dest.fadeRect);
			}
			NancySceneState.changeScene(dest.scene);
		}
		finishExecution();
		break;
	}
}

void DrivingPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Drive continuously so momentum, steering and the chaser animate every frame.
	if (_variant == kChase) {
		updateChaser();
	}

	updatePhysics(input);

	if (_state == kRun) {
		drawScene();
	}
}

} // End of namespace Action
} // End of namespace Nancy
