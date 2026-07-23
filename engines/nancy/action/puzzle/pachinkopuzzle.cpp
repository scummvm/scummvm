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

// Collision geometry. Pins are stored as 1x1 points on a staggered ~30px grid; on screen
// they are small round bumpers, so the hit distance matches the drawn pin + ball radii (a
// ball may still thread the wider gaps, which is fine - the stagger catches it a row later).
static const double kPinRadius = 4.0;
static const double kBallRadius = 5.0;
static const int kPhysicsSubsteps = 4;			// per frame, to avoid tunnelling the pins

static const uint32 kHoleLitMs = 400;			// how long a hole stays lit after a catch

// A climber reaches the pot once its accumulated steps hit this goal. moverSpeed per catch
// is 2 (Miner) / 3 (Yeti), so the Yeti climbs faster - matching the game's "the Yeti
// usually wins". The exact goal is not recovered from the chunk; this is a tuned value.
static const int kClimbGoal = 30;

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
	readRect(stream, _ballEntry);				// top-right entry chute
	_velMin = stream.readSint32LE();			// launch-speed floor
	_velMax = stream.readSint32LE();			// launch-speed ceiling
	_spawnYMin = stream.readSint32LE();			// launch-heading (deg) range
	_spawnYMax = stream.readSint32LE();
	stream.skip(4);
	_launchVecLen = stream.readSint32LE();
	stream.skip(4);
	readRect(stream, _panelBounds);				// pin-panel bounds (walls + floor)
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

	stream.skip(1);								// per-machine flag byte

	// The 55-byte blob: a filename[33] (the result movie - MUS_PachinkoWinANIM /
	// MUS_PachinkoLoseANIM), then its 16-byte destination rect, then int32 + int16.
	readFilename(stream, m.movieName);
	readRect(stream, m.movieDest);
	stream.skip(6);

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

// The four holes are the bumper zones (type 0x16) that carry a bell sound. The sound name
// maps each hole to its climber: Miner/Explosion feed the Gold Digger (win), Yeti/Ouch feed
// the Yeti (lose).
void PachinkoPuzzle::buildHoles() {
	_holes.clear();
	for (const ActionZone &z : _zones) {
		if (z.type != 0x16 || z._sound.names.empty()) {
			continue;
		}

		Machine *climber = nullptr;
		for (const Common::String &n : z._sound.names) {
			if (n.contains("Miner") || n.contains("Explosion")) {
				climber = &_winMachine;
				break;
			}
			if (n.contains("Yeti") || n.contains("Ouch")) {
				climber = &_loseMachine;
				break;
			}
		}
		if (!climber) {
			continue;
		}

		Hole hole;
		hole.rect = z.rect;
		hole.climber = climber;
		hole.sound = z._sound;
		_holes.push_back(hole);
	}

	// Attach each hole's "lit" sprite from the matching overlay (0x0d) zone, which shares
	// the hole's rect and names the lit board overlay.
	for (const ActionZone &z : _zones) {
		if (z.type != 0x0d || z.overlaySrcRects.empty()) {
			continue;
		}
		for (Hole &hole : _holes) {
			if (hole.rect == z.rect) {
				hole.litSrc = z.overlaySrcRects[0];
				hole.litDest = z.overlayDestRect;
				if (_litImageName.empty()) {
					_litImageName = Common::Path(z.overlayName.c_str());
				}
				break;
			}
		}
	}

	if (!_litImageName.empty()) {
		g_nancy->_resource->loadImage(_litImageName, _litImage);
		_litImage.setTransparentColor(_drawSurface.getTransparentColor());
	}
}

// The climber's current on-screen anchor: the point on its climb path from the bottom
// (moverStart) to the pot (moverEnd), advanced by how many balls it has caught.
Common::Point PachinkoPuzzle::climberAnchor(const Machine &m) const {
	double t = (double)m.climbSteps / (double)kClimbGoal;
	if (t > 1.0) {
		t = 1.0;
	}
	Common::Point start((m.moverStart.left + m.moverStart.right) / 2, (m.moverStart.top + m.moverStart.bottom) / 2);
	Common::Point end((m.moverEnd.left + m.moverEnd.right) / 2, (m.moverEnd.top + m.moverEnd.bottom) / 2);
	return Common::Point((int16)(start.x + (end.x - start.x) * t), (int16)(start.y + (end.y - start.y) * t));
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

	buildHoles();

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
	// Balls appear at the top-right entry chute and fall left/down through the pins.
	ball.x = (_ballEntry.left + _ballEntry.right) / 2.0;
	ball.y = (_ballEntry.top + _ballEntry.bottom) / 2.0;
	ball.active = true;
	_balls.push_back(ball);

	playSoundBlock(_plinkSounds);
}

