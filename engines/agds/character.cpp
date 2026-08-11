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

#include "agds/character.h"
#include "agds/agds.h"
#include "agds/animation.h"
#include "agds/object.h"
#include "agds/region.h"
#include "agds/resourceManager.h"
#include "common/array.h"
#include "common/debug.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/util.h"
#include "graphics/managed_surface.h"
#include <math.h>

namespace AGDS {

Character::Character(AGDSEngine *engine, const Common::String &name) : _engine(engine), _object(), _animation(nullptr), _jokes(false),
																	   _name(name),
																	   _enabled(true), _visible(false), _stopped(false), _shown(false),
																	   _phase(-1), _frames(0), _direction(-1), _movementDirections(0),
																	   _description(nullptr), _standing(false),
																	   _walkState(kWalkIdle), _targetDir(-1), _pathIndex(0),
																	   _finalDir(-1), _exactMove(false), _walkPhase(0),
																	   _cyclesA(0), _cyclesB(0), _playStopAnim(false), _scaleIdx(0),
																	   _errX(0), _errY(0), _errStepX(0), _errStepY(0),
																	   _errPeriodX(1), _errPeriodY(1), _errPhaseX(0), _errPhaseY(0),
																	   _walkTickCnt(0), _cycleErrX(0), _cycleErrY(0), _corrX(0), _corrY(0),
																	   _decodedFrame(0) {
}

Character::~Character() {
}

void Character::load(Common::SeekableReadStream &stream) {
	debug("loading character...");
	stream.readUint32LE(); // unk
	uint16 magic = stream.readUint16LE();
	switch (magic) {
	case 0xdead:
		_movementDirections = 16;
		break;
	case 0x8888:
		_movementDirections = 8;
		break;
	default:
		error("invalid magic %04x", magic);
	}

	_animations.clear();
	while (stream.pos() < stream.size()) {
		uint size = stream.readUint32LE();
		uint index = stream.readUint16LE();
		debug("header size %u, index: %u", size, index);

		uint16 frames = stream.readUint16LE();
		uint16 format = stream.readUint16LE();
		Common::String filename = readString(stream);

		AnimationDescription animation;
		animation.filename = filename;
		debug("%s:%u: animation %s, frames: %d, format: %d", _name.c_str(), _animations.size(), animation.filename.c_str(), frames, format);
		while (frames--) {
			int x = stream.readSint16LE();
			int y = stream.readSint16LE();
			uint w = stream.readUint32LE();
			uint h = stream.readUint32LE();
			stream.readUint32LE(); // bit depth
			int cropX = stream.readSint32LE(); // position of the crop inside the FLIC frame
			int cropY = stream.readSint32LE();
			stream.readUint32LE(); // in-memory pixel pointer slot
			stream.readUint32LE(); // flag byte + palette pointer slot (packed)
			stream.readByte();
			uint canvasW = stream.readUint32LE(); // logical canvas width
			uint canvasH = stream.readUint32LE(); // logical canvas height
			stream.readUint32LE(); // CDCDCDCD
			int offX = stream.readSint32LE(); // anchor tweak inside canvas
			int offY = stream.readSint32LE();
			stream.readUint32LE(); // CDCDCDCD
			uint unk11 = stream.readByte();
			stream.readUint32LE(); // CDCDCDCD
			AnimationDescription::Frame frame = {x, y, w, h, canvasW, canvasH, offX, offY, cropX, cropY};
			animation.frames.push_back(frame);
			debug("frame %d, %d, %dx%d canvas %ux%u off %d,%d crop %d,%d (unk %u)",
				  x, y, w, h, canvasW, canvasH, offX, offY, cropX, cropY, unk11);
		}
		_animations[index] = animation;
	}
}

void Character::associate(const Common::String &name) {
	_object = _engine->loadObject(name);
	_engine->runObject(_object);
}

void Character::visible(bool visible) {
	if (visible) {
		_shown = true;
	}
	_visible = visible;
}

void Character::loadState(Common::ReadStream &stream) {
	int x = stream.readUint16LE();
	int y = stream.readUint16LE();
	int dir = stream.readSint16LE();
	debug("character at %d, %d, dir: %d", x, y, dir);
	position(Common::Point(x, y));
	direction(dir);
	_visible = stream.readUint16LE();
	_enabled = stream.readUint16LE();
}

void Character::saveState(Common::WriteStream &stream) const {
	stream.writeUint16LE(_pos.x);
	stream.writeUint16LE(_pos.y);
	stream.writeUint16LE(_direction);
	stream.writeUint16LE(_visible);
	stream.writeUint16LE(_enabled);
}

bool Character::direction(int dir) {
	debug("setDirection %d", dir);

	if (dir == -1) {
		// -1 sentinel from the game data means "keep current direction":
		// do nothing, do not touch _direction, do not swap animation.
		return false;
	}

	// out-of-range directions fall back to 8 (facing camera), like the original
	if (dir < 0 || dir > 15) {
		warning("Character %s: direction %d out of range, defaulting to 8",
		        _object ? _object->getName().c_str() : "?", dir);
		dir = 8;
	}

	_direction = dir;
	// idle facing shows phase 0 frame 0 (the standing pose)
	return animate(animIndexFor(dir, 0), 100, false);
}

void Character::notifyProcess(const Common::String &name) {
	debug("%s:notifyProcess %s", _name.c_str(), name.c_str());
	if (!_processName.empty())
		_engine->reactivate(name, "Character::notifyProcess", false);

	_processName = name;
}

// --- Walking (faithful reimplementation of the original state machine) ---

static inline double wDistSq(Common::Point a, Common::Point b) {
	double dx = a.x - b.x, dy = a.y - b.y;
	return dx * dx + dy * dy;
}

double Character::scalePermyriad(int v, int scale) {
	double r = (double)v * scale / 10000.0;
	if (r > -0.1 && r < 0.1)
		r = (r >= 0) ? 0.1 : -0.1;
	return r;
}

int Character::perspectiveScalePermyriad(int y) const {
	auto screen = _engine->getCurrentScreen();
	if (!screen)
		return 10000;
	return (int)(screen->getZScale(y) * 10000.0f + 0.5f);
}

int Character::scaleForSegmentY(int simY, int targetY) const {
	int y;
	if (_direction == 4 || _direction == 12)
		y = _pos.y;                                    // horizontal
	else if (_direction <= 3 || _direction >= 13)
		y = MAX(simY, targetY);                        // walking up
	else
		y = MIN(simY, targetY);                        // walking down
	return perspectiveScalePermyriad(y);
}

Common::Point Character::totalDisp(int dir, int phase) const {
	const AnimationDescription *desc = animationDescription(animIndexFor(dir, phase));
	if (!desc || desc->frames.empty())
		return Common::Point();
	const AnimationDescription::Frame &f = desc->frames[desc->frames.size() - 1];
	return Common::Point(f.x, f.y);
}

void Character::checkTrapRegions() {
	auto *screen = _engine->getCurrentScreen();
	if (!screen)
		return;
	auto objects = screen->find(_pos);
	for (auto &object : objects) {
		auto region = object->getTrapRegion();
		if (region && region->pointIn(_pos)) {
			debug("starting trap process");
			_engine->runProcess(object, object->getTrapHandler());
		}
	}
}

bool Character::loadWalkAnim() {
	int index = animIndexFor(_direction, _walkPhase < 0 ? 0 : _walkPhase);
	const AnimationDescription *desc = animationDescription(index);
	if (!desc)
		return false;
	if (_description != desc || !_animation) {
		auto animation = _engine->loadAnimation(desc->filename);
		if (!animation) {
			warning("no walk animation file %s", desc->filename.c_str());
			return false;
		}
		_description = desc;
		_animation = animation;
		_frames = desc->frames.size();
		_decodedFrame = 0;
		_jokes = false;
		_standing = true; // frames are driven by the walk machine, not Animation::tick
		_animation->scale(perspectiveScale());
	}
	if (_phase < _decodedFrame) {
		_animation->rewind();
		_animation->decodeNextFrame();
		_decodedFrame = 0;
	}
	while (_decodedFrame < _phase && _decodedFrame + 1 < (int)_description->frames.size()) {
		_animation->decodeNextFrame();
		_decodedFrame++;
	}
	return true;
}

int Character::advanceWalkPhase(bool cont) {
	switch (_walkPhase) {
	case 0:
		if (_cyclesA == 0 && !cont)
			_walkPhase = _playStopAnim ? 3 : -1;
		else {
			if (_cyclesA > 0)
				_cyclesA--;
			_walkPhase = 1;
		}
		break;
	case 1:
		if (_cyclesB == 0 && !cont)
			_walkPhase = _playStopAnim ? 4 : -1;
		else {
			if (_cyclesB > 0)
				_cyclesB--;
			_walkPhase = 2;
		}
		break;
	case 2:
		if (_cyclesA == 0 && !cont)
			_walkPhase = _playStopAnim ? 3 : -1;
		else {
			if (_cyclesA > 0)
				_cyclesA--;
			_walkPhase = 1;
		}
		break;
	default:
		_walkPhase = cont ? 0 : -1;
		break;
	}
	return _walkPhase;
}

void Character::setCycleScale(uint idx, int scale) {
	while (_cycleScales.size() <= idx)
		_cycleScales.push_back(scale);
	_cycleScales[idx] = scale;
}

void Character::setupSegmentPhased(int dx, int dy) {
	bool firstSeg = (_pathIndex == 1);
	bool playStart = firstSeg && _walkPhase == 0;
	bool lastWaypoint = (_pathIndex + 1 >= _path.size());
	_playStopAnim = lastWaypoint && _walkState != kWalkLeaving;

	int scale0 = perspectiveScalePermyriad(_pos.y);
	_cycleScales.clear();
	_cycleScales.push_back(scale0);

	Common::Point dispA = totalDisp(_direction, 1);
	Common::Point dispB = totalDisp(_direction, 2);
	Common::Point dispS;
	if (playStart)
		dispS = totalDisp(_direction, 0);
	else if (!firstSeg)
		advanceWalkPhase(true); // keep the A/B alternation going across waypoints
	Common::Point dispS3 = _playStopAnim ? totalDisp(_direction, 3) : Common::Point();
	Common::Point dispS4 = _playStopAnim ? totalDisp(_direction, 4) : Common::Point();

	int sgnX = dx < 0 ? -1 : 1, adx = ABS(dx);
	int sgnY = dy < 0 ? -1 : 1, ady = ABS(dy);
	Common::Point target(_pos.x + sgnX * adx, _pos.y + sgnY * ady);

	int curPhase = _walkPhase ? _walkPhase : 1;
	double simX = _pos.x, simY = _pos.y;
	uint idx = 0;
	if (playStart) {
		simX += (int)scalePermyriad(dispS.x, scale0);
		simY += (int)scalePermyriad(dispS.y, scale0);
		idx = 1; // scale[0] is consumed by the start animation
	}
	int sc = scaleForSegmentY((int)simY, target.y);
	setCycleScale(idx++, sc);

	auto endWithStop = [&](int cp, int scl) -> Common::Point {
		Common::Point e((int16)simX, (int16)simY);
		if (_playStopAnim) {
			Common::Point d = (cp == 1) ? dispS3 : dispS4;
			e.x += (int)scalePermyriad(d.x, scl);
			e.y += (int)scalePermyriad(d.y, scl);
		}
		return e;
	};

	Common::Point end = endWithStop(curPhase, sc);
	double best = wDistSq(end, target);
	_cyclesA = _cyclesB = 0;
	Common::Point prevEnd = end;
	double prevBest = best;
	for (int iter = 0; iter < 100; ++iter) {
		prevEnd = end;
		prevBest = best;
		if (curPhase == 1) {
			simX += (int)scalePermyriad(dispA.x, sc);
			simY += (int)scalePermyriad(dispA.y, sc);
			curPhase = 2;
			_cyclesA++;
		} else {
			simX += (int)scalePermyriad(dispB.x, sc);
			simY += (int)scalePermyriad(dispB.y, sc);
			curPhase = 1;
			_cyclesB++;
		}
		sc = scaleForSegmentY((int)simY, target.y);
		setCycleScale(idx++, sc);
		end = endWithStop(curPhase, sc);
		best = wDistSq(end, target);
		if (best >= prevBest)
			break;
	}
	_scaleIdx = 0;
	// last simulated cycle overshot: retract it (curPhase already flipped)
	if (curPhase == 1) {
		if (_cyclesB > 0)
			_cyclesB--;
	} else {
		if (_cyclesA > 0)
			_cyclesA--;
	}
	// no start, no stop, zero cycles: force one cycle, keep the overshoot end
	if (!playStart && !_playStopAnim && !_cyclesA && !_cyclesB) {
		prevEnd = end;
		if (curPhase == 1)
			_cyclesB++;
		else
			_cyclesA++;
	}
	// --- error budget: distribute the difference between the animation's
	// natural end point and the exact waypoint across the walk frames ---
	auto frameCountOf = [&](int phase) -> int {
		const AnimationDescription *d = animationDescription(animIndexFor(_direction, phase));
		return (d && !d->frames.empty()) ? (int)d->frames.size() : 1;
	};
	int stopIdx = (_cyclesA > _cyclesB) ? 4 : 3;
	int extra = _playStopAnim ? frameCountOf(stopIdx) - (2 * frameCountOf(stopIdx)) / 10 : 0;
	int plannedDX = prevEnd.x - _pos.x;
	int plannedDY = prevEnd.y - _pos.y;
	_errX = sgnX * adx - plannedDX;
	_errY = sgnY * ady - plannedDY;
	if (_exactMove && _playStopAnim && playStart) {
		if (!_cyclesA && !_cyclesB && _errX * sgnX < 0) {
			// planned end overshoots even with zero cycles: just turn
			_walkState = kWalkTurning;
			return;
		}
	}
	if (playStart)
		extra += frameCountOf(0) - (2 * frameCountOf(0)) / 10 - 1;
	int total = _cyclesB * (frameCountOf(2) - 1) + extra + _cyclesA * (frameCountOf(1) - 1);
	if (total <= 0)
		total = 1;
	_errStepX = _errX / total;
	_errX %= total;
	_errStepY = _errY / total;
	_errY %= total;
	_errPeriodX = _errX ? total / ABS(_errX) : 1;
	_errPeriodY = _errY ? total / ABS(_errY) : 1;
	if (_errPeriodX <= 0)
		_errPeriodX = 1;
	if (_errPeriodY <= 0)
		_errPeriodY = 1;
	_errPhaseX = (total - _errPeriodX * ABS(_errX)) / 2;
	_errPhaseY = (total - _errPeriodY * ABS(_errY)) / 2;

	// the in-progress cycle consumes one planned cycle, or, with nothing
	// planned at all, we skip straight to the stop animation
	if (_playStopAnim && !playStart && !_cyclesA && !_cyclesB)
		_walkPhase = (_walkPhase == 1) ? 3 : 4;
	else if (_walkPhase == 1) {
		if (_cyclesA > 0)
			_cyclesA--;
	} else if (_walkPhase == 2) {
		if (_cyclesB > 0)
			_cyclesB--;
	}
}

void Character::startNextSegment() {
	if (_pathIndex >= _path.size()) {
		_walkState = kWalkIdle;
		return;
	}
	_segTarget = _path[_pathIndex];
	int dx = _segTarget.x - _pos.x;
	int dy = _segTarget.y - _pos.y;
	_segStart = _pos;
	_cycleErrX = _cycleErrY = _corrX = _corrY = 0;
	_walkTickCnt = 0;

	int oldDir = _direction;
	_direction = getDirectionForMovement(Common::Point(dx, dy));
	if (phased())
		setupSegmentPhased(dx, dy);
	_pathIndex++;
	_targetDir = _direction;
	_direction = oldDir; // keep facing the old way until the turn completes
}

void Character::stepErrorCorrection() {
	auto frameCountOf = [&](int phase) -> int {
		const AnimationDescription *d = animationDescription(animIndexFor(_direction, phase));
		return (d && !d->frames.empty()) ? (int)d->frames.size() : 1;
	};
	// skip the first 20% of the start anim and the last 20% of stop anims
	if (_walkPhase == 0 && _phase < (2 * frameCountOf(0)) / 10)
		return;
	if (_walkPhase >= 3) {
		int c = frameCountOf(_walkPhase);
		if (_phase >= c - (2 * c) / 10)
			return;
	}
	if (_walkTickCnt >= _errPhaseX && _errX != 0 && _walkTickCnt % _errPeriodX == 0) {
		if (_errX > 0) {
			_corrX++;
			_errX--;
		} else {
			_corrX--;
			_errX++;
		}
	}
	if (_walkTickCnt >= _errPhaseY && _errY != 0 && _walkTickCnt % _errPeriodY == 0) {
		if (_errY > 0) {
			_corrY++;
			_errY--;
		} else {
			_corrY--;
			_errY++;
		}
	}
	_walkTickCnt++;
	_corrX += _errStepX;
	_corrY += _errStepY;
}

void Character::advanceWalkFrame() {
	_phase++;
	const AnimationDescription *desc = animationDescription(animIndexFor(_direction, _walkPhase < 0 ? 0 : _walkPhase));
	int count = (desc && !desc->frames.empty()) ? (int)desc->frames.size() : 1;
	if (_phase >= count) {
		auto screen = _engine->getCurrentScreen();
		RegionPtr region = screen ? screen->region() : RegionPtr();
		bool pending = _walkState == kWalkRemovePending || _walkState == kWalkLeavePending ||
		               _walkState == kWalkStopPending;
		if (pending && region && region->walkPointInside(_pos) > 0) {
			_walkPhase = -1;
		} else {
			// cycle boundary: bank the actual-vs-predicted error
			_cycleErrX = _pos.x - _segStart.x - _corrX;
			_cycleErrY = _pos.y - _segStart.y - _corrY;
			advanceWalkPhase(false);
			_scaleIdx++;
		}
		_phase = 1;
	}
}

bool Character::walkTickAdvance() {
	int savedPhase = _walkPhase;
	advanceWalkFrame();
	if (_walkPhase == -1) {
		switch (_walkState) {
		case kWalkRemovePending:
		case kWalkLeavePending: {
			auto screen = _engine->getCurrentScreen();
			RegionPtr region = screen ? screen->region() : RegionPtr();
			if (region && region->walkPointInside(_pos) > 0) {
				_walkPhase = savedPhase;
				return false;
			}
			_walkPhase = _playStopAnim ? 0 : savedPhase;
			break;
		}
		case kWalkStopPending:
			_walkPhase = 0;
			break;
		default:
			_walkPhase = _playStopAnim ? 0 : savedPhase;
			break;
		}
		return false;
	}
	stepErrorCorrection();
	if (!loadWalkAnim())
		return false;
	if (_description && !_description->frames.empty()) {
		int idx = CLIP(_phase, 0, (int)_description->frames.size() - 1);
		const AnimationDescription::Frame &f = _description->frames[idx];
		int sc = _cycleScales.empty()
		             ? 10000
		             : _cycleScales[CLIP(_scaleIdx, 0, (int)_cycleScales.size() - 1)];
		_pos.x = _segStart.x + _corrX + _cycleErrX + (int)scalePermyriad(f.x, sc);
		_pos.y = _segStart.y + _corrY + _cycleErrY + (int)scalePermyriad(f.y, sc);
	}
	return true;
}

void Character::arrive(int prevState) {
	switch (prevState) {
	case kWalkWalking:
	case kWalkRemovePending:
	case kWalkStopPending:
		if (_finalDir != -1) {
			// reload the standing pose before turning so the displayed
			// frame matches the anchor again
			_walkPhase = 0;
			_phase = 0;
			direction(_direction);
			_walkState = kWalkTurning;
			_targetDir = _finalDir;
			return;
		}
		break;
	case kWalkLeaving:
	case kWalkLeavePending:
		advanceWalkPhase(true);
		break;
	default:
		break;
	}
	_walkPhase = 0;
	_phase = 0;
	direction(_direction); // reload the standing pose
	checkTrapRegions();
	// the waiting process is reactivated by tick() now that we are idle
}

void Character::walkingTick() {
	if (walkTickAdvance())
		return;

	int prevState = _walkState;
	bool snap = false;
	Common::Point savedTarget;
	auto screen = _engine->getCurrentScreen();
	RegionPtr region = screen ? screen->region() : RegionPtr();
	bool inside = region && region->walkPointInside(_pos) > 0;

	switch (_walkState) {
	case kWalkRemovePending:
		if (inside) {
			advanceWalkPhase(true);
			_path.clear();
			_pathIndex = 0;
			_walkState = kWalkIdle;
			Common::String proc = _processName;
			_processName.clear();
			moveTo(proc, _finalTarget, _finalDir, _exactMove);
			return;
		}
		_phase = 0;
		break;
	case kWalkLeavePending:
		if (inside) {
			advanceWalkPhase(true);
			_path.clear();
			_pathIndex = 0;
			_walkState = kWalkIdle;
			Common::String proc = _processName;
			_processName.clear();
			moveTo(proc, _finalTarget, _finalDir, false, true);
			return;
		}
		_phase = 0;
		break;
	case kWalkStopPending: {
		_playStopAnim = true;
		_cyclesA = _cyclesB = 0;
		advanceWalkPhase(false);
		_path.clear();
		_pathIndex = 0;
		_walkState = kWalkIdle;
		Common::Point d = totalDisp(_direction, _walkPhase < 0 ? 0 : _walkPhase);
		int sc = _cycleScales.empty() ? 10000 : _cycleScales[0];
		Common::Point t(_pos.x + (int)scalePermyriad(d.x, sc),
		                _pos.y + (int)scalePermyriad(d.y, sc));
		_finalTarget = t;
		if (region && region->walkPointInside(t) > 0) {
			Common::String proc = _processName;
			_processName.clear();
			moveTo(proc, t, _finalDir);
			return;
		}
		_walkPhase = 0;
		_phase = 0;
		break;
	}
	default: // kWalkWalking, kWalkLeaving
		savedTarget = _segTarget;
		snap = true;
		_phase = 0;
		break;
	}

	startNextSegment();
	if (_walkState == kWalkIdle) {
		if (snap)
			_pos = savedTarget; // land exactly on the waypoint
		arrive(prevState);
	} else {
		loadWalkAnim(); // resync the displayed frame with the new segment
	}
}

void Character::turnOneStep() {
	int step = (_movementDirections == 16) ? 1 : 2;
	int diff = (_direction - _targetDir) & 15;
	if (diff <= 8)
		_direction = (_direction == 0) ? 16 - step : _direction - step;
	else
		_direction = (_direction == 16 - step) ? 0 : _direction + step;
	loadWalkAnim(); // refresh the sprite in the new facing, same phase/frame
}

void Character::walkUpdateTick() {
	if (_walkState == kWalkIdle || _jokes)
		return;
	// 8-direction characters can only face even directions; an odd turn
	// target would never be reached with 2-step turns
	if (_movementDirections == 8)
		_targetDir &= ~1;
	if (_direction == _targetDir) {
		if (_walkState == kWalkTurning) {
			_walkState = kWalkIdle;
			_walkPhase = 0;
			_phase = 0;
			direction(_direction);
		} else {
			walkingTick();
		}
	} else {
		turnOneStep();
	}
}

void Character::stopMoving(bool force) {
	if (_jokes) {
		_jokes = false;
		_phase = -1;
		_frames = 0;
		direction(_direction);
		return;
	}
	bool deferred = (_walkState == kWalkRemovePending || _walkState == kWalkLeavePending ||
	                 _walkState == kWalkStopPending) &&
	                !force;
	if (!deferred) {
		_path.clear();
		_pathIndex = 0;
	}
	if (deferred)
		return;
	_walkState = kWalkIdle;
	_walkPhase = 0;
	_phase = 0;
	if (_finalDir >= 0 && _finalDir <= 15)
		_direction = _finalDir;
	else if (_direction < 0 || _direction > 15)
		_direction = 0;
	direction(_direction);
}

void Character::requestStop(int dir) {
	if (_jokes) {
		stopMoving(true);
		if (dir != -1)
			direction(dir);
		return;
	}
	switch (_walkState) {
	case kWalkIdle:
		if (dir != -1)
			direction(dir);
		return;
	case kWalkTurning:
		_walkState = kWalkIdle;
		if (dir != -1) {
			_direction = dir;
			direction(dir);
		}
		return;
	default:
		if (_walkPhase >= 3) // already finishing a stop animation
			return;
		_walkState = kWalkStopPending;
		_finalDir = dir;
		return;
	}
}

bool Character::moveTo(const Common::String &processName, Common::Point dst, int dir,
                       bool userMove, bool leave) {
	if (!_visible || !_enabled)
		return false;

	debug("character move %d,%d dir %d user %d leave %d", dst.x, dst.y, dir, userMove, leave);
	notifyProcess(processName);
	_shown = true;

	auto *screen = _engine->getCurrentScreen();
	RegionPtr region = screen ? screen->region() : RegionPtr();
	if (!region || region->empty()) {
		// no walk region on this screen: legacy teleport
		_pos = dst;
		bool r = direction(dir);
		checkTrapRegions();
		return r;
	}

	Common::Point clickPos = dst;

	// make sure we start from a walkable spot
	if (region->walkPointInside(_pos) < 1) {
		Common::Point p = _pos;
		if (region->snapSpiral(p, 8))
			_pos = p;
		else
			_pos = region->center;
	}

	// resolve current state
	if (_jokes)
		stopMoving(true); // abort gesture, walk proceeds
	switch (_walkState) {
	case kWalkIdle:
		break;
	case kWalkTurning:
		_targetDir = _direction;
		_walkState = kWalkIdle;
		break;
	default: // currently walking: defer
		if (leave) {
			_walkState = kWalkLeavePending;
			_exactMove = false;
			_finalDir = dir;
			_finalTarget = dst;
			return true;
		}
		if (_walkState == kWalkWalking && dst == _finalTarget) {
			_exactMove = userMove;
			_finalDir = dir;
			return true; // already going there
		}
		_walkState = kWalkRemovePending;
		_exactMove = userMove;
		_finalDir = dir;
		_finalTarget = dst;
		return true;
	}

	if (_pos == dst) { // turn in place
		if (dir == -1)
			return false;
		_targetDir = dir;
		_walkState = kWalkTurning;
		return true;
	}

	if (!region->buildWalkPath(_path, _pos, dst)) {
		// destination unreachable: find a "near way"
		Common::Point p = dst;
		bool onVertex = false;
		bool ok = region->snapToNearestEdge(p, onVertex);
		if (ok) {
			if (!onVertex) {
				if (p == clickPos)
					ok = region->lineStepUntilInside(p, region->center.x - p.x, region->center.y - p.y);
				else
					ok = region->lineStepUntilInside(p, p.x - clickPos.x, p.y - clickPos.y);
			} else {
				ok = region->snapSpiral(p, 5);
			}
		}
		if (ok)
			ok = region->buildWalkPath(_path, _pos, p);
		if (ok && _path.size() >= 2 &&
		    wDistSq(_path[_path.size() - 1], _path[_path.size() - 2]) < 169.0) // < 13 px
			_path.pop_back();
		if (!ok || _path.size() < 2) {
			_path.clear();
			_processName.clear(); // nothing to wait for
			return false;
		}
		dst = p;
	}

	if (leave && _walkState == kWalkIdle)
		_walkState = kWalkLeaving;
	if (_walkState != kWalkLeaving)
		_walkState = kWalkWalking;

	_pathIndex = 1;
	_exactMove = userMove;
	_finalDir = dir;
	_finalTarget = dst;
	startNextSegment();
	if (_walkState == kWalkTurning) {
		// setup decided the step is too short: just turn toward the target
		if (dir == -1)
			_targetDir = getDirectionForMovement(Common::Point(clickPos.x - _pos.x, clickPos.y - _pos.y));
		else
			_targetDir = dir;
	}
	return true;
}

void Character::pointTo(const Common::String &processName, Common::Point dst) {
	debug("character point to %d,%d, process: %s", dst.x, dst.y, processName.c_str());
	notifyProcess(processName);
	_shown = true;
	if (_walkState == kWalkIdle && !_jokes && dst != _pos) {
		int dir = getDirectionForMovement(Common::Point(dst.x - _pos.x, dst.y - _pos.y));
		if (_movementDirections == 8)
			dir &= ~1;
		if (dir != _direction) {
			_targetDir = dir;
			_walkState = kWalkTurning;
			return; // the waiting process wakes once the turn completes
		}
	}
	// already facing the target (or busy): wake the process on the next
	// idle tick
}

bool Character::animate(int animIndex, int speed, bool jokes) {
	if (animIndex == -1 || !_enabled)
		return false;

	if (_stopped) {
		debug("character stopped, skipping");
		_stopped = false;
		return false;
	}

	auto character = jokes ? _engine->jokes() : this;
	auto description = character->animationDescription(animIndex);
	if (!description) {
		warning("no %s animation %d", jokes ? "jokes" : "character", animIndex);
		return false;
	}
	auto animation = _engine->loadAnimation(description->filename);
	if (!animation) {
		warning("no %s animation file %s", jokes ? "jokes" : "character", description->filename.c_str());
		return false;
	}
	_description = description;
	_shown = true;
	_animation = animation;
	_animation->speed(speed);
	_animation->rewind();
	_phase = 0;
	_frames = _animation->frames();
	_jokes = jokes;
	// only jokes and walking advance frames; idle poses stay on frame 0
	_standing = !jokes;
	if (jokes)
		_jokesDirection = animIndex;
	// apply the perspective scale now: paint() can run before the next tick()
	_animation->scale(perspectiveScale());
	debug("character animation frames: %d, enabled: %d, visible: %d", _frames, _enabled, _visible);
	return true;
}

float Character::perspectiveScale() const {
	auto screen = _engine->getCurrentScreen();
	float scale = screen ? screen->getZScale(_pos.y) : 1.0f;

	// normalize by the canvas height ratio between the reference direction 4
	// and the current animation; during a joke, the standing pose is used
	if (phased() && _direction >= 0) {
		int phase = _jokes ? 0 : CLIP(_walkPhase, 0, kAnimsPerDirection - 1);
		int frame = _jokes ? 0 : CLIP(_phase, 0, _frames > 0 ? _frames - 1 : 0);
		const AnimationDescription::Frame *num = frameAt(4, phase, frame);
		const AnimationDescription::Frame *den = frameAt(_direction, phase, frame);
		if (num && den && den->canvasH > 0)
			scale = scale * num->canvasH / den->canvasH;
	}
	return scale;
}

bool Character::animate(Common::Point pos, int direction, int speed) {
	debug("animate character: %d,%d %d %d", pos.x, pos.y, direction, speed);
	auto ok = animate(direction, speed, true);
	if (!ok)
		return false;

	_animationPos = pos;

	return true;
}

void Character::stop() {
	_stopped = true;
}

void Character::leave(const Common::String &processName) {
	debug("character %s: leave, process: %s", _object->getName().c_str(), processName.c_str());
	notifyProcess(processName);
}

void Character::tick(bool reactivate) {
	if (!active())
		return;

	walkUpdateTick();

	if (_animation) {
		_animation->scale(perspectiveScale());

		if (!_stopped && !_standing && _phase >= 0 && _phase < _frames) {
			_animation->tick();
			_phase = _animation->phase();
			if (_phase >= _frames) {
				bool wasJokes = _jokes;
				_jokes = false;
				_phase = -1;
				_frames = 0;
				if (wasJokes)
					direction(_direction);
			}
			return;
		}
	}

	if (_walkState == kWalkIdle &&
	    reactivate && !_processName.empty() && !_engine->activeCurtain()) {
		_engine->reactivate(_processName, "Character::tick");
		_processName.clear();
	}
}

bool Character::pointIn(Common::Point pos) const {
	if (!_animation)
		return false;

	Common::Rect rect(_animation->width(), _animation->height());
	rect.moveTo(animationPosition());
	return rect.contains(pos);
}

Common::Point Character::animationPosition() const {
	Common::Point pos = _pos;

	if (!_animation)
		return pos;

	float scale = _animation->scale();
	int anchorX = 0, anchorY = 0;

	if (_description && !_description->frames.empty()) {
		auto &frames = _description->frames;
		int idx = CLIP(_phase, 0, (int)frames.size() - 1);
		const AnimationDescription::Frame &cur = frames[idx];
		const AnimationDescription::Frame &first = frames[0];

		// records describe a crop of the FLIC frame; since the whole FLIC
		// frame is drawn, the crop position is folded into the anchor
		if (phased()) {
			if (_jokes) {
				// gestures keep the feet anchored where the standing pose was
				anchorX = (int)cur.w / 2 + first.offX - cur.x + ((int)first.canvasW / 2 - (int)first.w / 2) + cur.cropX;
				anchorY = (int)cur.h + first.offY - cur.y + ((int)first.canvasH - (int)first.h) + cur.cropY;
			} else {
				// bottom-center of the logical canvas
				anchorX = (int)cur.canvasW / 2 + cur.offX + cur.cropX;
				anchorY = (int)cur.canvasH + cur.offY + cur.cropY;
			}
		} else {
			anchorX = (int)first.w / 2 + cur.x + cur.cropX;
			anchorY = (int)first.h + cur.y + cur.cropY;
		}
	} else {
		// No frame metadata: anchor at the bottom-center of the already
		// scaled bitmap.
		pos.x -= _animation->width() / 2;
		pos.y -= _animation->height();
		if (_jokes)
			pos += _animationPos;
		return pos;
	}

	pos.x -= (int)(anchorX * scale);
	pos.y -= (int)(anchorY * scale);

	// Script-supplied gesture offsets are applied unscaled, and only while
	// the gesture is playing.
	if (_jokes)
		pos += _animationPos;

	return pos;
}

void Character::paint(Graphics::Surface &backbuffer, Common::Point pos) const {
	if (!_enabled || !visible() || !_animation)
		return;

	pos += animationPosition();

	int fogAlpha = 0;
	if (_fog) {
		auto z = this->z();
		if (z >= _fogMinZ && z < _fogMaxZ) {
			fogAlpha = 255 * (z - _fogMinZ) / (_fogMaxZ - _fogMinZ);
		} else if (z >= _fogMaxZ) {
			fogAlpha = 255;
		}
	}
	_animation->paint(backbuffer, pos, _fog.get(), fogAlpha);
}

int Character::z() const {
	return g_system->getHeight() - _pos.y;
}

void Character::reset() {
	_fog.reset();
	_shown = false;
	_animation.reset();
	_description = nullptr;
	_phase = -1;
	_frames = 0;
	_jokes = false;
	_walkPhase = 0;
	_decodedFrame = 0;
	cancelWalk();
}

void Character::setFog(Graphics::ManagedSurface *surface, int minZ, int maxZ) {
	_fog.reset(surface);
	_fogMinZ = minZ;
	_fogMaxZ = maxZ;
}

int Character::getDirectionForMovement(Common::Point delta) {
	auto angle = atan2(delta.y, delta.x);
	if (angle < 0)
		angle += M_PI * 2;

	if (_movementDirections == 16) {
		if (angle < 6.1850053125 && angle > 0.0981746875) {
			if (angle > 0.5235983333333333) {
				if (angle > 0.9490219791666666) {
					if (angle > 1.374445625) {
						if (angle > 1.767144375) {
							if (angle > 2.192568020833333) {
								if (angle > 2.617991666666666) {
									if (angle > 3.0434153125) {
										if (angle > 3.2397646875) {
											if (angle > 3.665188333333333) {
												if (angle > 4.090611979166667) {
													if (angle > 4.516035625) {
														if (angle > 4.908734375) {
															if (angle > 5.334158020833333) {
																if (angle > 5.759581666666667)
																	return 3;
																else
																	return 2;
															} else {
																return 1;
															}
														} else {
															return 0;
														}
													} else {
														return 15;
													}
												} else {
													return 14;
												}
											} else {
												return 13;
											}
										} else {
											return 12;
										}
									} else {
										return 11;
									}
								} else {
									return 10;
								}
							} else {
								return 9;
							}
						} else {
							return 8;
						}
					} else {
						return 7;
					}
				} else {
					return 6;
				}
			} else {
				return 5;
			}
		} else {
			return 4;
		}
	} else if (angle < 5.89048125 && angle > 0.39269875) {
		if (angle > 1.17809625) {
			if (angle > 1.96349375) {
				if (angle > 2.74889125) {
					if (angle > 3.53428875) {
						if (angle > 4.31968625) {
							if (angle > 5.105083749999999)
								return 2;
							else
								return 0;
						} else {
							return 14;
						}
					} else {
						return 12;
					}
				} else {
					return 10;
				}
			} else {
				return 8;
			}
		} else {
			return 6;
		}
	} else {
		return 4;
	}
}

} // namespace AGDS
