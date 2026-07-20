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
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/pachinkopuzzle.h"

namespace Nancy {
namespace Action {

// -- Physics constants --
static const double kGravity = 50.0;			// downward accel (px/s^2)
static const double kRestitution = 0.85;		// bounce energy retained
static const double kTwoPi = 6.283185307179586;
static const double kDeg2Rad = 0.017453292519943295;
// Launch-param jitter: progress*0.95 + (rand%100)*0.002; launch-Y jitter: (rand%100)*0.01.
static const double kSpawnParamMax = 0.95;
static const double kSpawnParamJitter = 0.002;
static const double kSpawnYJitter = 0.01;
// Per-ball deceleration is not in the chunk (it lives in a runtime-built physics body), so
// a small constant drag is used here as an approximation.
static const double kDrag = 12.0;				// px/s per second

static double wrapAngle(double a) {
	while (a < 0.0) {
		a += kTwoPi;
	}
	while (a >= kTwoPi) {
		a -= kTwoPi;
	}
	return a;
}

void PachinkoPuzzle::readData(Common::SeekableReadStream &stream) {
	readFilename(stream, _imageName);			// 0x00 - board overlay

	// 167-byte header blob.
	readRect(stream, _ballSrc);					// ball sprite source
	stream.skip(16);							// four unidentified int32
	_velMin = stream.readSint32LE();			// launch-speed floor
	_velMax = stream.readSint32LE();			// launch-speed ceiling
	_spawnYMin = stream.readSint32LE();			// launch-heading (deg) range
	_spawnYMax = stream.readSint32LE();
	stream.skip(4);
	_launchVecLen = stream.readSint32LE();
	stream.skip(4);
	stream.skip(16);							// four unidentified int32
	_eventFlag = stream.readSint16LE();			// "in progress" flag
	readFilename(stream, _ballImageName);		// ball sprite sheet
	readRect(stream, _launcherBallSrc);
	readRect(stream, _launcherBallDest);
	readRect(stream, _launcherHotspot);			// the clickable launcher
	_spawnWindowMin = stream.readSint32LE();
	_spawnWindowMax = stream.readSint32LE();	// spawn window (ms)

	// The random "plink" cues played each time a ball is launched (LeverPull*).
	_plinkSounds.readData(stream);

	// The two catch machines: Miner (win) then Yeti (lose).
	readMachine(stream, _winMachine);
	readMachine(stream, _loseMachine);

	// Static pin collision rects.
	int16 numPins = stream.readSint16LE();
	if (numPins > 0) {
		_pins.resize(numPins);
		for (int16 i = 0; i < numPins; ++i) {
			readRect(stream, _pins[i]);
		}
	}

	// The polymorphic Nancy13 ActionZone array (bumpers / walls / overlays). The shared
	// ActionZone reader handles the Nancy13 layout when told to.
	readActionZoneArray(stream, _zones, true);

	// The base trailer: a count-prefixed array of 23-byte hotspot records. The sample
	// carries the single "give up / exit" hotspot.
	int16 numExit = stream.readSint16LE();
	for (int16 i = 0; i < numExit; ++i) {
		Common::Rect r;
		readRect(stream, r);
		uint16 cursorType = stream.readUint16LE();
		uint16 sceneID = stream.readUint16LE();
		int16 exitFlagLabel = stream.readSint16LE();
		byte exitFlagValue = stream.readByte();

		if (i == 0) {
			_exitHotspot = r;
			_exitCursorType = cursorType;
			_exitScene.sceneID = sceneID;
			// The field after the scene id is a flag label (set on give-up), not a frame.
			_exitScene.frameID = 0;
			_exitFlag.label = exitFlagLabel;
			_exitFlag.flag = exitFlagValue;
		}
	}
}

void PachinkoPuzzle::readMachine(Common::SeekableReadStream &stream, Machine &m) {
	readFilename(stream, m.imageName);			// the ANIM_OVL sprite strip
	m.animRate = stream.readSint32LE();			// frames per second

	// Sprite-strip source rects (an embedded "sprite container": int16 count + rects).
	int16 numFrames = stream.readSint16LE();
	if (numFrames > 0) {
		m.frames.resize(numFrames);
		for (int16 i = 0; i < numFrames; ++i) {
			readRect(stream, m.frames[i]);
		}
	}

	// The slide "mover": a start rect, an end/catch rect, and a speed.
	readRect(stream, m.moverStart);
	readRect(stream, m.moverEnd);
	m.moverSpeed = stream.readSint32LE();

	m.winchSound.readData(stream);				// snd1 - the winch-up cue

	// The 55-byte blob: its leading filename is the result movie (MUS_PachinkoWinANIM for
	// the Miner); an empty filename means no movie.
	byte blob[55];
	stream.read(blob, sizeof(blob));
	int len = 0;
	while (len < 33 && blob[len] != 0) {
		++len;
	}
	m.movieName = Common::String((const char *)blob, len);

	m.resultSound.readData(stream);				// snd2 - the win/lose voice cue

	stream.skip(1);								// byte (unused here)
	stream.skip(4);								// int32 (unused here)

	m.fastSound.readData(stream);				// snd3 - the fast-winch cue
}

void PachinkoPuzzle::loadMachineImage(Machine &m) {
	if (!m.imageName.empty()) {
		g_nancy->_resource->loadImage(m.imageName, m.image);
		m.image.setTransparentColor(_drawSurface.getTransparentColor());
	}
}

void PachinkoPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(), g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_imageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
	if (!_ballImageName.empty()) {
		g_nancy->_resource->loadImage(_ballImageName, _ballImage);
		_ballImage.setTransparentColor(_drawSurface.getTransparentColor());
	}