// Reflect the ball off any pin it now overlaps (pins collide as circles). Returns true if a
// bounce happened.
bool PachinkoPuzzle::collidePins(Ball &ball) const {
	const double hitDist = kPinRadius + kBallRadius;
	for (const Common::Rect &pin : _pins) {
		double px = (pin.left + pin.right) / 2.0;
		double py = (pin.top + pin.bottom) / 2.0;
		double dx = ball.x - px;
		double dy = ball.y - py;
		double d2 = dx * dx + dy * dy;
		if (d2 >= hitDist * hitDist || d2 < 1e-6) {
			continue;
		}

		// Reflect the velocity about the surface normal (pin centre -> ball) and push the
		// ball back out to the contact distance.
		double d = sqrt(d2);
		double nxn = dx / d;
		double nyn = dy / d;
		double vx = cos(ball.angle) * ball.speed;
		double vy = -sin(ball.angle) * ball.speed;
		double dot = vx * nxn + vy * nyn;
		vx -= 2.0 * dot * nxn;
		vy -= 2.0 * dot * nyn;

		ball.x = px + nxn * hitDist;
		ball.y = py + nyn * hitDist;
		ball.speed = sqrt(vx * vx + vy * vy) * kRestitution;
		ball.angle = wrapAngle(atan2(-vy, vx));
		return true;
	}
	return false;
}

int PachinkoPuzzle::catchInHole(const Ball &ball) const {
	Common::Point p((int16)ball.x, (int16)ball.y);
	for (uint i = 0; i < _holes.size(); ++i) {
		if (_holes[i].rect.contains(p)) {
			return (int)i;
		}
	}
	return -1;
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

	// Advance in small sub-steps so the ball cannot tunnel between the pins, bouncing off a
	// pin or a side wall on the way.
	double subDt = dt / kPhysicsSubsteps;
	for (int s = 0; s < kPhysicsSubsteps; ++s) {
		ball.x += cos(ball.angle) * ball.speed * subDt;
		ball.y -= sin(ball.angle) * ball.speed * subDt;

		if (collidePins(ball)) {
			continue;
		}

		// Side walls of the panel keep the ball in play; the ball enters from the right of
		// the right wall, so that wall only reflects once the ball is inside.
		double vx2 = cos(ball.angle) * ball.speed;
		double vy2 = -sin(ball.angle) * ball.speed;
		bool reflected = false;
		if (ball.x < _panelBounds.left + kBallRadius && vx2 < 0.0) {
			ball.x = _panelBounds.left + kBallRadius;
			vx2 = -vx2;
			reflected = true;
		} else if (ball.x < _panelBounds.right && ball.x > _panelBounds.right - kBallRadius && vx2 > 0.0) {
			ball.x = _panelBounds.right - kBallRadius;
			vx2 = -vx2;
			reflected = true;
		}
		if (reflected) {
			ball.speed = sqrt(vx2 * vx2 + vy2 * vy2) * kRestitution;
			ball.angle = wrapAngle(atan2(-vy2, vx2));
		}
	}
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

	// The two climbers, drawn bottom-centred on their current position along the climb path.
	const Machine *machines[2] = { &_winMachine, &_loseMachine };
	for (int i = 0; i < 2; ++i) {
		const Machine &m = *machines[i];
		if (m.frames.empty() || m.image.empty()) {
			continue;
		}
		const Common::Rect &src = m.frames[m.frame % m.frames.size()];
		Common::Point anchor = climberAnchor(m);
		Common::Point pos(anchor.x - src.width() / 2, anchor.y - src.height());
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

	// Holes lit by a recent catch.
	if (!_litImage.empty()) {
		uint32 now = g_nancy->getTotalPlayTime();
		for (const Hole &hole : _holes) {
			if (hole.litUntil > now && !hole.litSrc.isEmpty()) {
				_drawSurface.blitFrom(_litImage, hole.litSrc, Common::Point(hole.litDest.left, hole.litDest.top));
			}
		}
	}

	// The result animation, when playing, drawn at the active machine's movie dest.
	if (_pzState == kWaitResult && _resultMovie.isVideoLoaded() && _activeMachine) {
		_resultMovie.drawFrame(_drawSurface, Common::Point(_activeMachine->movieDest.left, _activeMachine->movieDest.top));
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

				// A ball that drops in a hole is removed and climbs its machine a step.
				int hole = catchInHole(ball);
				if (hole >= 0) {
					ball.active = false;
					playSoundBlock(_holes[hole].sound);
					_holes[hole].litUntil = now + kHoleLitMs;
					Machine *climber = _holes[hole].climber;
					climber->climbSteps += climber->moverSpeed;
					continue;
				}

				// A ball that reaches the floor or leaves the panel is discarded (a miss).
				if (ball.y > _panelBounds.bottom || ball.x < _panelBounds.left - 32 ||
						ball.x > _panelBounds.right + 64 || ball.y > _drawSurface.h + 32) {
					ball.active = false;
				}
			}

			// The first climber to reach the pot ends the game (Gold Digger = win, Yeti = lose).
			if (_winMachine.climbSteps >= kClimbGoal) {
				_solved = true;
				_activeMachine = &_winMachine;
				_pzState = kPlayResult;
			} else if (_loseMachine.climbSteps >= kClimbGoal) {
				_solved = false;
				_activeMachine = &_loseMachine;
				_pzState = kPlayResult;
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
