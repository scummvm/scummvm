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

#include "common/random.h"

#include "engines/nancy/nancy.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"
#include "engines/nancy/input.h"
#include "engines/nancy/cursor.h"
#include "engines/nancy/util.h"

#include "engines/nancy/state/scene.h"
#include "engines/nancy/action/puzzle/minigolfpuzzle.h"

namespace Nancy {
namespace Action {

// TODO - open items:
//  - Physics constants (power scaling, restitution) are approximations.
//  - No abandon/exit path (quitting an unsolved hole) is identified.

static const double kMaxDrag = 250.0;		// aim distance (mask px) for a full-power shot
static const double kRestitution = 0.8;		// wall-bounce energy retained

// Isometric projection (mask space -> screen): rotate 45 degrees, foreshorten Y by
// half. cos45 == sin45; the Y component is additionally scaled by kIsoYScale.
static const double kIsoCos = 0.70710678118654752;	// cos(pi/4) == sin(pi/4)
static const double kIsoYScale = 0.5;

void MinigolfPuzzle::readData(Common::SeekableReadStream &stream) {
	// 106-byte PuzzleBase header (bulk-copied by the original into the puzzle object).
	readFilename(stream, _ballImageName);		// 0x00
	readFilename(stream, _holeBoundaryName);	// 0x21
	_maxSpeed = stream.readSint32LE();			// 0x42
	_decel = stream.readDoubleLE();				// 0x46
	_launchMode = stream.readByte();			// 0x4e
	readRect(stream, _teeRect);					// 0x4f - the tee / ball-start square
	_initialPower = stream.readSint16LE();		// 0x5f
	_initialAngle = stream.readSint16LE();		// 0x61
	_winEventFlag = stream.readSint16LE();		// 0x63
	stream.skip(4);								// 0x65
	_mirrorFlag = stream.readByte();			// 0x69

	// Ball roll animation frames (one sprite per evenly-spaced rotation).
	uint16 numFrames = stream.readUint16LE();
	_ballFrames.resize(numFrames);
	for (uint i = 0; i < numFrames; ++i) {
		readRect(stream, _ballFrames[i]);
	}

	// Two random-sound blocks: the putt and the wall-bounce cues.
	_puttSound.readData(stream);
	_wallSound.readData(stream);

	// The hole/sink/overlay zones.
	readActionZoneArray(stream, _zones);
}

void MinigolfPuzzle::init() {
	Common::Rect vpBounds = NancySceneState.getViewport().getBounds();
	_drawSurface.create(vpBounds.width(), vpBounds.height(),
		g_nancy->_graphics->getInputPixelFormat());
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	setTransparent(true);
	setVisible(true);
	moveTo(vpBounds);

	g_nancy->_resource->loadImage(_ballImageName, _image);
	_image.setTransparentColor(_drawSurface.getTransparentColor());
	g_nancy->_resource->loadImage(_holeBoundaryName, _boundaryMask);
	if (_boundaryMask.empty()) {
		warning("MinigolfPuzzle: boundary overlay '%s' did not load - walls will not work",
			_holeBoundaryName.toString().c_str());
	}

	// Isometric projection anchors: the mask centre maps to the viewport centre.
	_vpCenterX = vpBounds.width() / 2.0;
	_vpCenterY = vpBounds.height() / 2.0;
	if (!_boundaryMask.empty()) {
		_maskCenterX = _boundaryMask.w / 2.0;
		_maskCenterY = _boundaryMask.h / 2.0;
	}

	// The hole is the zone that plays the sink sounds (GOL_Sink*). (_teeRect comes
	// from the header.) All coordinates are in mask/course space. That zone's
	// "special effect" is really the win transition: its leading id is the scene to
	// change to when the ball is potted (with the effect being the fade).
	for (const ActionZone &z : _zones) {
		bool isHole = false;
		for (const Common::String &n : z._sound.names) {
			if (n.contains("Sink") || n.contains("sink")) {
				isHole = true;
				break;
			}
		}
		if (isHole) {
			_holeRect = z.rect;
			_sinkSound = z._sound;
			_winScene.sceneID = z.specialEffectId;
			if (z.hasSpecialEffect) {
				_winHasFade = true;
				_winFadeType = z.seType;
				_winFadeTotalTime = z.seTotalTime;
				_winFadeToBlackTime = z.seFadeToBlackTime;
				_winFadeRect = z.seRect;
			}
			break;
		}
	}

	// Start the ball on the tee. In the mask the fairway is WHITE and walls/out-of-
	// bounds are BLACK, so the ball start sits on white - calibrate "open" there.
	if (!_teeRect.isEmpty()) {
		_ballX = (_teeRect.left + _teeRect.right) / 2.0;
		_ballY = (_teeRect.top + _teeRect.bottom) / 2.0;
	} else if (!_boundaryMask.empty()) {
		_ballX = _boundaryMask.w / 2.0;
		_ballY = _boundaryMask.h * 0.75;
	} else {
		_ballX = vpBounds.width() / 2.0;
		_ballY = vpBounds.height() * 0.75;
	}

	if (!_boundaryMask.empty() && _ballX >= 0 && _ballY >= 0 &&
			_ballX < _boundaryMask.w && _ballY < _boundaryMask.h) {
		_openColor = _boundaryMask.getPixel((int)_ballX, (int)_ballY);
	}

	// A default aim in the level's preset direction, so the preview shows at once.
	// The cursor sits behind the ball (opposite the launch direction).
	double a = (double)_initialAngle * (M_PI / 180.0);
	_aimCursor = Common::Point((int16)(_ballX - cos(a) * kMaxDrag), (int16)(_ballY + sin(a) * kMaxDrag));

	// Every hole starts with the player placing the ball on the tee.
	_mgState = kPlacing;
	_lastUpdate = g_nancy->getTotalPlayTime();

	redraw();
}

Common::Point MinigolfPuzzle::projectToScreen(double mx, double my) const {
	double px = mx - _maskCenterX;
	double py = my - _maskCenterY;
	double sx = _vpCenterX + kIsoCos * (px - py);
	double sy = _vpCenterY + kIsoCos * kIsoYScale * (px + py);
	return Common::Point((int16)(sx + 0.5), (int16)(sy + 0.5));
}

void MinigolfPuzzle::unprojectToMask(int sx, int sy, double &mx, double &my) const {
	double dx = (sx - _vpCenterX) / kIsoCos;
	double dy = (sy - _vpCenterY) / (kIsoCos * kIsoYScale);
	// Inverse of the (px-py, px+py) rotation.
	double px = (dx + dy) / 2.0;
	double py = (dy - dx) / 2.0;
	mx = px + _maskCenterX;
	my = py + _maskCenterY;
}

bool MinigolfPuzzle::isWall(int px, int py) const {
	if (_boundaryMask.empty()) {
		return false;	// no mask - fall back to open play
	}
	if (px < 0 || py < 0 || px >= (int)_boundaryMask.w || py >= (int)_boundaryMask.h) {
		return true;	// off the course
	}
	return _boundaryMask.getPixel(px, py) != _openColor;
}

void MinigolfPuzzle::drawBall() {
	if (_ballFrames.empty()) {
		return;
	}

	const Common::Rect &src = _ballFrames[_ballFrame % _ballFrames.size()];
	Common::Point c = projectToScreen(_ballX, _ballY);
	Common::Point pos(c.x - src.width() / 2, c.y - src.height() / 2);
	_drawSurface.blitFrom(_image, src, pos);
}

void MinigolfPuzzle::drawAimPreview() {
	// Velocity the current aim would produce (mirrors launchBall: away from cursor).
	double aimX = _ballX - _aimCursor.x;
	double aimY = _ballY - _aimCursor.y;
	double len = sqrt(aimX * aimX + aimY * aimY);
	if (len < 3.0) {
		return;
	}
	double power = MIN(len / kMaxDrag, 1.0) * (double)_maxSpeed;
	double x = _ballX, y = _ballY;
	double vx = (aimX / len) * power;
	double vy = (aimY / len) * power;

	// March the shot forward and drop a ghost ball (the ball sprite itself) at
	// intervals along the predicted path, stopping at the hole or when it stalls.
	const Common::Rect ghostSrc = _ballFrames.empty() ? Common::Rect() : _ballFrames[0];
	const int kSteps = 240;
	for (int i = 0; i < kSteps; ++i) {
		bool reachedHole = stepBall(x, y, vx, vy, 1.0 / 30.0, false);
		if (i % 12 == 0 && !ghostSrc.isEmpty()) {
			Common::Point gc = projectToScreen(x, y);
			Common::Point gp(gc.x - ghostSrc.width() / 2, gc.y - ghostSrc.height() / 2);
			_drawSurface.blitFrom(_image, ghostSrc, gp);
		}
		if (reachedHole || sqrt(vx * vx + vy * vy) < 5.0) {
			break;
		}
	}
}

void MinigolfPuzzle::redraw() {
	_drawSurface.clear(g_nancy->_graphics->getTransColor());
	if (_mgState == kAiming) {
		drawAimPreview();
	}
	drawBall();
	_needsRedraw = true;
}

void MinigolfPuzzle::playSoundBlock(const RandomSoundBlock &block) {
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

void MinigolfPuzzle::launchBall(const Common::Point &maskCursor) {
	// The club is pulled back behind the ball: the shot travels away from the
	// cursor (ball - cursor), golf-backswing style, not toward it.
	double aimX = _ballX - maskCursor.x;
	double aimY = _ballY - maskCursor.y;
	double len = sqrt(aimX * aimX + aimY * aimY);
	if (len < 3.0) {
		return;
	}

	double power = MIN(len / kMaxDrag, 1.0) * (double)_maxSpeed;
	_velX = (aimX / len) * power;
	_velY = (aimY / len) * power;

	playSoundBlock(_puttSound);
	_mgState = kMoving;
	_lastUpdate = g_nancy->getTotalPlayTime();
}

bool MinigolfPuzzle::stepBall(double &x, double &y, double &vx, double &vy, double dt, bool playSounds) {
	double dispX = vx * dt;
	double dispY = vy * dt;

	// Walk the displacement in ~1px sub-steps so a fast ball can't tunnel through
	// a thin wall (the original halves the step recursively for the same reason),
	// reflecting off each axis independently so it can slide along an angled wall.
	int steps = (int)ceil(MAX(ABS(dispX), ABS(dispY)));
	if (steps < 1) {
		steps = 1;
	}
	double sx = dispX / steps;
	double sy = dispY / steps;
	bool bounced = false;

	for (int i = 0; i < steps; ++i) {
		double nx = x + sx;
		double ny = y + sy;

		// Potting takes priority over wall reflection: the cup reads as non-fairway
		// in the mask, so a sub-step that reaches the hole zone sinks the ball rather
		// than bouncing off it. Checked per sub-step so a fast ball can't skip it.
		if (!_holeRect.isEmpty() && _holeRect.contains(Common::Point((int16)(nx + 0.5), (int16)(ny + 0.5)))) {
			x = nx;
			y = ny;
			return true;
		}

		if (isWall((int)(nx + 0.5), (int)(y + 0.5))) {
			sx = -sx;
			vx = -vx * kRestitution;
			nx = x;
			bounced = true;
		}
		if (isWall((int)(x + 0.5), (int)(ny + 0.5))) {
			sy = -sy;
			vy = -vy * kRestitution;
			ny = y;
			bounced = true;
		}
		if (isWall((int)(nx + 0.5), (int)(ny + 0.5))) {
			sx = -sx;
			sy = -sy;
			vx = -vx * kRestitution;
			vy = -vy * kRestitution;
			nx = x;
			ny = y;
			bounced = true;
		}

		x = nx;
		y = ny;
	}

	double f = 1.0 - _decel * dt;
	if (f < 0.0) {
		f = 0.0;
	}
	vx *= f;
	vy *= f;

	if (playSounds && bounced) {
		playSoundBlock(_wallSound);
	}
	return false;
}

void MinigolfPuzzle::updateBall() {
	uint32 now = g_nancy->getTotalPlayTime();
	double dt = (now - _lastUpdate) / 1000.0;
	_lastUpdate = now;
	if (dt <= 0.0) {
		return;
	}
	if (dt > 0.1) {
		dt = 0.1;
	}

	bool reachedHole = stepBall(_ballX, _ballY, _velX, _velY, dt, true);

	if (!_ballFrames.empty()) {
		_ballFrame = (_ballFrame + 1) % _ballFrames.size();
	}

	// Potting the ball wins.
	if (reachedHole) {
		_velX = _velY = 0.0;
		_ballX = (_holeRect.left + _holeRect.right) / 2.0;
		_ballY = (_holeRect.top + _holeRect.bottom) / 2.0;
		_mgState = kSunk;
		_solved = true;
		_sunkTime = now;
		playSoundBlock(_sinkSound);
		if (_winEventFlag != -1) {
			NancySceneState.setEventFlag(_winEventFlag, g_nancy->_true);
		}
		return;
	}

	// Coming to rest without sinking readies the next stroke from where it stopped.
	if (sqrt(_velX * _velX + _velY * _velY) < 5.0) {
		_velX = _velY = 0.0;
		_mgState = kAiming;
	}
}

void MinigolfPuzzle::execute() {
	switch (_state) {
	case kBegin:
		init();
		registerGraphics();
		_state = kRun;
		// fall through
	case kRun:
		if (_mgState == kMoving) {
			updateBall();
			redraw();
		} else if (_mgState == kSunk) {
			// Hold on the sunk ball briefly, then release the record.
			if (g_nancy->getTotalPlayTime() - _sunkTime > 1500) {
				_state = kActionTrigger;
			}
		}
		break;
	case kActionTrigger:
		// Potting shows the hole's win scene (and sets the win event flag). The sink
		// zone's special effect is the fade that covers the change, so start it just
		// before the scene change (it captures the current frame, then dissolves to
		// the new scene).
		if (_solved && _winScene.sceneID >= 1000 && _winScene.sceneID != kNoScene) {
			if (_winHasFade) {
				NancySceneState.specialEffect(_winFadeType, _winFadeTotalTime, _winFadeToBlackTime, _winFadeRect);
			}
			NancySceneState.changeScene(_winScene);
		}
		finishExecution();
		break;
	}
}

void MinigolfPuzzle::handleInput(NancyInput &input) {
	if (_state != kRun) {
		return;
	}

	// Keep the golf-club cursor while the ball is rolling / sunk (no input taken).
	if (_mgState == kMoving || _mgState == kSunk) {
		g_nancy->_cursor->setCursorType((CursorManager::CursorType)34, true);
		return;
	}

	// Cursor position, converted screen -> viewport -> mask (course) space.
	Common::Rect screenPt(input.mousePos.x, input.mousePos.y, input.mousePos.x + 1, input.mousePos.y + 1);
	Common::Rect vpPt = NancySceneState.getViewport().convertScreenToViewport(screenPt);
	double cmx, cmy;
	unprojectToMask(vpPt.left, vpPt.top, cmx, cmy);
	Common::Point cursor((int16)cmx, (int16)cmy);

	if (_mgState == kPlacing) {
		// The ball can only be dropped on the tee square; it tracks the cursor
		// there until a click sets it down. Uses the drag-and-drop hand cursor.
		g_nancy->_cursor->setCursorType(CursorManager::kDragHand);
		if (!_teeRect.isEmpty()) {
			_ballX = CLIP<int>(cursor.x, _teeRect.left, _teeRect.right - 1);
			_ballY = CLIP<int>(cursor.y, _teeRect.top, _teeRect.bottom - 1);
		} else {
			_ballX = cursor.x;
			_ballY = cursor.y;
		}
		redraw();
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			_mgState = kAiming;
		}
		return;
	}

	// kAiming: the golf-club cursor aims (angle) and its distance from the ball
	// sets the power; a click strikes the ball. The club is CURS cursor type 34.
	g_nancy->_cursor->setCursorType((CursorManager::CursorType)34, true);
	_aimCursor = cursor;
	redraw();
	if (input.input & NancyInput::kLeftMouseButtonUp) {
		launchBall(cursor);
	}
}

} // End of namespace Action
} // End of namespace Nancy