	loadMachineImage(_winMachine);
	loadMachineImage(_loseMachine);

	NancySceneState.setNoHeldItem();

	// In-progress marker: set on entry, cleared once a ball is caught.
	if (_eventFlag > 0) {
		NancySceneState.setEventFlag(_eventFlag, g_nancy->_true);
	}

	_pzState = kRunning;
	_lastUpdate = g_nancy->getTotalPlayTime();

	redraw();
	registerGraphics();
}

SoundDescription PachinkoPuzzle::playSoundBlock(const RandomSoundBlock &block) {
	SoundDescription desc;
	if (block.names.empty()) {
		return desc;
	}

	uint idx = block.names.size() == 1 ? 0 : g_nancy->_randomSource->getRandomNumber(block.names.size() - 1);
	const Common::String &name = block.names[idx];
	if (name.empty() || name == "NO SOUND") {
		return desc;
	}

	desc.name = name;
	desc.channelID = block.channel;
	desc.numLoops = block.numLoops > 0 ? block.numLoops : 1;
	desc.volume = block.volume;

	g_nancy->_sound->loadSound(desc);
	g_nancy->_sound->playSound(desc);
	return desc;
}

void PachinkoPuzzle::setDataCursor(uint16 cursorType) const {
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)cursorType, true);
}

void PachinkoPuzzle::spawnBall() {
	uint32 now = g_nancy->getTotalPlayTime();
	uint32 elapsed = now - _spawnClickTime;

	// The launch parameter grows across the spawn window, plus a small random jitter.
	double launchParam = kSpawnParamMax;
	int span = _spawnWindowMax - _spawnWindowMin;
	if ((int)elapsed <= _spawnWindowMax && span > 0) {
		int rel = (int)elapsed - _spawnWindowMin;
		if (rel < 0) {
			rel = 0;
		}
		launchParam = ((double)rel / (double)span) * kSpawnParamMax;
	}
	launchParam += (double)(g_nancy->_randomSource->getRandomNumber(99)) * kSpawnParamJitter;

	// The launch heading (degrees) comes from the [_spawnYMin,_spawnYMax] range.
	double headingDeg = (double)_spawnYMin +
		(double)(_spawnYMax - _spawnYMin) * (double)(g_nancy->_randomSource->getRandomNumber(99)) * kSpawnYJitter;

	Ball ball;
	ball.speed = (double)(_velMax - _velMin) * launchParam + (double)_velMin;
	ball.angle = wrapAngle(headingDeg * kDeg2Rad);
	ball.x = (_launcherHotspot.left + _launcherHotspot.right) / 2.0;
	ball.y = (_launcherBallDest.top + _launcherBallDest.bottom) / 2.0;
	ball.active = true;
	_balls.push_back(ball);

	playSoundBlock(_plinkSounds);
}

