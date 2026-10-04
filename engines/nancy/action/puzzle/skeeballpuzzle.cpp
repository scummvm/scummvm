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

#include "common/algorithm.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/skeeballpuzzle.h"

namespace Nancy {
namespace Action {

// How fast the picture of a rolling ball turns
static const double kRollFrameRate = 15.0;

SkeeBallPuzzle::~SkeeBallPuzzle() {
	delete _heldBall;
	for (Ball *ball : _balls) {
		delete ball;
	}
}

static void readRectList(Common::SeekableReadStream &stream, Common::Array<Common::Rect> &rects) {
	rects.resize(stream.readUint16LE());
	for (Common::Rect &rect : rects) {
		readExclusiveRect(stream, rect);
	}
}

static void readSceneBlock(Common::SeekableReadStream &stream, SceneChangeWithFlag &scene, RandomSoundBlock &sound) {
	scene._sceneChange.sceneID = stream.readUint16LE();
	scene._sceneChange.frameID = stream.readUint16LE();
	scene._sceneChange.continueSceneSound = kContinueSceneSound;
	scene._flag.label = stream.readSint16LE();
	scene._flag.flag = stream.readByte();
	sound.readData(stream);
}

bool SkeeBallPuzzle::compareStateNames(const TargetState &a, const TargetState &b) {
	return a.name.compareToIgnoreCase(b.name) < 0;
}

uint SkeeBallPuzzle::findState(const Common::String &name) const {
	for (uint i = 0; i < _states.size(); ++i) {
		if (_states[i].name.equalsIgnoreCase(name)) {
			return i;
		}
	}

	error("SkeeBallPuzzle: there is no target picture called %s", name.c_str());
}

void SkeeBallPuzzle::readData(Common::SeekableReadStream &stream) {
	// The sizes and positions here are what the balls move by, so the rects are read as they are
	readFilename(stream, _imageName);
	_numSpins = stream.readSint32LE();
	_spinTime = (uint32)(stream.readDoubleLE() * 1000);
	readRectList(stream, _targetSrcs);

	_states.resize(stream.readUint16LE());
	for (TargetState &state : _states) {
		readFilename(stream, state.name);
		state.frame = stream.readSint32LE();
	}

	// A target turns to the next picture in the order of their names
	Common::sort(_states.begin(), _states.end(), compareStateNames);

	Common::String startState;
	readFilename(stream, startState);
	uint startStateID = findState(startState);

	_targets.resize(stream.readUint16LE());
	for (Target &target : _targets) {
		readExclusiveRect(stream, target.dest);
		target.stateID = startStateID;
		target.frame = _states[startStateID].frame;
	}

	_dispenserCursor = stream.readUint16LE();
	readExclusiveRect(stream, _dispenserHotspot);
	readExclusiveRect(stream, _dispenserDest);
	readFilename(stream, _dispenserMovieName);

	_aimCursor = stream.readUint16LE();
	readExclusiveRect(stream, _arrowSrc);
	readExclusiveRect(stream, _aimHotspot);
	_minAngle = stream.readDoubleLE() * M_PI / 180;
	_maxAngle = stream.readDoubleLE() * M_PI / 180;
	_minPower = stream.readDoubleLE();
	_maxPower = stream.readDoubleLE();

	readExclusiveRect(stream, _laneBounds);
	readExclusiveRect(stream, _ramp);
	_friction = stream.readDoubleLE();
	readExclusiveRect(stream, _ballStart);
	_gravity = stream.readDoubleLE();
	_heightScale = stream.readDoubleLE();
	readRectList(stream, _ballSrcs);

	_pickUpSound.readData(stream);
	_rollSound.readData(stream);
	_spinSound.readData(stream);

	_goals.resize(stream.readUint16LE());
	for (Goal &goal : _goals) {
		Common::String stateName;
		readFilename(stream, stateName);
		goal.stateID = findState(stateName);
		readSceneBlock(stream, goal.scene, goal.sound);
	}

	readExitHotspot(stream);
}

void SkeeBallPuzzle::init() {
	initViewportSurface();
	loadImage();

	_angle = _maxAngle;
	_power = _maxPower;

	if (!_dispenserMovieName.empty() && _dispenserMovie.loadFile(_dispenserMovieName)) {
		_dispenserMovie.playRange(0, _dispenserMovie.getFrameCount() - 1);
	}

	_lastUpdateTime = g_nancy->getTotalPlayTime();
	redraw();
}

void SkeeBallPuzzle::createBall() {
	_heldBall = new Ball();
	PhysicalObject &body = _heldBall->body;
	body.setSize(_ballStart.width(), _ballStart.height());
	body.setFriction(_friction);
	body.setBounds(Common::Rect(_laneBounds.left, _laneBounds.top, _laneBounds.right + 1, _laneBounds.bottom + 1));
	body.setBoundsMode(PhysicalObject::kBounceOffBounds);
	body.moveTo(_ballStart.left + _ballStart.width() * 0.5, _ballStart.top + _ballStart.height() * 0.5);
	body.setSpeed(0);
	body.setHeading(_angle);
}

bool SkeeBallPuzzle::updateBall(Ball &ball, uint32 numTicks) {
	PhysicalObject &body = ball.body;
	if (body.getSpeed() == 0) {
		return true;
	}

	body.update(numTicks);

	// Off the ramp the ball flies, and falls
	const double seconds = numTicks * PhysicalObject::kTickLength / 1000.0;
	if (ball.height != 0 || ball.verticalSpeed != 0) {
		ball.verticalSpeed -= seconds * _gravity;
		ball.height += seconds * ball.verticalSpeed;
	}

	ball.frame += body.getSpeed() / (0.5 * _maxPower + 0.5 * _minPower) * kRollFrameRate * numTicks;
	while (!_ballSrcs.empty() && ball.frame >= _ballSrcs.size()) {
		ball.frame -= _ballSrcs.size();
	}

	// The ball gets off the ground once it is past the ramp, seen from where it started
	Common::Rect rect = body.getRect();
	double rampX = _ramp.left + _ramp.width() / 2;
	double rampY = _ramp.top + _ramp.height() / 2;
	double startX = _ballStart.left + _ballStart.width() / 2 - rampX;
	double startY = _ballStart.top + _ballStart.height() / 2 - rampY;
	double ballX = rect.left + rect.width() / 2 - rampX;
	double ballY = rect.top + rect.height() / 2 - rampY;
	if (startX * ballX + startY * ballY < 0 && ball.verticalSpeed == 0) {
		body.setFriction(0);
		ball.verticalSpeed = body.getSpeed() * 0.5;
		body.setSpeed(ball.verticalSpeed);
	}

	if (ball.height >= 0) {
		return false;
	}

	// Down again, on the nearest target
	Target *nearest = nullptr;
	int nearestDistance = 0;
	Common::Point center(rect.left + rect.width() / 2, rect.top + rect.height() / 2);
	for (Target &target : _targets) {
		int dx = target.dest.left + target.dest.width() / 2 - center.x;
		int dy = target.dest.top + target.dest.height() / 2 - center.y;
		int distance = dx * dx + dy * dy;
		if (!nearest || distance < nearestDistance) {
			nearest = &target;
			nearestDistance = distance;
		}
	}

	if (nearest) {
		spinTarget(*nearest);
	}

	return true;
}

void SkeeBallPuzzle::spinTarget(Target &target) {
	const int32 numFrames = _targetSrcs.size();

	target.spinStartTime = g_nancy->getTotalPlayTime();
	target.spinStartFrame = target.frame;
	target.isSpinning = true;
	target.stateID = (target.stateID + 1) % _states.size();

	// A few whole turns, then on to the frame of the next picture
	int32 rest = _states[target.stateID].frame - target.frame;
	if (rest < 0) {
		rest += numFrames;
	}

	target.spinEndFrame = numFrames * _numSpins + target.frame + rest;
	playSoundBlock(_spinSound);
}

void SkeeBallPuzzle::updateTargets() {
	const uint32 now = g_nancy->getTotalPlayTime();
	bool hasStopped = false;

	for (Target &target : _targets) {
		if (!target.isSpinning) {
			continue;
		}

		if (now < target.spinStartTime + _spinTime) {
			double progress = (now - target.spinStartTime) / (double)_spinTime;
			target.frame = target.spinStartFrame + (int32)((target.spinEndFrame - target.spinStartFrame) * progress);
		} else {
			target.isSpinning = false;
			target.frame = _states[target.stateID].frame;
			hasStopped = true;
		}
	}

	if (!hasStopped) {
		return;
	}

	// A goal is met once every target shows its picture
	for (uint i = 0; i < _goals.size(); ++i) {
		Goal &goal = _goals[i];
		if (goal.isMet) {
			continue;
		}

		bool isMet = true;
		for (const Target &target : _targets) {
			if (target.isSpinning || target.frame != _states[goal.stateID].frame) {
				isMet = false;
				break;
			}
		}

		if (!isMet) {
			continue;
		}

		goal.isMet = true;
		playSoundBlock(goal.sound);
		NancySceneState.setEventFlag(goal.scene._flag);

		if (goal.scene._sceneChange.sceneID != kNoSceneNancy17) {
			_metGoal = i;
		}
	}
}

void SkeeBallPuzzle::drawBall(const Ball &ball) {
	if (_ballSrcs.empty()) {
		return;
	}

	const Common::Rect &src = _ballSrcs[CLIP<int>((int)ball.frame, 0, _ballSrcs.size() - 1)];
	Common::Rect rect = ball.body.getRect();

	// The higher the ball, the larger it looks
	double scale = _heightScale != 0 ? ball.height / _heightScale + 1.0 : 1.0;
	Common::Rect dest(rect.left, rect.top, rect.left + (int)(src.width() * scale), rect.top + (int)(src.height() * scale));
	_drawSurface.transBlitFrom(_image, src, dest, _image.getTransparentColor());
}

void SkeeBallPuzzle::drawArrow() {
	// The arrow turns around its left end, which sits on the ball, and gets longer with the power
	const double cosA = cos(_angle);
	const double sinA = sin(_angle);
	const double stretch = (_power - _minPower) / (_maxPower - _minPower) + 0.5;
	const Common::Point pivot(_aimHotspot.left + _aimHotspot.width() / 2, _aimHotspot.top + _aimHotspot.height() / 2);
	const double halfHeight = _arrowSrc.height() / 2.0;
	const int radius = (int)ceil(_arrowSrc.width() * stretch + _arrowSrc.height());
	const uint32 transColor = _image.getTransparentColor();

	for (int y = MAX<int>(0, pivot.y - radius); y < MIN<int>(_drawSurface.h, pivot.y + radius); ++y) {
		for (int x = MAX<int>(0, pivot.x - radius); x < MIN<int>(_drawSurface.w, pivot.x + radius); ++x) {
			// Back to the arrow's own picture, which points right
			double dx = x + 0.5 - pivot.x;
			double dy = y + 0.5 - pivot.y;
			double alongX = dx * cosA - dy * sinA;
			double alongY = dx * sinA + dy * cosA;
			int srcX = (int)floor(alongX / stretch);
			int srcY = (int)floor(alongY + halfHeight);

			if (srcX < 0 || srcX >= _arrowSrc.width() || srcY < 0 || srcY >= _arrowSrc.height()) {
				continue;
			}

			uint32 color = _image.getPixel(_arrowSrc.left + srcX, _arrowSrc.top + srcY);
			if (color != transColor) {
				_drawSurface.setPixel(x, y, color);
			}
		}
	}
}

void SkeeBallPuzzle::redraw() {
	_drawSurface.clear(_drawSurface.getTransparentColor());

	if (_dispenserMovie.isVideoLoaded()) {
		_dispenserMovie.drawFrame(_drawSurface, _dispenserDest.origin());
	}

	for (const Target &target : _targets) {
		if (!_targetSrcs.empty()) {
			const Common::Rect &src = _targetSrcs[target.frame % _targetSrcs.size()];
			_drawSurface.blitFrom(_image, src, target.dest.origin());
		}
	}

	if (_heldBall) {
		drawBall(*_heldBall);
	}

	for (const Ball *ball : _balls) {
		drawBall(*ball);
	}

	if (_isAiming) {
		drawArrow();
	}

	_needsRedraw = true;
}

void SkeeBallPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun: {
		if (_exitRequested) {
			_state = kActionTrigger;
			break;
		}

		if (_dispenserMovie.isRangePlaying()) {
			_dispenserMovie.update();
		}

		uint32 now = g_nancy->getTotalPlayTime();
		uint32 numTicks = (now - _lastUpdateTime) / PhysicalObject::kTickLength;
		_lastUpdateTime += numTicks * PhysicalObject::kTickLength;

		if (numTicks > 0) {
			// The rolling sound goes on while any ball rolls
			if (!_balls.empty() && !isSoundBlockPlaying(_rollSound)) {
				playSoundBlock(_rollSound);
			} else if (_balls.empty() && isSoundBlockPlaying(_rollSound)) {
				g_nancy->_sound->stopSound(_rollSound.channel);
			}

			for (uint i = 0; i < _balls.size(); ) {
				if (updateBall(*_balls[i], numTicks)) {
					delete _balls[i];
					_balls.remove_at(i);
				} else {
					++i;
				}
			}

			updateTargets();
		}

		if (_metGoal != -1) {
			_state = kActionTrigger;
		}

		redraw();
		break;
	}
	case kActionTrigger:
		if (_metGoal != -1) {
			NancySceneState.changeScene(_goals[_metGoal].scene._sceneChange);
		} else {
			_exitScene.execute();
		}