bool PachinkoPuzzle::collideBall(Ball &ball, double nx, double ny) const {
	// Reflect off any pin or bumper rect the ball would enter. The axis of the smaller
	// penetration decides which velocity component flips (an AABB response).
	Common::Point p((int16)nx, (int16)ny);

	const Common::Rect *hit = nullptr;
	for (const Common::Rect &r : _pins) {
		if (r.contains(p)) {
			hit = &r;
			break;
		}
	}
	if (!hit) {
		for (const ActionZone &z : _zones) {
			// Walls (type 1/0x14) and bumpers (0x16) are solid; overlays are not.
			if ((z.type == 0x01 || z.type == 0x14 || z.type == 0x16) && z.rect.contains(p)) {
				hit = &z.rect;
				if (!z._sound.names.empty()) {
					const_cast<PachinkoPuzzle *>(this)->playSoundBlock(z._sound);
				}
				break;
			}
		}
	}

	if (!hit) {
		return false;
	}

	double vx = cos(ball.angle) * ball.speed;
	double vy = -sin(ball.angle) * ball.speed;

	// Penetration depth on each axis; flip the axis with the shallower overlap.
	double penLeft = nx - hit->left;
	double penRight = hit->right - nx;
	double penTop = ny - hit->top;
	double penBottom = hit->bottom - ny;
	double penX = MIN(penLeft, penRight);
	double penY = MIN(penTop, penBottom);

	if (penX < penY) {
		vx = -vx;
	} else {
		vy = -vy;
	}

	ball.speed = sqrt(vx * vx + vy * vy) * kRestitution;
	ball.angle = wrapAngle(atan2(-vy, vx));
	return true;
}

void PachinkoPuzzle::stepBall(Ball &ball, double dt) {
	// Gravity: convert the polar velocity to cartesian, add gravity, convert back.
	double vx = cos(ball.angle) * ball.speed;
	double vy = -sin(ball.angle) * ball.speed;
	vy += kGravity * dt;
	ball.speed = sqrt(vx * vx + vy * vy);
	ball.angle = wrapAngle(atan2(-vy, vx));

	// Deceleration (drag).
	ball.speed -= kDrag * dt;
	if (ball.speed < 0.0) {
		ball.speed = 0.0;
	}

	// Move, reflecting on a collision (single sub-step is sufficient at this frame rate).
	double dist = ball.speed * dt;
	double nx = ball.x + cos(ball.angle) * dist;
	double ny = ball.y - sin(ball.angle) * dist;
	if (!collideBall(ball, nx, ny)) {
		ball.x = nx;
		ball.y = ny;
	}
}

bool PachinkoPuzzle::ballSettled(const Ball &ball, const Machine &m) const {
	Common::Point c = m.catchPoint();
	double dx = ball.x - c.x;
	double dy = ball.y - c.y;
	// A generous catch radius (the machine's mover end rect half-size).
	int r = MAX(m.moverEnd.width(), m.moverEnd.height()) / 2 + 8;
	return (dx * dx + dy * dy) <= (double)(r * r);
}

void PachinkoPuzzle::advanceMachine(Machine &m, uint32 now) {
	if (m.frames.empty() || m.animRate <= 0) {
		return;
	}
	uint32 period = 1000 / (uint32)m.animRate;
	if (now >= m.nextFrameTime) {
		m.frame = (m.frame + 1) % m.frames.size();
		m.nextFrameTime = now + period;
	}
}

void PachinkoPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());

	// The two machines (their current animation frame), drawn at their mover start.
	const Machine *machines[2] = { &_winMachine, &_loseMachine };
	for (int i = 0; i < 2; ++i) {
		const Machine &m = *machines[i];
		if (m.frames.empty() || m.image.empty()) {
			continue;
		}
		const Common::Rect &src = m.frames[m.frame % m.frames.size()];
		Common::Point pos(m.moverStart.left, m.moverStart.top);
		_drawSurface.blitFrom(m.image, src, pos);
	}

	// The balls in flight.
	if (!_ballImage.empty() && !_ballSrc.isEmpty()) {
		int w = _ballSrc.width();
		int h = _ballSrc.height();
		for (const Ball &ball : _balls) {
			if (!ball.active) {
				continue;
			}
			Common::Point pos((int16)ball.x - w / 2, (int16)ball.y - h / 2);
			_drawSurface.blitFrom(_ballImage, _ballSrc, pos);
		}
	}

	// The result animation, when playing.
	if (_pzState == kWaitResult && _resultMovie.isVideoLoaded()) {
		_resultMovie.drawFrame(_drawSurface, Common::Point(0, 0));
	}

	_needsRedraw = true;
}

void PachinkoPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		_state = kRun;
		// fall through
	case kRun: {
		uint32 now = g_nancy->getTotalPlayTime();
		double dt = (now - _lastUpdate) / 1000.0;
		_lastUpdate = now;
		if (dt > 0.1) {
			dt = 0.1;	// clamp long stalls
		}

		if (_exitRequested) {
			_pzState = kComplete;
		}

		switch (_pzState) {
		case kRunning: {
			if (_spawnPending) {
				spawnBall();
				_spawnPending = false;
			}

			advanceMachine(_winMachine, now);
			advanceMachine(_loseMachine, now);

			for (uint i = 0; i < _balls.size(); ++i) {
				Ball &ball = _balls[i];
				if (!ball.active) {
					continue;
				}
				stepBall(ball, dt);

				// A caught ball decides the outcome and starts that machine's result.
				if (ballSettled(ball, _winMachine)) {
					ball.active = false;
					_solved = true;
					_activeMachine = &_winMachine;
					_pzState = kPlayResult;
					break;
				}
				if (ballSettled(ball, _loseMachine)) {
					ball.active = false;
					_solved = false;
					_activeMachine = &_loseMachine;
					_pzState = kPlayResult;
					break;
				}

				// A ball that leaves the board is discarded.
				if (ball.y > _drawSurface.h + 64 || ball.x < -64 || ball.x > _drawSurface.w + 64) {
					ball.active = false;
				}
			}

			redraw();
			break;
		}
		case kPlayResult: {
			if (_eventFlag > 0) {
				NancySceneState.setEventFlag(_eventFlag, g_nancy->_false);
			}
			if (_activeMachine) {
				_resultSoundDesc = playSoundBlock(_activeMachine->resultSound);
				if (!_activeMachine->movieName.empty() &&
						_resultMovie.loadFile(_activeMachine->movieName) &&
						_resultMovie.getFrameCount() > 0) {
					_resultMovie.playRange(0, _resultMovie.getFrameCount() - 1);
				}
			}
			_resultTime = now;
			_pzState = kWaitResult;
			redraw();
			break;
		}
		case kWaitResult: {
			bool movieDone = !_resultMovie.isVideoLoaded() ||
				(!_resultMovie.isRangePlaying());
			if (_resultMovie.isVideoLoaded() && _resultMovie.update()) {
				redraw();
			}
			bool soundDone = _resultSoundDesc.name.empty() ||
				!g_nancy->_sound->isSoundPlaying(_resultSoundDesc);
			if (movieDone && soundDone && now - _resultTime > 500) {
				if (_resultMovie.isVideoLoaded()) {
					_resultMovie.close();
				}
				_pzState = kComplete;
			}
			break;
		}
		case kComplete:
			_state = kActionTrigger;
			break;
		}

		break;
	}
	case kActionTrigger:
		// The give-up hotspot and the completion path both route to the exit scene; the
		// win/lose branch is driven downstream by the solved flag and the puzzle event flag.
		NancySceneState.setEventFlag(_exitFlag);
		if (_exitScene.sceneID != kNoScene) {
			NancySceneState.changeScene(_exitScene);
		}
		finishExecution();
		break;
	}
}

void PachinkoPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun || _pzState != kRunning) {
		return;
	}

	const bool click = (input.input & NancyInput::kLeftMouseButtonUp) != 0;

	if (!_launcherHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_launcherHotspot).contains(input.mousePos)) {
		setDataCursor(_exitCursorType);
		if (click) {
			// One launcher click queues one ball.
			_spawnPending = true;
			_spawnClickTime = g_nancy->getTotalPlayTime();
		}
		input.eatMouseInput();
		return;
	}

	if (!_exitHotspot.isEmpty() &&
			NancySceneState.getViewport().convertViewportToScreen(_exitHotspot).contains(input.mousePos)) {
		setDataCursor(_exitCursorType);
		if (click) {
			_exitRequested = true;
		}
	}
}

} // End of namespace Action
} // End of namespace Nancy