		finishExecution();
		break;
	}
}

void SkeeBallPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _exitRequested || _metGoal != -1) {
		return;
	}

	Common::Rect viewportBounds = NancySceneState.getViewport().getBounds();
	if (!NancySceneState.getViewport().convertViewportToScreen(viewportBounds).contains(input.mousePos)) {
		return;
	}

	Common::Rect mouseRect = NancySceneState.getViewport().convertScreenToViewport(
		Common::Rect(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1));
	Common::Point pos = mouseRect.origin();

	const bool isDispenserReady = !_dispenserMovie.isRangePlaying();

	if (_isAiming) {
		// The ball is pulled back: it goes the other way, harder the further it is pulled
		Common::Point center(_aimHotspot.left + _aimHotspot.width() / 2, _aimHotspot.top + _aimHotspot.height() / 2);
		double dx = center.x - pos.x;
		double dy = center.y - pos.y;
		_angle = PhysicalObject::getAngle(dx, dy);
		if (_angle < _minAngle) {
			_angle = _minAngle;
		} else if (_angle > _maxAngle) {
			_angle = _maxAngle;
		}

		double halfArrow = _arrowSrc.width() * 0.5;
		double amount = CLIP((sqrt(dx * dx + dy * dy) - halfArrow) / _arrowSrc.width(), 0.0, 1.0);
		_power = _minPower + (_maxPower - _minPower) * amount;

		setDataCursor(_aimCursor);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			// Let go
			_heldBall->lastUpdateTime = g_nancy->getTotalPlayTime();
			_heldBall->body.setHeading(_angle);
			_heldBall->body.setSpeed(_power);
			_balls.push_back(_heldBall);
			_heldBall = nullptr;
			_isAiming = false;
			input.eatMouseInput();
		}

		return;
	}

	const bool pressed = input.input & NancyInput::kLeftMouseButtonDown;

	if (!_heldBall && _dispenserHotspot.contains(pos)) {
		setDataCursor(_dispenserCursor, isDispenserReady);

		if (pressed && isDispenserReady) {
			createBall();
			if (_dispenserMovie.isVideoLoaded()) {
				_dispenserMovie.playRange(0, _dispenserMovie.getFrameCount() - 1);
			}

			playSoundBlock(_pickUpSound);
			input.eatMouseInput();
		}

		return;
	}

	if (_heldBall && _aimHotspot.contains(pos)) {
		setDataCursor(_aimCursor);

		if (pressed) {
			_isAiming = true;
			_angle = (_minAngle + _maxAngle) / 2;
			_power = _minPower;
			input.eatMouseInput();
		}

		return;
	}

	if (hoverExitHotspot(input) && (input.input & NancyInput::kLeftMouseButtonUp)) {
		_exitRequested = true;
		input.eatMouseInput();
	}
}

} // End of namespace Action
} // End of namespace Nancy
